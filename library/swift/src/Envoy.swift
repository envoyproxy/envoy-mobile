import Envoy
import Foundation

@objcMembers
public final class Envoy: NSObject {
  private let streamEngine: EnvoyEngineStreamInterface.Type
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
  public func startStream(request: Request, handler: ResponseHandler) -> StreamEmitter? {
    let observer = EnvoyObserver(handler: handler)
    var stream = self.streamEngine.startStream(with: observer)

    switch stream.status {
    case .success:
      return EnvoyStreamEmitter(stream: &stream)
    case .failure:
      defer { handler.onError(EnvoyError.streamCreationFailed) }
      return nil
    }
  }
}

extension ResponseHandler {
  func transformHeaders(_ headers: [[String: String]]?) -> [String: [String]] {
    guard let headers = headers else {
      return [:]
    }

    return headers.reduce(into: [String: [String]]()) { result, next in
      next.forEach { result[$0.key, default: []].append($0.value) }
    }
  }

  func statusCode(fromHeaders headers: [String: [String]]) -> Int? {
    return headers[":status"]?
      .compactMap(Int.init)
      .first
  }
}

private extension EnvoyObserver {
  convenience init(handler: ResponseHandler) {
    self.init()

    self.onHeaders = { headers, endStream in
      let headers = handler.transformHeaders(headers)
      handler.onHeaders(headers,
                        statusCode: handler.statusCode(fromHeaders: headers) ?? 0,
                        endStream: endStream)
    }

    self.onData = { data, endStream in
      handler.onData(data, endStream: endStream)
    }

    self.onMetadata = { metadata in
      handler.onMetadata(handler.transformHeaders(metadata))
    }

    self.onTrailers = { trailers in
      handler.onTrailers(handler.transformHeaders(trailers))
    }

    self.onError = { error in
      handler.onError(EnvoyError(errorCode: Int(clamping: error.errorCode.rawValue),
                                 message: error.message, cause: error))
    }
  }
}
