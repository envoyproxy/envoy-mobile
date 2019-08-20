import Foundation

/// Envoy's implementation of `Client`, buildable using `EnvoyBuilder`.
@objcMembers
public final class Envoy: NSObject {
  private let engine: EnvoyEngine
  private let runner: RunnerThread

  /// Indicates whether this Envoy instance is currently active and running.
  public var isRunning: Bool {
    return self.runner.isExecuting
  }

  /// Indicates whether the Envoy instance is terminated.
  public var isTerminated: Bool {
    return self.runner.isFinished
  }

  /// Initialize a new Envoy instance using a string configuration.
  ///
  /// - parameter configYAML: Configuration YAML to use for starting Envoy.
  /// - parameter logLevel:   Log level to use for this instance.
  /// - parameter engine:     The underlying engine to use for starting Envoy.
  init(configYAML: String, logLevel: LogLevel = .info, engine: EnvoyEngine) {
    self.engine = engine
    self.runner = RunnerThread(configYAML: configYAML, logLevel: logLevel, engine: engine)
    self.runner.start()
  }

  // MARK: - Private

  private final class RunnerThread: Thread {
    private let engine: EnvoyEngine
    private let configYAML: String
    private let logLevel: LogLevel

    init(configYAML: String, logLevel: LogLevel, engine: EnvoyEngine) {
      self.configYAML = configYAML
      self.logLevel = logLevel
      self.engine = engine
    }

    override func main() {
      self.engine.run(withConfig: self.configYAML, logLevel: self.logLevel.stringValue)
    }
  }
}

extension Envoy: Client {
  public func startStream(with request: Request, handler: ResponseHandler) -> StreamEmitter {
    let httpStream = self.engine.startStream(with: handler.underlyingObserver)
    NSLog("Sending request with headers: \(request.outboundHeaders())")
    httpStream.sendHeaders(request.outboundHeaders(), close: false)
    return EnvoyStreamEmitter(stream: httpStream)
  }

  public func sendUnary(_ request: Request, body: Data?,
                        trailers: [String: [String]], handler: ResponseHandler)
  {
    let emitter = self.startStream(with: request, handler: handler)
    if let body = body {
      emitter.sendData(body)
    }

    emitter.close(trailers: trailers)
  }
}
