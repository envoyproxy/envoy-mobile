package io.envoyproxy.envoymobile

import io.envoyproxy.envoymobile.engine.EnvoyEngine
import java.lang.ref.WeakReference

/**
 * Envoy implementation of `StatsClient`.
 */
internal class StatsClientImpl constructor(
  internal val engine: EnvoyEngine
) : StatsClient {

  override fun counter(vararg elements: Element): Counter {
    return Counter(WeakReference(engine), elements.asList())
  }
}
