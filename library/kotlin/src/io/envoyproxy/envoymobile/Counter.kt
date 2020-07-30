package io.envoyproxy.envoymobile

import io.envoyproxy.envoymobile.engine.EnvoyEngine
import java.lang.ref.WeakReference

/**
 * A stat.
 *
 * Current the supported stat type is counter, and it can increment.
 */
class Stat internal constructor(
        private val envoyEngine: WeakReference<EnvoyEngine>,
        private val elements: List<String>
) {

    /**
     * Increment the stat.
     */
    fun incCounter() {
        envoyEngine.get()?.incCounter(elements.joinToString(separator = "."))
    }
}