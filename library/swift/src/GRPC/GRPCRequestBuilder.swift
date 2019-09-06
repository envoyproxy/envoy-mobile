import EnvoyInterfaces
import Foundation

/// <#function description#>
@objcMembers
public final class GRPCRequestBuilder: NSObject {
    private let underlyingBuilder: RequestBuilder

    public init(path: String, authority: String, useHTTPS: Bool = true) {
        self.underlyingBuilder = RequestBuilder(method: .post,
                                                scheme: useHTTPS ? "https" : "http",
                                                authority: authority,
                                                path: path)
        self.underlyingBuilder.addHeader(name: "content-type", value: "application/grpc")
    }

    /// <#function description#>
    @discardableResult
    public func addHeader(name: String, value: String) -> GRPCRequestBuilder {
        self.underlyingBuilder.addHeader(name: name, value: value)
        return self
    }

    /// <#function description#>
    @discardableResult
    public func removeHeaders(name: String) -> GRPCRequestBuilder {
        self.underlyingBuilder.removeHeaders(name: name)
        return self
    }

    /// <#function description#>
    @discardableResult
    public func removeHeader(name: String, value: String) -> GRPCRequestBuilder {
        self.underlyingBuilder.removeHeader(name: name, value: value)
        return self
    }

    /// <#function description#>
    @discardableResult
    public func addTimeoutMS(_ timeoutMS: UInt?) -> GRPCRequestBuilder {
        let headerName = "grpc-timeout"
        if let timeoutMS = timeoutMS {
            self.addHeader(name: headerName, value: "\(timeoutMS)m")
        } else {
            self.removeHeaders(name: headerName)
        }

        return self
    }

    /// <#function description#>
    public func build() -> Request {
        return self.underlyingBuilder.build()
    }
}
