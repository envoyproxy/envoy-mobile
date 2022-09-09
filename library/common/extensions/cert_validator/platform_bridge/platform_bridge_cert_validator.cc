#include "library/common/extensions/cert_validator/platform_bridge/platform_bridge_cert_validator.h"

#include <list>
#include <type_traits>

#include "library/common/data/utility.h"

namespace Envoy {
namespace Extensions {
namespace TransportSockets {
namespace Tls {

PlatformBridgeCertValidator::PlatformBridgeCertValidator(
    const Envoy::Ssl::CertificateValidationContextConfig* config, SslStats& stats,
    const envoy_cert_validator* platform_validator)
    : config_(config), stats_(stats), platform_validator_(platform_validator) {
  ENVOY_BUG(config != nullptr && config->caCert().empty() &&
                config->certificateRevocationList().empty(),
            "Invalid cert validation context config.");
  if (config_ != nullptr) {
    allow_untrusted_certificate_ = config_->trustChainVerification() ==
                                   envoy::extensions::transport_sockets::tls::v3::
                                       CertificateValidationContext::ACCEPT_UNTRUSTED;
  }
}

PlatformBridgeCertValidator::~PlatformBridgeCertValidator() {
  // Wait for validation threads to finish.
  for (auto& [id, thread] : validation_threads_) {
    if (thread.joinable()) {
      thread.join();
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
    bool is_server) {
  ASSERT(!is_server);
  if (sk_X509_num(&cert_chain) == 0) {
    if (ssl_extended_info) {
      ssl_extended_info->setCertificateValidationStatus(
          Envoy::Ssl::ClientValidationStatus::NotValidated);
    }
    const char* error = "verify cert failed: empty cert chain";
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
    unsigned char* der = nullptr;
    int der_length = i2d_X509(cert, &der);
    ASSERT(der_length > 0 && der != nullptr);

    absl::string_view cert_str(reinterpret_cast<char*>(der), static_cast<size_t>(der_length));
    certs.push_back(Data::Utility::copyToBridgeData(cert_str));
    OPENSSL_free(der);
  }

  std::string host_name; // validation_context.host_name);
  if (transport_socket_options != nullptr &&
      !transport_socket_options->verifySubjectAltNameListOverride().empty()) {
    host_name = transport_socket_options->verifySubjectAltNameListOverride()[0];
  }
  std::thread verification_thread(&PlatformBridgeCertValidator::verifyCertChainByPlatform, this,
                                  std::move(certs), std::move(callback), transport_socket_options,
                                  host_name);
  std::thread::id thread_id = verification_thread.get_id();
  validation_threads_[thread_id] = std::move(verification_thread);
  return {ValidationResults::ValidationStatus::Pending, absl::nullopt, absl::nullopt};
}

void PlatformBridgeCertValidator::verifyCertChainByPlatform(
    std::vector<envoy_data> certs, Ssl::ValidateResultCallbackPtr callback,
    const Network::TransportSocketOptionsConstSharedPtr transport_socket_options,
    const std::string host_name) {
  ENVOY_LOG(debug, "Start verifyCertChainByPlatform for host {}", host_name);
  // This is running in a stand alone thread other than the engine thread.
  ASSERT(!certs.empty());
  envoy_data leaf_cert_der = certs[0];
  bssl::UniquePtr<X509> leaf_cert(d2i_X509(
      nullptr, const_cast<const unsigned char**>(&leaf_cert_der.bytes), leaf_cert_der.length));
  envoy_cert_validation_result result =
      platform_validator_->validate_cert(certs.data(), certs.size(), host_name.c_str());
  bool success = (result.result == ENVOY_SUCCESS);
  absl::string_view error_details;
  uint8_t tls_alert = SSL_AD_CERTIFICATE_UNKNOWN;
  OptRef<Stats::Counter> counter_ref;
  if (success) {
    // Verify that host name matches leaf cert.
    success = DefaultCertValidator::verifySubjectAltName(
        leaf_cert.get(), (transport_socket_options != nullptr
                              ? transport_socket_options->verifySubjectAltNameListOverride()
                              : std::vector<std::string>{host_name}));
    if (!success) {
      error_details = "PlatformBridgeCertValidator_verifySubjectAltName failed: SNI mismatch.";
      if (!allow_untrusted_certificate_) {
        tls_alert = SSL_AD_BAD_CERTIFICATE;
      } else {
        success = true;
      }
      ENVOY_LOG(debug, error_details);
      counter_ref = makeOptRef(stats_.fail_verify_san_);
    }
  } else {
    if (!allow_untrusted_certificate_) {
      error_details = result.error_details;
      tls_alert = result.tls_alert;
    } else {
      success = true;
    }
    counter_ref = makeOptRef(stats_.fail_verify_error_);
    ENVOY_LOG(debug, result.error_details);
  }

  std::thread::id thread_id = std::this_thread::get_id();
  callback->dispatcher().post([this, success, error = std::string(error_details), tls_alert,
                               counter_ref, cb = callback.release(), thread_id, host_name]() {
    ENVOY_LOG(debug, "Get validation result for {} from platform", host_name);
    validation_threads_[thread_id].join();
    validation_threads_.erase(thread_id);
    Ssl::ValidateResultCallbackPtr scoped_cb(cb);
    if (counter_ref.has_value()) {
      const_cast<Stats::Counter&>(counter_ref.ref()).inc();
    }
    cb->onCertValidationResult(success, error, tls_alert);
  });

  ENVOY_LOG(trace,
            "Finished platform cert validation for {}, post result callback to network thread",
            host_name);
  platform_validator_->validation_done();
}

} // namespace Tls
} // namespace TransportSockets
} // namespace Extensions
} // namespace Envoy
