import Foundation

/// Builder used for constructing instances of `Request` types.
@objcMembers
public final class RequestBuilder: NSObject {
  /// URL for the request.
  public private(set) var url: URL
  /// Method for the request.
  public private(set) var method: RequestMethod
  /// Headers to send with the request.
  /// Multiple values for a given name are valid, and will be sent as comma-separated values.
  public private(set) var headers: [String: [String]] = [:]
  /// Trailers to send with the request.
  /// Multiple values for a given name are valid, and will be sent as comma-separated values.
  public private(set) var trailers: [String: [String]] = [:]
  // Serialized data to send as the body of the request.
  public private(set) var body: Data?
  // Retry policy to use for this request.
  public private(set) var retryPolicy: RetryPolicy?

  // MARK: - Initializers

  /// Internal initializer used for converting a request back to a builder.
  init(request: Request) {
    self.url = request.url
    self.method = request.method
    self.headers = request.headers
    self.trailers = request.trailers
    self.body = request.body
    self.retryPolicy = request.retryPolicy
  }

  /// Public initializer.
  public init(url: URL, method: RequestMethod) {
    self.url = url
    self.method = method
  }

  // MARK: - Builder functions

  @discardableResult
  public func addHeader(name: String, value: String) -> RequestBuilder {
    self.headers[name, default: []].append(value)
    return self
  }

  @discardableResult
  public func removeHeader(name: String) -> RequestBuilder {
    self.headers.removeValue(forKey: name)
    return self
  }

  @discardableResult
  public func addTrailer(name: String, value: String) -> RequestBuilder {
    self.trailers[name, default: []].append(value)
    return self
  }

  @discardableResult
  public func removeTrailer(name: String) -> RequestBuilder {
    self.trailers.removeValue(forKey: name)
    return self
  }

  @discardableResult
  public func addBody(_ body: Data?) -> RequestBuilder {
    self.body = body
    return self
  }

  @discardableResult
  public func addRetryPolicy(_ retryPolicy: RetryPolicy) -> RequestBuilder {
    self.retryPolicy = retryPolicy
    return self
  }

  public func build() -> Request {
    return Request(method: self.method,
                   url: self.url,
                   headers: self.headers,
                   trailers: self.trailers,
                   body: self.body,
                   retryPolicy: self.retryPolicy)
  }
}

// MARK: - Objective-C helpers

extension Request {
  /// Convenience builder function to allow for cleaner Objective-C syntax.
  ///
  /// For example:
  ///
  /// Request *request = [Request withUrl:url method:RequestMethodGet build:^(RequestBuilder *builder) {
  ///   [builder addBody:bodyData];
  ///   [builder addRetryPolicy:retryPolicy];
  /// }];
  @objc
  public static func with(url: URL, method: RequestMethod, build: (RequestBuilder) -> Void) -> Request {
    let builder = RequestBuilder(url: url, method: method)
    build(builder)
    return builder.build()
  }
}
