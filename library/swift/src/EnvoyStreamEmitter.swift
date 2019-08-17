import Foundation

@objcMembers
final class EnvoyStreamEmitter {
  private let stream: EnvoyHTTPStream

  init(stream: EnvoyHTTPStream) {
    self.stream = stream
  }
}

extension EnvoyStreamEmitter: StreamEmitter {
  func sendData(_ data: Data) -> StreamEmitter {
    self.stream.sendData(data)
    return self
  }

  func sendMetadata(_ metadata: [String: [String]]) -> StreamEmitter {
    self.stream.sendMetadata(metadata)
    return self
  }

  func close(trailers: [String: [String]]) {
    self.stream.sendTrailers(trailers)
  }

  func cancel() {
    _ = self.stream.cancel()
  }
}
