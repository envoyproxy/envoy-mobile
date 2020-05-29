@_implementationOnly import EnvoyEngine
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

  // MARK: - Public interface

  /// Create a new inactive stream which can be used to start active streams.
  ///
  /// - returns: The new inactive stream.
  public func newStream() -> InactiveStream {
    return InactiveStream(engine: self.engine)
  }
}
