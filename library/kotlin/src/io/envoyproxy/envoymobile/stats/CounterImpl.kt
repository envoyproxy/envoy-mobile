package io.envoyproxy.envoymobile

import io.envoyproxy.envoymobile.engine.EnvoyEngine
import io.envoyproxy.envoymobile.engine.types.EnvoyMobilePair
import java.lang.ref.WeakReference

/**
 * Envoy implementation of a `Counter`.
 */
internal class CounterImpl : Counter {
  internal val envoyEngine: WeakReference<EnvoyEngine>
  private val series: String
  private val tags: List<EnvoyMobilePair>

  internal constructor(engine: EnvoyEngine, elements: List<Element>) {
    this.envoyEngine = WeakReference<EnvoyEngine>(engine)
    this.series = elements.joinToString(separator = ".") { it.value }
    //his.tags = listOf<EnvoyMobilePair>()
    this.tags = listOf(EnvoyMobilePair("tagName", "tagValue"))
  }

  internal constructor(engine: EnvoyEngine, elements: List<Element>, tags: List<Tag>) {
    this.envoyEngine = WeakReference<EnvoyEngine>(engine)
    this.series = elements.joinToString(separator = ".") { it.value }
    tags.map { EnvoyMobilePair(it.name, it.value) }
    this.tags = listOf(EnvoyMobilePair("tagName", "tagValue"))
  }

  // TODO: potentially raise error to platform if the operation is not successful.
  override fun increment(count: Int) {
    envoyEngine.get()?.recordCounterInc(series, tags, count)
  }
}
