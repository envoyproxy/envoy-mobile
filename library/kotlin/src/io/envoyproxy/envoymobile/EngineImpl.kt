package io.envoyproxy.envoymobile

import io.envoyproxy.envoymobile.engine.EnvoyConfiguration
import io.envoyproxy.envoymobile.engine.EnvoyEngine

/**
 * An implementation of {@link Engine}.
 */
class EngineImpl internal constructor(
        engine: EnvoyEngine,
        envoyConfiguration: EnvoyConfiguration?,
        configurationYAML: String?,
        logLevel: LogLevel
) : Engine {

    private val streamClient: StreamClient
    private val statsClient: StatsClient

    constructor(
            engine: EnvoyEngine,
            envoyConfiguration: EnvoyConfiguration,
            logLevel: LogLevel = LogLevel.INFO
    ) : this(engine, envoyConfiguration, null, logLevel)

    constructor(
            engine: EnvoyEngine,
            configurationYAML: String,
            logLevel: LogLevel = LogLevel.INFO
    ) : this(engine, null, configurationYAML, logLevel)

    init {
        streamClient = StreamClientImpl(engine)
        statsClient = StatsClientImpl(engine)
        if (envoyConfiguration == null) {
            engine.runWithConfig(configurationYAML, logLevel.level)
        } else {
            engine.runWithConfig(envoyConfiguration, logLevel.level)
        }
    }

    override fun getStreamClient(): StreamClient {
        return streamClient
    }

    override fun getStatsClient(): StatsClient {
        return statsClient
    }
}
