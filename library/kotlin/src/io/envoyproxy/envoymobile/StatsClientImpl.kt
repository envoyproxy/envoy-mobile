package io.envoyproxy.envoymobile

import java.lang.ref.WeakReference

import io.envoyproxy.envoymobile.engine.EnvoyEngine

/**
 * Client used to create new stats.
 */
class StatsClient internal constructor(private val envoyEngine: EnvoyEngine) {

    /**
     * @return a {@link StatBuilder}
     */
    fun statBuilder(): StatBuilder = StatBuilder(WeakReference(envoyEngine))
}