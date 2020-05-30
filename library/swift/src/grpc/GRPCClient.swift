import Foundation

/// Client that supports sending and receiving gRPC traffic.
@objcMembers
public final class GRPCClient: NSObject {
  private let streamClient: StreamClient

  /// Create a new gRPC client instance.
  ///
  /// - parameter streamClient: The stream client to use for gRPC streams.
  public init(streamClient: StreamClient) {
    self.streamClient = streamClient
  }

  /// Create a new gRPC inactive stream which can be used to start active streams.
  ///
  /// - returns: The new gRPC inactive stream.
  public func newGRPCStream() -> GRPCInactiveStream {
    let inactiveStream = self.streamClient.newStream()
    return GRPCInactiveStream(underlyingStream: inactiveStream)
  }
}
