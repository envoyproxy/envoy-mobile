import Envoy
import Foundation

final class EnvoyStreamEmitter {
  private let stream: UnsafeMutablePointer<EnvoyStream>
  private lazy var isClosed = Atomic(false)

  init(stream: UnsafeMutablePointer<EnvoyStream>) {
    self.stream = stream
  }
}

extension EnvoyStreamEmitter: StreamEmitter {
  func isActive() -> Bool {
    return !self.isClosed.value
  }

  func sendData(_ data: Data) throws -> StreamEmitter {
    // TODO: wrap EnvoyEngine in a protocol
    switch EnvoyEngine.send(data, to: self.stream, close: false) {
    case .success:
      return self
    case .failure:
      throw EnvoyError.streamSendFailed
    }
  }

  func sendMetadata(_ metadata: [String: [String]]) throws -> StreamEmitter {
//    let metadata = zip(metadata.keys, metadata.values)
    switch EnvoyEngine.sendMetadata(metadata, to: self.stream, close: false) {
    case .success:
      return self
    case .failure:
      throw EnvoyError.streamSendFailed
    }
  }

  func close(trailers: [String: [String]]) throws {
    switch EnvoyEngine.sendTrailers(trailers, to: self.stream, close: true) {
    case .success:
      break
    case .failure:
      throw EnvoyError.closeStreamFailed
    }
  }

  func cancel() throws {
    switch EnvoyEngine.locallyClose(self.stream) {
    case .success:
      break
    case .failure:
      throw EnvoyError.closeStreamFailed
    }
  }
}
