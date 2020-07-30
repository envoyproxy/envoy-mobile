package io.envoyproxy.envoymobile

/**
 * Client used to create new stats.
 */
interface StatsClient {

    /**
     * @return a {@link StatBuilder}
     */
    fun statBuilder(): StatBuilder
}