package io.envoyproxy.envoymobile

import io.envoyproxy.envoymobile.engine.types.EnvoyHTTPFilterCallbacks
import java.util.Timer
import kotlin.concurrent.scheduleAtFixedRate

/**
 * Envoy implementation of `RequestFilterCallbacks`.
 */
internal class RequestFilterCallbacksImpl constructor(
  internal val callbacks: EnvoyHTTPFilterCallbacks,
) : RequestFilterCallbacks {

  private val timer by lazy { Timer() }

  override fun resumeRequest() {
    callbacks.resumeIteration()
  }

  override fun resetIdleTimer() {
    callbacks.resetIdleTimer()
  }

  override fun pauseRequestTimeout() {
    timer.scheduleAtFixedRate(5, 5) {
      resetIdleTimer()
    }
  }

  override fun resumeRequestTimeout() {
    timer.cancel()
  }
}
