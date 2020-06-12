import Foundation

/// Builder used for constructing instances of `GRPCResponseTrailers`.
@objcMembers
public final class GRPCResponseTrailersBuilder: HeadersBuilder {
  /// Initialize a new instance of the builder.
  public convenience init() {
    self.init(headers: [:])
  }

  /// Add a gRPC status to the response headers.
  ///
  /// - parameter status: The gRPC status to add.
  ///
  /// - returns: This builder.
  public func addGrpcStatus(_ status: Int) -> GRPCResponseTrailersBuilder {
    self.internalSet(name: "grpc-status", value: ["\(status)"])
    return self
  }

  /// Add a gRPC message to the response headers.
  ///
  /// - parameter message: The gRPC message to add.
  ///
  /// - returns: This builder.
  public func addGrpcMessage(_ message: String) -> GRPCResponseTrailersBuilder {
    self.internalSet(name: "grpc-message", value: ["\(message)"])
    return self
  }

  /// Build the response trailers using the current builder.
  ///
  /// - returns: New instance of response trailers.
  public func build() -> GRPCResponseTrailers {
    return GRPCResponseTrailers(headers: self.headers)
  }
}
