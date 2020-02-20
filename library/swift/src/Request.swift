import Foundation

/// Represents an Envoy HTTP request. Use `RequestBuilder` to construct new instances.
@objcMembers
public final class Request: NSObject {
  /// Method for the request.
  public let method: RequestMethod
  /// The URL scheme for the request (i.e., "https").
  public let scheme: String
  /// The URL authority for the request (i.e., "api.foo.com").
  public let authority: String
  /// The URL path for the request (i.e., "/foo").
  public let path: String
  /// The protcol version to use for upstream requests.
  public let upstreamHttpProtocol: UpstreamHttpProtocol
  /// Headers to send with the request.
  /// Multiple values for a given name are valid, and will be sent as comma-separated values.
  public let headers: [String: [String]]
  // Retry policy to use for this request.
  public let retryPolicy: RetryPolicy?

  /// Converts the request back to a builder so that it can be modified (i.e., by a filter).
  ///
  /// - returns: A new builder including all the properties of this request.
  public func newBuilder() -> RequestBuilder {
    return RequestBuilder(request: self)
  }

  /// Internal initializer called from the builder to create a new request.
  init(method: RequestMethod,
       scheme: String,
       authority: String,
       path: String,
       upstreamHttpProtocol: UpstreamHttpProtocol,
       headers: [String: [String]] = [:],
       retryPolicy: RetryPolicy?)
  {
    self.method = method
    self.scheme = scheme
    self.authority = authority
    self.path = path
    self.upstreamHttpProtocol = upstreamHttpProtocol
    self.headers = headers
    self.retryPolicy = retryPolicy
  }
}

// MARK: - Equatable overrides

extension Request {
  public override func isEqual(_ object: Any?) -> Bool {
    guard let other = object as? Request else {
      return false
    }

    return self.method == other.method
      && self.scheme == other.scheme
      && self.authority == other.authority
      && self.path == other.path
      && self.upstreamHttpProtocol == other.upstreamHttpProtocol
      && self.headers == other.headers
      && self.retryPolicy == other.retryPolicy
  }
}
