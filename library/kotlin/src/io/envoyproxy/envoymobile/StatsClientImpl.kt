package io.envoyproxy.envoymobile

import io.envoyproxy.envoymobile.engine.EnvoyEngine

/**
 * Envoy implementation of `StatsClient`.
 */
internal class StatsClientImpl constructor(
  internal val engine: EnvoyEngine
) : StatsClient {

  override fun counter(vararg elements: Element): Counter {
    return CounterImpl(engine, elements.asList())
  }

  override fun gauge(vararg elements: Element): Gauge {
    return GaugeImpl(engine, elements.asList())
  }
}
