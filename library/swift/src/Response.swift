import Foundation

/// Represents an Envoy HTTP response. Use `ResponseBuilder` to construct new instances.
@objcMembers
public final class Response: NSObject {
  /// Status code returned with the response.
  public let statusCode: Int
  /// Headers returned with the response.
  /// Multiple values for a given name are valid, and will be sent as comma-separated values.
  public let headers: [String: [String]]
  /// Trailers returned with the response.
  /// Multiple values for a given name are valid, and will be sent as comma-separated values.
  public let trailers: [String: [String]]
  /// Serialized data returned as the body of the response.
  public let body: Data?

  /// Converts the response back to a builder so that it can be modified (i.e., by a filter).
  ///
  /// - returns: A new builder including all the properties of this response.
  public func newBuilder() -> ResponseBuilder {
    return ResponseBuilder(response: self)
  }

  /// Internal initializer called from the builder to create a new response.
  init(statusCode: Int,
       headers: [String: [String]] = [:],
       trailers: [String: [String]] = [:],
       body: Data?)
  {
    self.statusCode = statusCode
    self.headers = headers
    self.trailers = trailers
    self.body = body
  }
}
