package io.envoyproxy.envoymobile

class StatBuilder {
    private val stat: Stat
    private val engine: EnvoyClient

    constructor(engine: EnvoyClient) {
        stat = Stat(WeakReference(engine))
    }

    fun withName(name: String): StatBuilder {
        stat.statName = name
        return this
    }

    fun build(): Stat {
        return stat
    }
}