package io.envoyproxy.envoymobile

import java.lang.ref.WeakReference

class Stat {

    private val statName: String
    private val engine: WeakReference<EnvoyClient>

    constructor(engine: WeakReference<EnvoyClient>, statName: String) {
        this.engine = engine
        this.statName = statName
    }

    fun incCounter() {
        engine.get().incCounter(statName)
    }
}