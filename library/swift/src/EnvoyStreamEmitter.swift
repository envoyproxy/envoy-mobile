import Foundation

// This class will be combined with StreamEmitter to produce ActiveStream.

/// Default implementation of the `StreamEmitter` interface.
@objcMembers
final class EnvoyStreamEmitter {
  private let stream: EnvoyHTTPStream

  init(stream: EnvoyHTTPStream) {
    self.stream = stream
  }
}

extension EnvoyStreamEmitter: StreamEmitter {
  func sendData(_ data: Data) -> StreamEmitter {
    // Filters go here
    self.stream.send(data, close: false)
    return self
  }

  func close(trailers: RequestHeaders) {
    self.stream.sendTrailers(trailers)
  }

  func close(data: Data) {
    self.stream.send(data, close: true)
  }

  func cancel() {
    _ = self.stream.cancel()
  }
}
