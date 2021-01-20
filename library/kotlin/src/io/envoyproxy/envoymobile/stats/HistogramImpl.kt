package io.envoyproxy.envoymobile

import io.envoyproxy.envoymobile.engine.EnvoyEngine
import io.envoyproxy.envoymobile.engine.types.HistogramUnit
import java.lang.ref.WeakReference

/**
 * Envoy implementation of a `Histogram`.
 */
internal class HistogramImpl : Histogram {
  internal val envoyEngine: WeakReference<EnvoyEngine>
  internal val elements: List<Element>
  internal val unitMeasure: HistogramUnit

  internal constructor(engine: EnvoyEngine, elements: List<Element>, unitMeasure: HistogramUnit) {
    this.envoyEngine = WeakReference<EnvoyEngine>(engine)
    this.elements = elements
    this.unitMeasure = unitMeasure
  }

  override fun recordValue(value: Int) {
    envoyEngine.get()?.recordHistogramValue(
      elements.joinToString(separator = ".") { it.value }, value, unitMeasure
    )
  }
}
