package io.envoyproxy.envoymobile.engine;

import io.envoyproxy.envoymobile.engine.types.EnvoyEngineOnSetupComplete;
import io.envoyproxy.envoymobile.engine.types.EnvoyHTTPCallbacks;

/* Wrapper layer for calling into Envoy's C/++ API. */
public interface EnvoyEngine {
  /**
   * Creates a new stream with the provided callbacks.
   *
   * @param callbacks The callbacks for receiving callbacks from the stream.
   * @return A stream that may be used for sending data.
   */
  EnvoyHTTPStream startStream(EnvoyHTTPCallbacks callbacks);

  /**
   * Run the Envoy engine with the provided yaml string and log level.
   *
   * @param configurationYAML The configuration yaml with which to start Envoy.
   * @param logLevel          The log level to use when starting Envoy.
   * @param onSetupComplete   Called when the engine finishes its async initialization/startup.
   * @return A status indicating if the action was successful.
   */
  int runWithConfig(String configurationYAML, String logLevel,
                    EnvoyEngineOnSetupComplete onSetupComplete);

  /**
   * Run the Envoy engine with the provided EnvoyConfiguration and log level.
   *
   * @param envoyConfiguration The EnvoyConfiguration used to start Envoy.
   * @param logLevel           The log level to use when starting Envoy.
   * @param onSetupComplete    Called when the engine finishes its async initialization/startup.
   * @return A status indicating if the action was successful.
   */
  int runWithConfig(EnvoyConfiguration envoyConfiguration, String logLevel,
                    EnvoyEngineOnSetupComplete onSetupComplete);

  /**
   * Increments a counter with the given count.
   *
   * @param elements Elements of the counter stat.
   * @param count    Amount to add to the counter.
   */
  void recordCounter(String elements, int count);
}
