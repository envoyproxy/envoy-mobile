#pragma once

#include <thread>

#include "source/extensions/transport_sockets/tls/cert_validator/default_validator.h"

#include "absl/container/flat_hash_map.h"
#include "library/common/extensions/cert_validator/platform_bridge/c_types.h"

namespace Envoy {
namespace Extensions {
namespace TransportSockets {
namespace Tls {

// A certification validation implementation that uses the platform provided APIs to verify
// certificate chain. Since some platform APIs are slow blocking calls, in order not to block
// network events, this implementation creates stand-alone threads to make those calls for each
// validation.
class PlatformBridgeCertValidator : public CertValidator, Logger::Loggable<Logger::Id::connection> {
public:
  PlatformBridgeCertValidator(const Envoy::Ssl::CertificateValidationContextConfig* config,
                              SslStats& stats, const envoy_cert_validator* platform_validator);

  ~PlatformBridgeCertValidator() override;

  // CertValidator
  // These are not very meaningful interfaces for cert validator on client side and are only called
  // by server TLS context. Ideally they should be moved from CertValidator into an extended server
  // cert validator interface. And this class only extends the client interface. But their owner
  // (Tls::ContextImpl) doesn't have endpoint perspective today, so there will need more refactoring
  // to achieve this.
  void addClientValidationContext(SSL_CTX* /*context*/, bool /*require_client_cert*/) override {
    PANIC("Should not be reached");
  }
  void updateDigestForSessionId(bssl::ScopedEVP_MD_CTX& /*md*/,
                                uint8_t* /*hash_buffer[EVP_MAX_MD_SIZE]*/,
                                unsigned /*hash_length*/) override {
    PANIC("Should not be reached");
  }
  int doSynchronousVerifyCertChain(
      X509_STORE_CTX* /*store_ctx*/,
      Ssl::SslExtendedSocketInfo*
      /*ssl_extended_info*/,
      X509& /*leaf_cert*/,
      const Network::TransportSocketOptions* /*transport_socket_options*/) override {
    PANIC("Should not be reached");
  }
  absl::optional<uint32_t> daysUntilFirstCertExpires() const override { return absl::nullopt; }
  Envoy::Ssl::CertificateDetailsPtr getCaCertInformation() const override { return nullptr; }
  // Return empty string
  std::string getCaFileName() const override { return ""; }
  // Overridden to call into platform extension API asynchronously.
  ValidationResults
  doVerifyCertChain(STACK_OF(X509) & cert_chain, Ssl::ValidateResultCallbackPtr callback,
                    Ssl::SslExtendedSocketInfo* ssl_extended_info,
                    const Network::TransportSocketOptionsConstSharedPtr& transport_socket_options,
                    SSL_CTX& ssl_ctx,
                    const CertValidator::ExtraValidationContext& validation_context, bool is_server,
                    absl::string_view hostname) override;
  // As CA path will not be configured, make sure the return value won’t be SSL_VERIFY_NONE because
  // of that, so that doVerifyCertChain() will be called from the TLS stack.
  int initializeSslContexts(std::vector<SSL_CTX*> contexts,
                            bool handshaker_provides_certificates) override;

private:
  enum class ValidationFailureType {
    SUCCESS,
    FAIL_VERIFY_ERROR,
    FAIL_VERIFY_SAN,

  };
  class PendingValidation {
  public:
    PendingValidation(Event::Dispatcher& dispatcher, std::weak_ptr<PlatformBridgeCertValidator> weak_validator, const envoy_cert_validator* platform_validator, bool allow_untrusted_certificate,
		      const std::vector<envoy_data>& certs, absl::string_view hostname,
		      const std::vector<std::string>& subject_alt_names)
      : dispatcher_(dispatcher), weak_validator_(weak_validator), platform_validator_(platform_validator), allow_untrusted_certificate_(allow_untrusted_certificate), certs_(std::move(certs)), hostname_(hostname), subject_alt_names_(subject_alt_names) {}

    // Ensure that this class is never moved or copies to guarantee pointer stability.
    PendingValidation(const PendingValidation&) = delete;
    PendingValidation(PendingValidation&&) = delete;

    void postVerifyResultAndCleanUp(bool success, absl::string_view error_details,
                                    uint8_t tls_alert, ValidationFailureType failure_type);

    // Calls into platform APIs in a stand-alone thread to verify the given certs.
    // Once the validation is done, the result will be posted back to the current
    // thread to trigger callback and update verify stats.
    void verifyCertChainByPlatform();

  private:
    Event::Dispatcher& dispatcher_;
    std::weak_ptr<PlatformBridgeCertValidator> weak_validator_;
    const envoy_cert_validator* platform_validator_;
    const bool allow_untrusted_certificate_;
    const std::vector<envoy_data> certs_;
    const std::string hostname_;
    const std::vector<std::string> subject_alt_names_;
  };

  // The complete state for handling a platform verification on a worker thread.
  struct ValidationJob {
    // Only touched by the main thread.
    Ssl::ValidateResultCallbackPtr result_callback_;
    // Worker thread on which to perform validation.
    std::thread thread_;
    // Runs on the worker and must not be touched by any other thread.
    std::unique_ptr<PendingValidation> validation_;
  };

  // Called when a pending verification completes. Must be invoked on the main thread.
  void onVerificationComplete(std::thread::id thread_id,
			      std::string hostname,
			      bool success,
			      std::string error_details,
			      uint8_t tls_alert,
			      ValidationFailureType failure_type);

  const Envoy::Ssl::CertificateValidationContextConfig* config_;
  const bool allow_untrusted_certificate_;
  SslStats& stats_;
  // Latches the platform extension API.
  const envoy_cert_validator* platform_validator_;
  absl::flat_hash_map<std::thread::id, std::unique_ptr<ValidationJob>> validation_jobs_;
  // Used to vend weak pointers to worker threads.
  std::shared_ptr<PlatformBridgeCertValidator> shared_this_;
};

} // namespace Tls
} // namespace TransportSockets
} // namespace Extensions
} // namespace Envoy
