package io.envoyproxy.envoymobile

import io.envoyproxy.envoymobile.engine.EnvoyEngine
import java.lang.ref.WeakReference

/**
 * Envoy implementation of a `Distribution` for measurements of quantile data for int values
 */
internal class DistributionImpl : Distribution {
  var envoyEngine: WeakReference<EnvoyEngine>
  var series: String
  var tags: Map<String, String>

  internal constructor(engine: EnvoyEngine, elements: List<Element>, tags: List<Tag>) {
    this.envoyEngine = WeakReference<EnvoyEngine>(engine)
    this.series = elements.joinToString(separator = ".") { it.value }
    this.tags = convert(tags)
  }

  private constructor(engine: WeakReference<EnvoyEngine>, series: String, tags: List<Tag>) {
    this.envoyEngine = engine
    this.series = series
    this.tags = convert(tags)
  }

  override fun recordValue(value: Int) {
    envoyEngine.get()?.recordHistogramValue(series, tags, value)
  }

  override fun attach(tags: List<Tag>): Distribution {
    return DistributionImpl(envoyEngine, series, tags)
  }
}
