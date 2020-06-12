import Foundation

/// Trailers representing an inbound gRPC response.
@objcMembers
public final class GRPCResponseTrailers: Headers {
  /// gRPC status code received with the response.
  public private(set) lazy var grpcStatus: Int? =
    self.value(forName: "grpc-status")?.first.flatMap(Int.init)

  /// gRPC message received with the response.
  public private(set) lazy var grpcMessage: String? =
    self.value(forName: "grpc-message")?.first

  /// Convert the headers back to a builder for mutation.
  ///
  /// - returns: The new builder.
  public func toGRPCResponseTrailersBuilder() -> GRPCResponseTrailersBuilder {
    return GRPCResponseTrailersBuilder(headers: self.headers)
  }
}
