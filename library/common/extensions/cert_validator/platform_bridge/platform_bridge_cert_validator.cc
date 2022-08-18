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
  /*
  if (!config_->subjectAltNameMatchers().empty()) {
    for (const envoy::extensions::transport_sockets::tls::v3::SubjectAltNameMatcher& matcher :
         cert_validation_config->subjectAltNameMatchers()) {
      auto san_matcher = createStringSanMatcher(matcher);
      if (san_matcher == nullptr) {
        throw EnvoyException(
            absl::StrCat("Failed to create string SAN matcher of type ", matcher.san_type()));
      }
      subject_alt_name_matchers_.push_back(std::move(san_matcher));
    }
  }

  if (!config_->verifyCertificateHashList().empty()) {
    for (auto hash : cert_validation_config->verifyCertificateHashList()) {
      // Remove colons from the 95 chars long colon-separated "fingerprint"
      // in order to get the hex-encoded string.
      if (hash.size() == 95) {
        hash.erase(std::remove(hash.begin(), hash.end(), ':'), hash.end());
      }
      const auto& decoded = Hex::decode(hash);
      if (decoded.size() != SHA256_DIGEST_LENGTH) {
        throw EnvoyException(absl::StrCat("Invalid hex-encoded SHA-256 ", hash));
      }
      verify_certificate_hash_list_.push_back(decoded);
    }
  }

  if (!config_->verifyCertificateSpkiList().empty()) {
    for (const auto& hash : cert_validation_config->verifyCertificateSpkiList()) {
      const auto decoded = Base64::decode(hash);
      if (decoded.size() != SHA256_DIGEST_LENGTH) {
        throw EnvoyException(absl::StrCat("Invalid base64-encoded SHA-256 ", hash));
      }
      verify_certificate_spki_list_.emplace_back(decoded.begin(), decoded.end());
    }
  }
  */
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
                std::move(callback), transport_socket_options, host_name, allows_expired_cert_);
  std::thread::id t_id = t.get_id();
  validation_threads_[t_id] = std::move(t);
  return {ValidationResults::ValidationStatus::Pending, absl::nullopt, absl::nullopt};
}

void PlatformBridgeCertValidator::verifyCertChainByPlatform(
    std::vector<envoy_data> certs, Ssl::ValidateResultCallbackPtr callback,
    const Network::TransportSocketOptionsConstSharedPtr transport_socket_options,
    const std::string host_name, const bool allow_expired_cert) {
  // In a stand alone thread.
  envoy_data leaf_cert = copy_envoy_data(certs[0]);
  envoy_cert_validation_result result = platform_bridge_api_->validate_cert(
      certs.data(), certs.size(), host_name.c_str(), allow_expired_cert);
  std::thread::id t_id = std::this_thread::get_id();
  callback->dispatcher().post([this, result, cb = callback.release(), leaf_cert,
                               transport_socket_options, t_id]() {
    // Back to network thread.
    ASSERT(!validation_threads_[t_id].joinable());
    validation_threads_.erase(t_id);
    auto scoped_cb = Ssl::ValidateResultCallbackPtr(cb);
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

    std::string error_details;
    uint8_t tls_alert = SSL_AD_CERTIFICATE_UNKNOWN;
    success = verifyCertAndUpdateStatus(nullptr, cert.get(), transport_socket_options.get(),
                                        &error_details, &tls_alert);

    // Verify that host name matches leaf cert.
    if (success) {
      cb->onCertValidationResult(true, "", SSL_AD_CERTIFICATE_UNKNOWN);
    } else {
      cb->onCertValidationResult(false, error_details, tls_alert);
    }
  });
}

} // namespace Tls
} // namespace TransportSockets
} // namespace Extensions
} // namespace Envoy
