#include "library/common/extensions/cert_validator/platform_bridge/platform_bridge_cert_validator.h"

#include <list>
#include <memory>
#include <type_traits>

#include "library/common/data/utility.h"

namespace Envoy {
namespace Extensions {
namespace TransportSockets {
namespace Tls {

PlatformBridgeCertValidator::PlatformBridgeCertValidator(
    const Envoy::Ssl::CertificateValidationContextConfig* config, SslStats& stats,
    const envoy_cert_validator* platform_validator)
    : config_(config),
      allow_untrusted_certificate_(
config_ != nullptr &&
				   config_->trustChainVerification() ==
                                   envoy::extensions::transport_sockets::tls::v3::CertificateValidationContext::ACCEPT_UNTRUSTED),
      stats_(stats), platform_validator_(platform_validator) {
  ENVOY_BUG(config != nullptr && config->caCert().empty() &&
                config->certificateRevocationList().empty(),
            "Invalid certificate validation context config.");
}

PlatformBridgeCertValidator::~PlatformBridgeCertValidator() {
  // Wait for validation threads to finish.
  for (auto& [id, job] : validation_jobs_) {
    if (job->thread_.joinable()) {
      job->thread_.join();
    }
  }
}

int PlatformBridgeCertValidator::initializeSslContexts(std::vector<SSL_CTX*> /*contexts*/,
                                                       bool /*handshaker_provides_certificates*/) {
  return SSL_VERIFY_PEER;
}

ValidationResults PlatformBridgeCertValidator::doVerifyCertChain(
    STACK_OF(X509) & cert_chain, Ssl::ValidateResultCallbackPtr callback,
    Ssl::SslExtendedSocketInfo* ssl_extended_info,
    const Network::TransportSocketOptionsConstSharedPtr& transport_socket_options,
    SSL_CTX& /*ssl_ctx*/, const CertValidator::ExtraValidationContext& /*validation_context*/,
    bool is_server, absl::string_view hostname) {
  ASSERT(!is_server);
  if (sk_X509_num(&cert_chain) == 0) {
    if (ssl_extended_info) {
      ssl_extended_info->setCertificateValidationStatus(
          Envoy::Ssl::ClientValidationStatus::NotValidated);
    }
    const char* error = "verify cert chain failed: empty cert chain.";
    stats_.fail_verify_error_.inc();
    ENVOY_LOG(debug, error);
    return {ValidationResults::ValidationStatus::Failed, absl::nullopt, error};
  }
  if (callback == nullptr) {
    callback = ssl_extended_info->createValidateResultCallback();
  }

  std::vector<envoy_data> certs;
  for (uint64_t i = 0; i < sk_X509_num(&cert_chain); i++) {
    X509* cert = sk_X509_value(&cert_chain, i);
    unsigned char* cert_in_der = nullptr;
    int der_length = i2d_X509(cert, &cert_in_der);
    ASSERT(der_length > 0 && cert_in_der != nullptr);

    absl::string_view cert_str(reinterpret_cast<char*>(cert_in_der),
                               static_cast<size_t>(der_length));
    certs.push_back(Data::Utility::copyToBridgeData(cert_str));
    OPENSSL_free(cert_in_der);
  }

  absl::string_view host;
  if (transport_socket_options != nullptr &&
      !transport_socket_options->verifySubjectAltNameListOverride().empty()) {
    host = transport_socket_options->verifySubjectAltNameListOverride()[0];
  } else {
    host = hostname;
  }
  std::vector<std::string> subject_alt_names;
  if (transport_socket_options != nullptr) {
    subject_alt_names = transport_socket_options->verifySubjectAltNameListOverride();
  } else {
    subject_alt_names = {std::string(hostname)};
  }

  auto job = std::make_unique<ValidationJob>();
  job->result_callback_ = std::move(callback);
  job->validation_ = std::make_unique<PendingValidation>(job->result_callback_->dispatcher(), std::weak_ptr<PlatformBridgeCertValidator>(shared_this_), platform_validator_, allow_untrusted_certificate_, std::move(certs), host, subject_alt_names);
  job->thread_ = std::thread(&PendingValidation::verifyCertChainByPlatform, job->validation_.get());
  std::thread::id thread_id = job->thread_.get_id();
  validation_jobs_[thread_id] = std::move(job);
  return {ValidationResults::ValidationStatus::Pending, absl::nullopt, absl::nullopt};
}

void PlatformBridgeCertValidator::PendingValidation::verifyCertChainByPlatform() {
  ASSERT(!certs_.empty());
  ENVOY_LOG(trace, "Start verifyCertChainByPlatform for host {}", hostname_);
  // This is running in a stand alone thread other than the engine thread.
  envoy_data leaf_cert_der = certs_[0];
  bssl::UniquePtr<X509> leaf_cert(d2i_X509(
      nullptr, const_cast<const unsigned char**>(&leaf_cert_der.bytes), leaf_cert_der.length));
  envoy_cert_validation_result result =
      platform_validator_->validate_cert(certs_.data(), certs_.size(), hostname_.c_str());
  bool success = result.result == ENVOY_SUCCESS;
  if (!success) {
    ENVOY_LOG(debug, result.error_details);
    postVerifyResultAndCleanUp(/*success=*/allow_untrusted_certificate_,
                                                  result.error_details, result.tls_alert,
						  ValidationFailureType::FAIL_VERIFY_ERROR);
    return;
  }

  absl::string_view error_details;
  // Verify that host name matches leaf cert.
  success = DefaultCertValidator::verifySubjectAltName(leaf_cert.get(), subject_alt_names_);
  if (!success) {
    error_details = "PlatformBridgeCertValidator_verifySubjectAltName failed: SNI mismatch.";
    ENVOY_LOG(debug, error_details);
    postVerifyResultAndCleanUp(/*success=*/allow_untrusted_certificate_,
                                                  error_details, SSL_AD_BAD_CERTIFICATE,
						  ValidationFailureType::FAIL_VERIFY_SAN);
    return;
  }
  postVerifyResultAndCleanUp(success, error_details, SSL_AD_CERTIFICATE_UNKNOWN,
                                                ValidationFailureType::SUCCESS);
}

void PlatformBridgeCertValidator::PendingValidation::postVerifyResultAndCleanUp(
										bool success, absl::string_view error_details, uint8_t tls_alert,
    ValidationFailureType failure_type) {
  dispatcher_.post([success, tls_alert, failure_type,
		    hostname = hostname_,
		    weak_validator = weak_validator_,
		    error = std::string(error_details),
		    thread_id = std::this_thread::get_id()] {
    std::shared_ptr<PlatformBridgeCertValidator> validator = weak_validator.lock();
    if (!validator) {
      return;
    }
    validator->onVerificationComplete(thread_id, hostname, success,
					   error,
					   tls_alert,
					   failure_type);
  });
  if (platform_validator_->release_validator) {
    platform_validator_->release_validator();
  }
}

 void PlatformBridgeCertValidator::onVerificationComplete(std::thread::id thread_id,
							  std::string hostname,
							  bool success,
							  std::string error,
							  uint8_t tls_alert,
							  ValidationFailureType failure_type) {
  auto i = validation_jobs_.extract(thread_id);
  if (i.empty()) {
    IS_ENVOY_BUG("No job found for thread");
    return;
  }
  std::unique_ptr<ValidationJob>& job = i.mapped();
  ENVOY_LOG(trace, "Got validation result for {} from platform", hostname);
  job->thread_.join();
  switch (failure_type) {
  case ValidationFailureType::SUCCESS:
    break;
  case ValidationFailureType::FAIL_VERIFY_ERROR:
    stats_.fail_verify_error_.inc();
  case ValidationFailureType::FAIL_VERIFY_SAN:
    stats_.fail_verify_san_.inc();
  }
  job->result_callback_->onCertValidationResult(success, error, tls_alert);
  ENVOY_LOG(trace,
            "Finished platform cert validation for {}, post result callback to network thread",
            hostname);
}

} // namespace Tls
} // namespace TransportSockets
} // namespace Extensions
} // namespace Envoy
