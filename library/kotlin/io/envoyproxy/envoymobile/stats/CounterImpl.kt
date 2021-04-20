package io.envoyproxy.envoymobile

import io.envoyproxy.envoymobile.engine.EnvoyEngine
import java.lang.ref.WeakReference

/**
 * Envoy implementation of a `Counter`.
 */
internal class CounterImpl : Counter {
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

  // TODO: potentially raise error to platform if the operation is not successful.
  override fun increment(count: Int) {
    envoyEngine.get()?.recordCounterInc(series, tags, count)
  }

  override fun attach(tags: List<Tag>): Counter {
    return CounterImpl(this.envoyEngine, this.series, tags)
  }
}
