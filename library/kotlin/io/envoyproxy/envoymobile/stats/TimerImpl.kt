package io.envoyproxy.envoymobile

import io.envoyproxy.envoymobile.engine.EnvoyEngine
import java.lang.ref.WeakReference

/**
 * Envoy implementation of a `Timer` for time measurements e.g. distribution of durations.
 */
internal class TimerImpl : Timer {
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

  override fun completeWithDuration(durationMs: Int) {
    envoyEngine.get()?.recordHistogramDuration(series, tags, durationMs)
  }

  override fun attach(tags: List<Tag>): Timer {
    return TimerImpl(envoyEngine, series, tags)
  }
}
