@_implementationOnly import EnvoyEngine
import Foundation

/// Internal no-op mock implementation of the engine's `EnvoyHTTPStream`.
final class MockEnvoyHTTPStream: EnvoyHTTPStream {
  /// Callbacks associated with the stream.
  let callbacks: EnvoyHTTPCallbacks
  let explicitBuffering: Bool

  init(handle: Int, callbacks: EnvoyHTTPCallbacks, explicitBuffering: Bool) {
    self.callbacks = callbacks
    self.explicitBuffering = explicitBuffering
  }

  func sendHeaders(_ headers: [String: [String]], close: Bool) {}

  func read(_ data: NSMutableData) {}

  func send(_ data: Data, close: Bool) {}

  func sendTrailers(_ trailers: [String: [String]]) {}

  func cancel() -> Int32 { return 0 }

  func cleanUp() {}
}
