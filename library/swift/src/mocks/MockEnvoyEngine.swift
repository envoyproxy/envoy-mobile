@_implementationOnly import EnvoyEngine
import Foundation

/// Mock implementation of `EnvoyEngine`. Used internally for testing the bridging layer & mocking.
final class MockEnvoyEngine: NSObject {
  /// Closure called when `run(withConfig:)` is called.
  static var onRunWithConfig: ((_ config: EnvoyConfiguration, _ logLevel: String?) -> Void)?
  /// Closure called when `run(withConfigYAML:)` is called.
  static var onRunWithYAML: ((_ configYAML: String, _ logLevel: String?) -> Void)?
  /// Closure called when `recordCounter(_:count:)` is called.
  static var onRecordCounter: ((_ elements: String, _ count: UInt) -> Void)?
}

extension MockEnvoyEngine: EnvoyEngine {
  func run(withConfig config: EnvoyConfiguration, logLevel: String,
           onSetupComplete: @escaping () -> Void) -> Int32
  {
    MockEnvoyEngine.onRunWithConfig?(config, logLevel)
    onSetupComplete()
    return kEnvoySuccess
  }

  func run(withConfigYAML configYAML: String, logLevel: String,
           onSetupComplete: @escaping () -> Void) -> Int32
  {
    MockEnvoyEngine.onRunWithYAML?(configYAML, logLevel)
    onSetupComplete()
    return kEnvoySuccess
  }

  func startStream(with callbacks: EnvoyHTTPCallbacks) -> EnvoyHTTPStream {
    return MockEnvoyHTTPStream(handle: 0, callbacks: callbacks)
  }

  func recordCounter(_ elements: String, count: UInt) {
    MockEnvoyEngine.onRecordCounter?(elements, count)
  }
}
