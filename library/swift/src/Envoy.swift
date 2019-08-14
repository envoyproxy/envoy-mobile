import Foundation

@objcMembers
public final class Envoy: NSObject {
  private let runner: RunnerThread

  /// Indicates whether this Envoy instance is currently active and running.
  public var isRunning: Bool {
    return self.runner.isExecuting
  }

  /// Indicates whether the Envoy instance is terminated.
  public var isTerminated: Bool {
    return self.runner.isFinished
  }

  /// Initialize a new Envoy instance.
  ///
  /// - parameter config:   Configuration to use for starting Envoy.
  /// - parameter logLevel: Log level to use for this instance.
  public init(config: Configuration, logLevel: LogLevel = .info) {
    self.runner = RunnerThread(config: try! config.build(), logLevel: logLevel)
    self.runner.start()
  }

  private final class RunnerThread: Thread {
    private let config: String
    private let logLevel: LogLevel

    init(config: String, logLevel: LogLevel) {
      self.config = config
      self.logLevel = logLevel
    }

    override func main() {
      EnvoyEngineImpl.run(withConfig: self.config, logLevel: self.logLevel.stringValue)
    }
  }
}
