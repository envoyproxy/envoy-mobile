package io.envoyproxy.envoymobile

import io.envoyproxy.envoymobile.engine.EnvoyEngine
import java.lang.ref.WeakReference

/**
 * Envoy implementation of a `Gauge`.
 */
internal class GaugeImpl : Gauge {
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

  override fun set(value: Int) {
    envoyEngine.get()?.recordGaugeSet(series, tags, value)
  }

  override fun add(amount: Int) {
    envoyEngine.get()?.recordGaugeAdd(series, tags, amount)
  }

  override fun sub(amount: Int) {
    envoyEngine.get()?.recordGaugeSub(series, tags, amount)
  }

  override fun attach(tags: List<Tag>): Gauge {
    return GaugeImpl(envoyEngine, series, tags)
  }
}
