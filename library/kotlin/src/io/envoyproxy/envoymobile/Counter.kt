package io.envoyproxy.envoymobile

import io.envoyproxy.envoymobile.engine.EnvoyEngine
import java.lang.ref.WeakReference

/**
 * A stat.
 *
 * Current the supported stat type is counter, and it can increment.
 */
class Counter internal constructor(
  private val envoyEngine: WeakReference<EnvoyEngine>,
  private val elements: List<Element>
) {

  /**
   * Increment the counter by the given count.
   */
  fun increment(count: Int) {
    envoyEngine.get()?.recordCounter(
      elements.joinToString(separator = ".") { it.element }, count
    )
  }
}
