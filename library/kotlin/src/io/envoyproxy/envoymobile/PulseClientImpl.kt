package io.envoyproxy.envoymobile

import io.envoyproxy.envoymobile.engine.EnvoyEngine
import io.envoyproxy.envoymobile.engine.types.HistogramUnit

/**
 * Envoy implementation of `PulseClient`.
 */
internal class PulseClientImpl constructor(
  internal val engine: EnvoyEngine
) : PulseClient {

  override fun counter(vararg elements: Element): Counter {
    return CounterImpl(engine, elements.asList())
  }

  override fun gauge(vararg elements: Element): Gauge {
    return GaugeImpl(engine, elements.asList())
  }

  override fun histogram(unitMeasure: HistogramUnit, vararg elements: Element): Histogram {
    return HistogramImpl(engine, elements.asList(), unitMeasure)
  }
}
