#include "library/common/extensions/cert_validator/platform_bridge/platform_bridge_cert_validator.h"

#include "library/common/data/utility.h"

namespace Envoy {
namespace Extensions {
namespace TransportSockets {
namespace Tls {

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
    onVerifyError(nullptr, error);
    return {ValidationResults::ValidationStatus::Failed, absl::nullopt, error};
  }
  if (callback == nullptr) {
    callback = ssl_extended_info->createValidateResultCallback();
  }

  std::vector<envoy_data> certs;
  for (uint64_t i = 0; i < sk_X509_num(&cert_chain); i++) {
    X509* cert = sk_X509_value(&cert_chain, i);
    const int der_length = i2d_X509(cert, nullptr);
    std::unique_ptr<char[]> der(new char[static_cast<size_t>(der_length)]);
    certs[i] = Data::Utility::copyToBridgeData({der.get(), static_cast<size_t>(der_length)});
  }

  std::string host_name; // validation_context.host_name);
  std::thread t(&PlatformBridgeCertValidator::verifyCertChainByPlatform, this, std::move(certs),
                std::move(callback), transport_socket_options, host_name);
  std::thread::id t_id = t.get_id();
  validation_threads_[t_id] = std::move(t);
  return {ValidationResults::ValidationStatus::Pending, absl::nullopt, absl::nullopt};
}

void PlatformBridgeCertValidator::verifyCertChainByPlatform(
    std::vector<envoy_data> certs, Ssl::ValidateResultCallbackPtr callback,
    const Network::TransportSocketOptionsConstSharedPtr transport_socket_options,
    const std::string host_name) {
  // In a stand alone thread.
  envoy_data leaf_cert = copy_envoy_data(certs[0]);
  envoy_cert_validation_result result =
      platform_bridge_api_->validate_cert(certs.data(), certs.size(), host_name.c_str());
  std::thread::id t_id = std::this_thread::get_id();
  callback->dispatcher().post([this, result, cb = callback.release(), leaf_cert,
                               transport_socket_options, t_id, host_name]() {
    // Back to network thread.
    ASSERT(!validation_threads_[t_id].joinable());
    validation_threads_.erase(t_id);
    Ssl::ValidateResultCallbackPtr scoped_cb(cb);
    bool success = (result.result == ENVOY_SUCCESS);
    if (!success) {
      onVerifyError(nullptr, result.error_details);
      if (allow_untrusted_certificate_) {
        cb->onCertValidationResult(true, "", SSL_AD_CERTIFICATE_UNKNOWN);
      }
      cb->onCertValidationResult(false, result.error_details, result.tls_alert);
      return;
    }
    bssl::UniquePtr<X509> cert(
        d2i_X509(nullptr, const_cast<const unsigned char**>(&leaf_cert.bytes), leaf_cert.length));
    release_envoy_data(leaf_cert);

    // Verify that host name matches leaf cert.
    success = verifySubjectAltName(cert.get(), {host_name});
    if (success || allow_untrusted_certificate_) {
      cb->onCertValidationResult(true, "", SSL_AD_CERTIFICATE_UNKNOWN);
    } else {
      const char* error_details =
          "PlatformBridgeCertValidator_verifySubjectAltName failed: SNI mismatch.";
      onVerifyError(nullptr, error_details);
      cb->onCertValidationResult(false, error_details, SSL_AD_BAD_CERTIFICATE);
    }
  });
}

} // namespace Tls
} // namespace TransportSockets
} // namespace Extensions
} // namespace Envoy
