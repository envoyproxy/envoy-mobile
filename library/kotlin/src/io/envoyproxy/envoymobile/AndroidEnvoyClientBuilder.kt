package io.envoyproxy.envoymobile

import android.app.Application
import android.content.Context
import io.envoyproxy.envoymobile.engine.AndroidEngineImpl
import io.envoyproxy.envoymobile.engine.AndroidEnvoyConfiguration

class AndroidEnvoyClientBuilder @JvmOverloads constructor (
    context: Context,
    baseConfiguration: BaseConfiguration = Standard()
) : EnvoyClientBuilder(baseConfiguration) {
  private var appForLifecycleHandling: Application? = null

  init {
    addEngineType { AndroidEngineImpl(context) }
  }

  /**
   * Builds a new instance of Envoy using the provided configurations.
   *
   * @return A new instance of Envoy.
   */
  override fun build(): Envoy {
    return when (configuration) {
      is Custom -> {
        return Envoy(engineType(), configuration.yaml, logLevel)
      }
      is Standard -> {
        Envoy(engineType(), AndroidEnvoyConfiguration(statsDomain, connectTimeoutSeconds, dnsRefreshSeconds, dnsFailureRefreshSecondsBase, dnsFailureRefreshSecondsMax, statsFlushSeconds, appForLifecycleHandling), logLevel)
      }
    }
  }

  /**
   * Enables app lifecycle handling by subscribing the Envoy client to notifications and
   * performing optimizations based on them (i.e., flushing stats on app backgrounding).
   *
   * @param app the app to use for registering lifecycle handler callbacks.
   *
   * @return this builder.
   */
  fun addAppLifecycleHandling(app: Application): EnvoyClientBuilder {
    this.appForLifecycleHandling = app
    return this
  }
}
