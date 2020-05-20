import Foundation

/// Envoy's implementation of `HTTPClient`, buildable using `EnvoyClientBuilder`.
@objcMembers
public final class EnvoyClient: NSObject {
  private let engine: EnvoyEngine
  private let filterRegistry: FilterRegistry

  private enum ConfigurationType {
    case yaml(String)
    case typed(EnvoyConfiguration)
  }

  private init(configType: ConfigurationType, logLevel: LogLevel, engine: EnvoyEngine,
               filterRegistry: FilterRegistry)
  {
    self.engine = engine
    self.filterRegistry = filterRegistry
    super.init()

    switch configType {
    case .yaml(let configYAML):
      self.engine.run(withConfigYAML: configYAML, logLevel: logLevel.stringValue)
    case .typed(let config):
      self.engine.run(withConfig: config, logLevel: logLevel.stringValue)
    }
  }

  /// Initialize a new Envoy instance using a typed configuration.
  ///
  /// - parameter config:         Configuration to use for starting Envoy.
  /// - parameter logLevel:       Log level to use for this instance.
  /// - parameter engine:         The underlying engine to use for starting Envoy.
  /// - parameter filterRegistry: The filter registry to use for requests.
  convenience init(config: EnvoyConfiguration, logLevel: LogLevel = .info, engine: EnvoyEngine,
                   filterRegistry: FilterRegistry)
  {
    self.init(configType: .typed(config), logLevel: logLevel, engine: engine,
              filterRegistry: filterRegistry)
  }

  /// Initialize a new Envoy instance using a string configuration.
  ///
  /// - parameter configYAML:     Configuration yaml to use for starting Envoy.
  /// - parameter logLevel:       Log level to use for this instance.
  /// - parameter engine:         The underlying engine to use for starting Envoy.
  /// - parameter filterRegistry: The filter registry to use for requests.
  convenience init(configYAML: String, logLevel: LogLevel = .info, engine: EnvoyEngine,
                   filterRegistry: FilterRegistry)
  {
    self.init(configType: .yaml(configYAML), logLevel: logLevel, engine: engine,
              filterRegistry: filterRegistry)
  }
}

extension EnvoyClient: HTTPClient {
  public func start(_ headers: RequestHeaders, queue: DispatchQueue) -> Stream {
    let callbacks = EnvoyHTTPCallbacks()
    callbacks.dispatchQueue = queue

    let underlyingStream = self.engine.startStream(with: callbacks)
    let stream = Stream(underlyingStream: underlyingStream,
                        underlyingCallbacks: callbacks,
                        filterRegistry: self.filterRegistry)
    stream.sendHeaders(headers, endStream: false)
    return stream
  }

  @discardableResult
  public func send(_ headers: RequestHeaders, body: Data?, trailers: RequestTrailers?,
                   queue: DispatchQueue) -> Stream
  {
    let callbacks = EnvoyHTTPCallbacks()
    callbacks.dispatchQueue = queue

    let underlyingStream = self.engine.startStream(with: callbacks)
    let stream = Stream(underlyingStream: underlyingStream,
                        underlyingCallbacks: callbacks,
                        filterRegistry: self.filterRegistry)
    if let body = body, let trailers = trailers { // Close with trailers
      stream.sendHeaders(headers, endStream: false)
      stream.sendData(body)
      stream.close(trailers: trailers)
    } else if let body = body { // Close with data
      stream.sendHeaders(headers, endStream: false)
      stream.close(data: body)
    } else { // Close with headers-only
      stream.sendHeaders(headers, endStream: true)
    }

    return stream
  }
}
