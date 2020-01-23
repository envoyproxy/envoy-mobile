package io.envoyproxy.envoymobile

import android.content.Context
import io.envoyproxy.envoymobile.engine.AndroidEngineImpl

class AndroidEnvoyClientBuilder(
    context: Context,
    baseConfiguration: BaseConfiguration = Standard()
) : EnvoyClientBuilder(baseConfiguration) {

  init {
    addEngineType { AndroidEngineImpl(context) }
  }
}
