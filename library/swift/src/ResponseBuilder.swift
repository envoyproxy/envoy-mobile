import Foundation

/// Builder used for constructing instances of `Response` types.
@objcMembers
public final class ResponseBuilder: NSObject {
  /// Status code returned with the response.
  public private(set) var statusCode: Int = 200
  /// Headers returned with the response.
  /// Multiple values for a given name are valid, and will be sent as comma-separated values.
  public private(set) var headers: [String: [String]] = [:]
  /// Trailers returned with the response.
  /// Multiple values for a given name are valid, and will be sent as comma-separated values.
  public private(set) var trailers: [String: [String]] = [:]
  /// Serialized data returned as the body of the response.
  public private(set) var body: Data?

  // MARK: - Initializers

  /// Internal initializer used for converting a response back to a builder.
  convenience init(response: Response) {
    self.init()
    self.statusCode = response.statusCode
    self.headers = response.headers
    self.trailers = response.trailers
    self.body = response.body
  }

  // MARK: - Builder functions

  @discardableResult
  public func addStatusCode(_ statusCode: Int) -> ResponseBuilder {
    self.statusCode = statusCode
    return self
  }

  @discardableResult
  public func addHeader(name: String, value: String) -> ResponseBuilder {
    self.headers[name, default: []].append(value)
    return self
  }

  @discardableResult
  public func removeHeaders(name: String) -> ResponseBuilder {
    self.headers.removeValue(forKey: name)
    return self
  }

  @discardableResult
  public func removeHeader(name: String, value: String) -> ResponseBuilder {
    self.headers[name]?.removeAll(where: { $0 == value })
    if self.headers[name]?.isEmpty == true {
      self.headers.removeValue(forKey: name)
    }

    return self
  }

  @discardableResult
  public func addTrailer(name: String, value: String) -> ResponseBuilder {
    self.trailers[name, default: []].append(value)
    return self
  }

  @discardableResult
  public func removeTrailer(name: String) -> ResponseBuilder {
    self.trailers.removeValue(forKey: name)
    return self
  }

  @discardableResult
  public func removeTrailers(named name: String, value: String) -> ResponseBuilder {
    self.trailers[name]?.removeAll(where: { $0 == value })
    if self.trailers[name]?.isEmpty == true {
      self.trailers.removeValue(forKey: name)
    }

    return self
  }

  @discardableResult
  public func addBody(_ body: Data?) -> ResponseBuilder {
    self.body = body
    return self
  }

  public func build() -> Response {
    return Response(statusCode: self.statusCode,
                    headers: self.headers,
                    trailers: self.trailers,
                    body: self.body)
  }
}

// MARK: - Objective-C helpers

extension Response {
  /// Convenience builder function to allow for cleaner Objective-C syntax.
  ///
  /// For example:
  ///
  /// Response *res = [Response withBuild:^(ResponseBuilder *builder) {
  ///   [builder addBody:bodyData];
  ///   [builder addHeaderWithName:@"x-some-header" value:@"foo"];
  ///   [builder addTrailerWithName:@"x-some-trailer" value:@"foo"];
  /// }];
  @objc
  public static func with(build: (ResponseBuilder) -> Void) -> Response {
    let builder = ResponseBuilder()
    build(builder)
    return builder.build()
  }
}
