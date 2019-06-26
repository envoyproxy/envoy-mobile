import Foundation
import EnvoyEngine

public class Envoy {
  private let runner: EnvoyRunner

  public var isRunning: Bool {
    runner.isExecuting()
  }

  public var isTerminated: Bool {
    runner.isFinished()
  }

  public init(config: String, logLevel: String) {
    runner = EnvoyRunner(config: config, logLevel: logLevel)
    runner.start()
  }

  public convenience init(config: String) {
    self.init(config: config, logLevel: "info")
  }

  private class EnvoyRunner : Thread {
    let config: String
    let logLevel: String

    init(config: String, logLevel: String) {
      self.config = config
      self.logLevel = logLevel
    }

    func main() {
      EngineEngine.run(config: config, logLevel: logLevel)
    }
  }
}
