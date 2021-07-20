package io.envoyproxy.envoymobile

/**
 * Internal configuration representation for filters.
 */
internal sealed class FilterConfiguration(
  val name: String
  val typedConfig: Map<String: Any>
) {
  object ResetRouteCache: FilterConfiguration(
    "envoy.filters.http.route_cache_reset",
    mapOf(
      "@type" to "type.googleapis.com/envoymobile.extensions.filters.http.route_cache_reset.RouteCacheReset"
    )
  )

  data class Platform(name: String): FilterConfiguration(
    "envoy.filters.http.platform_bridge",
    mapOf(
      "platform_filter_name" to name,
      "@type" to "type.googleapis.com/envoymobile.extensions.filters.http.platform_bridge.PlatformBridge"
    )
  )

  data class Native(name: String, typedConfig: Map<String, Any>: FilterConfiguration(
    name,
    typedConfig
  )
}
