package io.envoyproxy.envoymobile

import android.app.Application
import android.content.Context
import io.envoyproxy.envoymobile.engine.AndroidEngineImpl

class AndroidEnvoyClientBuilder @JvmOverloads constructor (
    context: Context,
    baseConfiguration: BaseConfiguration = Standard()
) : EnvoyClientBuilder(baseConfiguration) {
  private var appForLifecycleHandling: Application? = null

  init {
    addEngineType { AndroidEngineImpl(context) }
  }

  /**
   * Enables app lifecycle handling by subscribing the Envoy client to notifications and
   * performing optimizations based on them (i.e., flusing stats on app backgrounding).
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
