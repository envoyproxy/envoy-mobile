import Foundation

/// Rules that may be used for retry policies.
/// See the `x-envoy-retry-on` Envoy header for documentation.
@objc
public enum RetryRule: Int {
  case fiveXX
  case gatewayError
  case connectFailure
  case retriableFourXX
  case refusedUpstream

  /// String representation of this rule.
  var stringValue: String {
    switch self {
    case .fiveXX:
      return "5xx"
    case .gatewayError:
      return "gateway-error"
    case .connectFailure:
      return "connect-failure"
    case .retriableFourXX:
      return "retriable-4xx"
    case .refusedUpstream:
      return "refused-upstream"
    }
  }
}

@objcMembers
public final class RetryPolicy: NSObject {
  /// Maximum number of retries that a request may be performed.
  public let maxRetryCount: UInt
  /// Whitelist of rules used for retrying.
  public let retryOn: [RetryRule]
  /// Timeout (in seconds) to apply to each retry.
  public let perRetryTimeoutSeconds: UInt?

  /// Public initializer.
  public init(maxRetryCount: UInt, retryOn: [RetryRule], perRetryTimeoutSeconds: UInt?) {
    self.maxRetryCount = maxRetryCount
    self.retryOn = retryOn
    self.perRetryTimeoutSeconds = perRetryTimeoutSeconds
  }
}
