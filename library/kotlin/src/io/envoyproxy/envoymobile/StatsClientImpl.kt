package io.envoyproxy.envoymobile

import io.envoyproxy.envoymobile.engine.EnvoyEngine
import java.lang.ref.WeakReference

/**
 * Envoy implementation of `StatsClient`.
 */
internal class StatsClientImpl constructor(
        internal val engine: EnvoyEngine
) : StatsClient {

    override fun getCounter(elements: List<String>): Counter = Counter(WeakReference(engine), elements)
}
