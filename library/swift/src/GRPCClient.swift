import Foundation

/// Client that supports sending and receiving gRPC traffic.
@objcMembers
public final class GRPCClient: NSObject {
  private let client: HTTPClient

  /// Create a new client instance.
  ///
  /// - parameter client: The HTTP client to use for gRPC streams.
  public init(with client: HTTPClient) {
    self.client = client
  }

  /// Send a gRPC request with the provided handler.
  ///
  /// - parameter request: The outbound gRPC request. See `GRPCRequestBuilder` for creation.
  /// - parameter handler: Handler for receiving responses.
  ///
  /// - returns: An emitter that can be used for sending more traffic over the stream.
  public func send(_ request: Request, handler: GRPCResponseHandler) -> GRPCStreamEmitter {
    let emitter = self.client.send(request, handler: handler.underlyingHandler)
    return GRPCStreamEmitter(emitter: emitter)
  }
}
