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
      throw Error.streamSendFailed
    }
  }

  func sendMetadata(_ metadata: [String: [String]]) throws -> StreamEmitter {
//    let metadata = zip(metadata.keys, metadata.values)
    switch EnvoyEngine.sendMetadata(metadata, to: self.stream, close: false) {
    case .success:
      return self
    case .failure:
      throw Error.streamSendFailed
    }
  }

  func close(trailers: [String: [String]]) throws {

  }

  func cancel() throws {

  }
}
