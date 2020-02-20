import Foundation

/// Builder used for constructing instances of `Request` types.
@objcMembers
public final class RequestBuilder: NSObject {
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
  public private(set) var headers: [String: [String]] = [:]
  // Retry policy to use for this request.
  public private(set) var retryPolicy: RetryPolicy?

  // MARK: - Initializers

  /// Internal initializer used for converting a request back to a builder.
  init(request: Request) {
    self.method = request.method
    self.scheme = request.scheme
    self.authority = request.authority
    self.path = request.path
    self.upstreamHttpProtocol = request.upstreamHttpProtocol
    self.headers = request.headers
    self.retryPolicy = request.retryPolicy
  }

  /// Public initializer.
  public init(method: RequestMethod,
              scheme: String = "https",
              authority: String,
              path: String,
              upstreamHttpProtocol: UpstreamHttpProtocol)
  {
    self.method = method
    self.scheme = scheme
    self.authority = authority
    self.path = path
    self.upstreamHttpProtocol = upstreamHttpProtocol
  }

  // MARK: - Builder functions

  @discardableResult
  public func addHeader(name: String, value: String) -> RequestBuilder {
    self.headers[name, default: []].append(value)
    return self
  }

  @discardableResult
  public func removeHeaders(name: String) -> RequestBuilder {
    self.headers.removeValue(forKey: name)
    return self
  }

  @discardableResult
  public func removeHeader(name: String, value: String) -> RequestBuilder {
    self.headers[name]?.removeAll(where: { $0 == value })
    if self.headers[name]?.isEmpty == true {
      self.headers.removeValue(forKey: name)
    }

    return self
  }

  @discardableResult
  public func addRetryPolicy(_ retryPolicy: RetryPolicy) -> RequestBuilder {
    self.retryPolicy = retryPolicy
    return self
  }

  public func build() -> Request {
    return Request(method: self.method,
                   scheme: self.scheme,
                   authority: self.authority,
                   path: self.path,
                   upstreamHttpProtocol: self.upstreamHttpProtocol,
                   headers: self.headers,
                   retryPolicy: self.retryPolicy)
  }
}

// MARK: - Objective-C helpers

extension Request {
  /// Convenience builder function to allow for cleaner Objective-C syntax.
  ///
  /// For example:
  ///
  /// Request *req = [Request withMethod:RequestMethodGet (...) build:^(RequestBuilder *builder) {
  ///   [builder addHeaderWithName:@"x-some-header" value:@"foo"];
  /// }];
  @objc
  public static func with(method: RequestMethod,
                          scheme: String,
                          authority: String,
                          path: String,
                          upstreamHttpProtocol: UpstreamHttpProtocol,
                          build: (RequestBuilder) -> Void)
    -> Request
  {
    let builder = RequestBuilder(method: method, scheme: scheme,
                                 authority: authority, path: path, upstreamHttpProtocol: upstreamHttpProtocol)
    build(builder)
    return builder.build()
  }
}
