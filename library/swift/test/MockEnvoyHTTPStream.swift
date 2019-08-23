import Envoy
import Foundation

final class MockEnvoyHTTPStream: NSObject, EnvoyHTTPStream {
  static var onHeaders: (([String: [String]], Bool) -> Void)?
  static var onData: ((Data, Bool) -> Void)?
  static var onTrailers: (([String: [String]]) -> Void)?

  init(handle: UInt64, observer: EnvoyObserver) {}

  func sendHeaders(_ headers: [String: [String]], close: Bool) {
    MockEnvoyHTTPStream.onHeaders?(headers, close)
  }

  func send(_ data: Data, close: Bool) {
    MockEnvoyHTTPStream.onData?(data, close)
  }

  func sendMetadata(_ metadata: [String: [String]]) {}

  func sendTrailers(_ trailers: [String: [String]]) {
    MockEnvoyHTTPStream.onTrailers?(trailers)
  }

  func cancel() -> Int32 {
    return 0
  }
}
