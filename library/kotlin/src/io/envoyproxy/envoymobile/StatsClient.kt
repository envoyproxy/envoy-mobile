package io.envoyproxy.envoymobile

/**
 * Client used to create new stat.
 */
StatsClient() {

    private engine: EnvoyClient

    constructor(engine: EnvoyClient) {
        this.engine = EnvoyClient
    }

    /**
     * Creates a new stat.
     *
     * @Return a {@link StatBuilder}
     */
    fun newStat(): StatBuilder = StatBuilder(engine)
}