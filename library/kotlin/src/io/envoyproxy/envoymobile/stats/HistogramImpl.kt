package io.envoyproxy.envoymobile

import io.envoyproxy.envoymobile.engine.EnvoyEngine
import java.lang.ref.WeakReference

/**
 * Envoy implementation of a `Histogram`.
 */
internal class HistogramImpl : Histogram {
  internal val envoyEngine: WeakReference<EnvoyEngine>
  internal val elements: List<Element>

  internal constructor(engine: EnvoyEngine, elements: List<Element>) {
    this.envoyEngine = WeakReference<EnvoyEngine>(engine)
    this.elements = elements
  }

  override fun record(value: Int) {
    envoyEngine.get()?.recordHistogramDurationMs(
      elements.joinToString(separator = ".") { it.value }, value
    )
  }
}
