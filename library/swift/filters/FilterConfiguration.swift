import Foundation

/// Internal configuration representation for filters.
internal struct FilterConfiguration {
  private let name: String
  private let typedConfig: Encodable

  fileprivate init(name: String, typedConfig: Encodable) {
    self.name = name
    self.typedConfig = typedConfig
  }
}

extension FilterConfiguration {
  static let ResetRouteCache = FilterConfiguration(
    name: "envoy.filters.http.route_cache_reset",
    typedConfig: [
      "@type": "type.googleapis.com/envoymobile.extensions.filters.http.route_cache_reset.RouteCacheReset"
    ]
  )

  static Platform(name: String): FilterConfiguration {
    FilterConfiguration(
      name: "envoy.filters.http.platform_bridge",
      typedConfig: [
        "platform_filter_name": name,
        "@type": "type.googleapis.com/envoymobile.extensions.filters.http.platform_bridge.PlatformBridge"
      ]
    )
  }

  static Native(name: String, typedConfig: Encodable): FilterConfiguration {
    FilterConfiguration(name: name, typedConfig: typedConfig)
  }
}

extension FilterConfiguration: Encodable {
  enum CodingKeys: String, CodingKey {
    case name = "name"
    case typedConfig = "typed_config"
  }
}
