package io.envoyproxy.envoymobile

import io.envoyproxy.envoymobile.engine.EnvoyEngine
import java.lang.ref.WeakReference

/**
 * Envoy implementation of a `Timer` for time measurements e.g. distribution of durations.
 */
internal class TimerImpl : Timer {
  internal val envoyEngine: WeakReference<EnvoyEngine>
  internal val elements: List<Element>

  internal constructor(engine: EnvoyEngine, elements: List<Element>) {
    this.envoyEngine = WeakReference<EnvoyEngine>(engine)
    this.elements = elements
  }

  override fun recordDuration(durationMs: Int) {
    envoyEngine.get()?.recordHistogramDuration(
      elements.joinToString(separator = ".") { it.value }, durationMs
    )
  }
}
