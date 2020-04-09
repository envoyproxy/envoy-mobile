final class ResponseHeaders {
    private var headers = [String: [String]]()

    func addHeader() {} // ...

    func removeHeader() {} // ...

    var statusCode: Int? {
        return self.headers[":status"]?.first.flatMap(Int.init)
    }
}

public class Headers {
    let headers: [String: [String]]

    public func value(forName name: String) -> [String] {
        return self.headers[name]
    }
}

public class HeadersBuilder {
    private(set) var headers = [String: [String]]()

    @discardableResult
    public func add(name: String, value: String) -> Self {
        self.headers[name, default: []].append(value)
        return self
    }

    @discardableResult
    public func set(name: String, value: [String]) -> Self {
        self.headers[name] = value
        return self
    }

    @discardableResult
    public func remove(name: String) -> Self {
        self.headers[name] = nil
        return self
    }
}

public final class RequestHeadersBuilder: HeadersBuilder {
  public init(method: RequestMethod,
              scheme: String = "https",
              authority: String,
              path: String)
  {
    super.init()
    self.addHeader(name: ":method", value: method.stringValue)
    self.addHeader(name: ":scheme", value: scheme)
    self.addHeader(name: ":authority", value: authority)
    self.addHeader(name: ":path", value: scheme)
  }

  public func build() -> RequestHeaders {
      return RequestHeaders(headers: self.headers)
  }
}

public final class RequestHeaders: Headers {
  public var method: RequestMethod {
    return RequestMethod(stringValue: self.headers[":method"]!.first!)!
  }

  public var scheme: String {
      return self.headers[":scheme"]!.first!
  }

  /// Method for the request.
  public let method: RequestMethod
  /// The URL scheme for the request (i.e., "https").
  public let scheme: String
  /// The URL authority for the request (i.e., "api.foo.com").
  public let authority: String
  /// The URL path for the request (i.e., "/foo").
  public let path: String
  /// Retry policy to use for this request.
  public let retryPolicy: RetryPolicy?
  /// The protocol version to use for upstream requests.
  public let upstreamHttpProtocol: UpstreamHttpProtocol?

  public func toBuilder() -> RequestHeadersBuilder {
      // Envoy consumes this
  }
}

public final class ResponseHeaders: Headers {
    public var statusCode: Int? {
        return self.headers[":status"]?.first.flatMap(Int.init)
    }
}

public final class RequestTrailers: Headers {}

public final class ResponseTrailers: Headers {}