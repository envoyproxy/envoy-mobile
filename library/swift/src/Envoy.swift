import Envoy
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
  /// - parameter config:   Configuration file that is recognizable by Envoy (YAML).
  /// - parameter logLevel: Log level to use for this instance.
  public init(config: String, logLevel: LogLevel = .info) {
    self.runner = RunnerThread(config: config, logLevel: logLevel)
    self.runner.start()
  }

  // MARK: - RunnerThread

  private final class RunnerThread: Thread {
    private let config: String
    private let logLevel: LogLevel

    init(config: String, logLevel: LogLevel) {
      self.config = config
      self.logLevel = logLevel
    }

    override func main() {
      EnvoyEngine.run(withConfig: self.config, logLevel: self.logLevel.stringValue)
    }
  }
}

extension Envoy: Client {
  public func startStream(request: Request, handler: ResponseHandler) -> StreamEmitter? {
    let observer = EnvoyObserver(handler: handler)
    var stream = EnvoyEngine.startStream(with: observer)

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
      guard let data = data else {
        return
      }

      handler.onData(data, endStream: endStream)
    }

    self.onMetadata = { metadata in
      handler.onMetadata(handler.transformHeaders(metadata))
    }

    self.onTrailers = { trailers in
      handler.onTrailers(handler.transformHeaders(trailers))
    }

    self.onError = { error in
      guard let error = error else {
        return
      }

      handler.onError(EnvoyError(errorCode: Int(clamping: error.errorCode.rawValue),
                                 message: error.message, cause: error))
    }
  }
}
