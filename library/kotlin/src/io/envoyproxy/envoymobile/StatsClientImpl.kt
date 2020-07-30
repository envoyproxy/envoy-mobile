package io.envoyproxy.envoymobile

import io.envoyproxy.envoymobile.engine.EnvoyEngine
import java.lang.ref.WeakReference

/**
 * Client used to create new stats.
 */
internal class StatsClientImpl constructor(private val envoyEngine: EnvoyEngine) : StatsClient {

    /**
     * @return a counter instantiated with the given elements.
     */
    override fun getCounter(elements: List<String>): Counter = Counter(WeakReference(envoyEngine), elements)
}