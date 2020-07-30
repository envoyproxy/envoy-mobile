package io.envoyproxy.envoymobile

/**
 * Client used to create new stats.
 */
interface StatsClient {

    /**
     * @return a counter instantiated with the given elements.
     */
    fun getCounter(elements: List<String>): Counter
}