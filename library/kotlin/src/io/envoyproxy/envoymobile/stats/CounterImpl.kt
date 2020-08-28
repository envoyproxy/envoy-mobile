package io.envoyproxy.envoymobile

import io.envoyproxy.envoymobile.engine.EnvoyEngine
import java.lang.ref.WeakReference

/**
 * Envoy implementation of a `Counter`.
 */
internal class CounterImpl : Counter {
  internal val envoyEngine: WeakReference<EnvoyEngine>
  internal val elements: List<Element>

  internal constructor(engine: EnvoyEngine, elements: List<Element>) {
    this.envoyEngine = WeakReference<EnvoyEngine>(engine)
    this.elements = elements
  }

  override fun increment(count: Int = 1) {
    envoyEngine.get()?.recordCounter(
      elements.joinToString(separator = ".") { it.element }, count
    )
  }
}
