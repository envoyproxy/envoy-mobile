package io.envoyproxy.envoymobile

import io.envoyproxy.envoymobile.engine.EnvoyConfiguration
import io.envoyproxy.envoymobile.engine.EnvoyEngine
import io.envoyproxy.envoymobile.engine.EnvoyEngineImpl
import io.envoyproxy.envoymobile.engine.types.EnvoyHTTPFilterFactory
import java.util.UUID

sealed class BaseConfiguration

class Standard : BaseConfiguration()
class Custom(val yaml: String) : BaseConfiguration()

/**
 * Builder used for creating and running a new `Engine` instance.
 */
open class EngineBuilder(
  private val configuration: BaseConfiguration = Standard()
) {
  private var logLevel = LogLevel.INFO
  private var engineType: () -> EnvoyEngine = { EnvoyEngineImpl() }

  private var statsDomain = "0.0.0.0"
  private var connectTimeoutSeconds = 30
  private var dnsRefreshSeconds = 60
  private var dnsFailureRefreshSecondsBase = 2
  private var dnsFailureRefreshSecondsMax = 10
  private var filterChain = mutableListOf<EnvoyHTTPFilterFactory>()
  private var statsFlushSeconds = 60
  private var appVersion = "unspecified"
  private var appId = "unspecified"
  private var virtualClusters = "[]"

  /**
   * Add a log level to use with Envoy.
   *
   * @param logLevel the log level to use with Envoy.
   *
   * @return this builder.
   */
  fun addLogLevel(logLevel: LogLevel): EngineBuilder {
    this.logLevel = logLevel
    return this
  }

  /**
   * Add a domain to flush stats to.
   *
   * @param statsDomain the domain to flush stats to.
   *
   * @return this builder.
   */
  fun addStatsDomain(statsDomain: String): EngineBuilder {
    this.statsDomain = statsDomain
    return this
  }

  /**
   * Add a timeout for new network connections to hosts in the cluster.
   *
   * @param connectTimeoutSeconds timeout for new network connections to hosts in the cluster.
   *
   * @return this builder.
   */
  fun addConnectTimeoutSeconds(connectTimeoutSeconds: Int): EngineBuilder {
    this.connectTimeoutSeconds = connectTimeoutSeconds
    return this
  }

  /**
   * Add a rate at which to refresh DNS.
   *
   * @param dnsRefreshSeconds rate in seconds to refresh DNS.
   *
   * @return this builder.
   */
  fun addDNSRefreshSeconds(dnsRefreshSeconds: Int): EngineBuilder {
    this.dnsRefreshSeconds = dnsRefreshSeconds
    return this
  }

  /**
   * Add a rate at which to refresh DNS in case of DNS failure.
   *
   * @param base rate in seconds.
   * @param max rate in seconds.
   *
   * @return this builder.
   */
  fun addDNSFailureRefreshSeconds(base: Int, max: Int): EngineBuilder {
    this.dnsFailureRefreshSecondsBase = base
    this.dnsFailureRefreshSecondsMax = max
    return this
  }

  /**
   * Add an interval at which to flush Envoy stats.
   *
   * @param statsFlushSeconds interval at which to flush Envoy stats.
   *
   * @return this builder.
   */
  fun addStatsFlushSeconds(statsFlushSeconds: Int): EngineBuilder {
    this.statsFlushSeconds = statsFlushSeconds
    return this
  }

  /**
   * Add an HTTP filter factory used to create filters for streams sent by this client.
   *
   * @param factory closure returning an instantiated filter.
   *
   * @return this builder.
   */
  fun addFilter(factory: () -> Filter): EngineBuilder {
    val filterName = UUID.randomUUID().toString()
    this.filterChain.add(FilterFactory(filterName, factory))
    return this
  }

  /**
   * Add the App Version of the App using this Envoy Client.
   *
   * @param appVersion the version.
   *
   * @return this builder.
   */
  fun addAppVersion(appVersion: String): EngineBuilder {
    this.appVersion = appVersion
    return this
  }

  /**
   * Add the App ID of the App using this Envoy Client.
   *
   * @param appId the ID.
   *
   * @return this builder.
   */
  fun addAppId(appId: String): EngineBuilder {
    this.appId = appId
    return this
  }

  /**
   * Add virtual cluster configuration.
   *
   * @param virtualClusters the JSON configuration string for virtual clusters.
   *
   * @return this builder.
   */
  fun addVirtualClusters(virtualClusters: String): EngineBuilder {
    this.virtualClusters = virtualClusters
    return this
  }

  /**
   * Builds and runs a new Engine instance with the provided configuration.
   *
   * @return A new instance of Envoy.
   */
  fun build(): Engine {
    return when (configuration) {
      is Custom -> {
        EngineImpl(engineType(), configuration.yaml, logLevel)
      }
      is Standard -> {
        EngineImpl(
          engineType(),
          EnvoyConfiguration(
            statsDomain, connectTimeoutSeconds,
            dnsRefreshSeconds, dnsFailureRefreshSecondsBase, dnsFailureRefreshSecondsMax,
            filterChain, statsFlushSeconds, appVersion, appId, virtualClusters
          ),
          logLevel
        )
      }
    }
  }

  /**
   * Add a specific implementation of `EnvoyEngine` to use for starting Envoy.
   *
   * A new instance of this engine will be created when `build()` is called.
   */
  fun addEngineType(engineType: () -> EnvoyEngine): EngineBuilder {
    this.engineType = engineType
    return this
  }
}
