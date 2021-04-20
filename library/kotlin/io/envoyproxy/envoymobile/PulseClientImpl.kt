package io.envoyproxy.envoymobile

import io.envoyproxy.envoymobile.engine.EnvoyEngine

/**
 * Envoy implementation of `PulseClient`.
 */
internal class PulseClientImpl constructor(
  internal val engine: EnvoyEngine
) : PulseClient {

  override fun counter(vararg elements: Element): Counter {
    return CounterImpl(engine, elements.asList(), emptyList())
  }

  override fun counter(vararg elements: Element, tags: List<Tag>): Counter {
    return CounterImpl(engine, elements.asList(), tags)
  }

  override fun gauge(vararg elements: Element): Gauge {
    return GaugeImpl(engine, elements.asList(), emptyList())
  }

  override fun gauge(vararg elements: Element, tags: List<Tag>): Gauge {
    return GaugeImpl(engine, elements.asList(), tags)
  }

  override fun timer(vararg elements: Element): Timer {
    return TimerImpl(engine, elements.asList(), emptyList())
  }

  override fun timer(vararg elements: Element, tags: List<Tag>): Timer {
    return TimerImpl(engine, elements.asList(), tags)
  }

  override fun distribution(vararg elements: Element): Distribution {
    return DistributionImpl(engine, elements.asList(), emptyList())
  }

  override fun distribution(vararg elements: Element, tags: List<Tag>): Distribution {
    return DistributionImpl(engine, elements.asList(), tags)
  }
}
