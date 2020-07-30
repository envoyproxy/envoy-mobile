package io.envoyproxy.envoymobile

import io.envoyproxy.envoymobile.engine.EnvoyEngine
import java.lang.ref.WeakReference

/**
 * Client used to create new stats.
 */
internal class StatsClientImpl constructor(private val envoyEngine: EnvoyEngine) : StatsClient {

    /**
     * @return a {@link StatBuilder}
     */
    override fun statBuilder(): StatBuilder = StatBuilder(WeakReference(envoyEngine))
}