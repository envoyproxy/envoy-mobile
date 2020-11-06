package io.envoyproxy.envoymobile

import android.app.Application
import android.util.Log
import io.envoyproxy.envoymobile.engine.AndroidEngineImpl

class AndroidEngineBuilder @JvmOverloads constructor(
  application: Application,
  baseConfiguration: BaseConfiguration = Standard()
) : EngineBuilder(baseConfiguration) {
  init {
    val start = System.currentTimeMillis()
    addEngineType {
      val s = System.currentTimeMillis()
      val androidEngineImpl = AndroidEngineImpl(application)
      val e = System.currentTimeMillis()
      Log.d("AndroidEngineBuilder","~~~ init ${e-s}")
      androidEngineImpl
    }
    val end = System.currentTimeMillis()
    Log.d("AndroidEngineBuilder","~~~ ${end-start}")
  }
}
