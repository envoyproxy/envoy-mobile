package io.envoyproxy.envoymobile

import io.envoyproxy.envoymobile.engine.EnvoyConfiguration
import io.envoyproxy.envoymobile.engine.EnvoyEngine
import io.envoyproxy.envoymobile.engine.types.EnvoyEngineOnSetupComplete

/**
 * An implementation of {@link Engine}.
 */
class EngineImpl internal constructor(
  internal val envoyEngine: EnvoyEngine,
  internal val envoyConfiguration: EnvoyConfiguration?,
  internal val configurationYAML: String?,
  internal val logLevel: LogLevel,
  internal val onSetupComplete: (() -> Unit)?
) : Engine {

  private val streamClient: StreamClient
  private val statsClient: StatsClient

  constructor(
    envoyEngine: EnvoyEngine,
    envoyConfiguration: EnvoyConfiguration,
    logLevel: LogLevel = LogLevel.INFO,
    onSetupComplete: (() -> Unit)?
  ) : this(envoyEngine, envoyConfiguration, null, logLevel, onSetupComplete)

  constructor(
    envoyEngine: EnvoyEngine,
    configurationYAML: String,
    logLevel: LogLevel = LogLevel.INFO,
    onSetupComplete: (() -> Unit)?
  ) : this(envoyEngine, null, configurationYAML, logLevel, onSetupComplete)

  init {
    streamClient = StreamClientImpl(envoyEngine)
    statsClient = StatsClientImpl(envoyEngine)
    if (envoyConfiguration == null) {
      envoyEngine.runWithConfig(configurationYAML, logLevel.level, onSetupComplete)
    } else {
      envoyEngine.runWithConfig(envoyConfiguration, logLevel.level, onSetupComplete)
    }
  }

  override fun streamClient(): StreamClient {
    return streamClient
  }

  override fun statsClient(): StatsClient {
    return statsClient
  }
}
