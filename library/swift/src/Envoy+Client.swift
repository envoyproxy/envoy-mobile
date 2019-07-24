import Foundation

extension Envoy: Client {
  public func startStream(request: Request, handler: ResponseHandler) -> StreamEmitter {
    // TODO: request.makeOutboundHeaders()...
    fatalError("\(#function) is not yet implemented")
  }
}
