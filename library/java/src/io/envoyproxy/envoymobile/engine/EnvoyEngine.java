package io.envoyproxy.envoymobile.engine;

import io.envoyproxy.envoymobile.engine.types.EnvoyObserver;

public interface EnvoyEngine {
  /**
   * Creates a new stream with the provided observer.
   *
   * @param observer The observer for receiving callbacks from the stream.
   * @return A stream that may be used for sending data.
   */
  EnvoyHTTPStream startStream(EnvoyObserver observer);

  /**
   * Run the Envoy engine with the provided envoyConfiguration and log level.
   *
   * @param config The configuration file with which to start Envoy.
   * @return A status indicating if the action was successful.
   */
  int runWithConfig(EnvoyConfiguration envoyConfiguration);

  /**
   * Run the Envoy engine with the provided envoyConfiguration and log level.
   *
   * @param config   The configuration file with which to start Envoy.
   * @param logLevel The log level to use when starting Envoy.
   * @return int A status indicating if the action was successful.
   */
  int runWithConfig(EnvoyConfiguration envoyConfiguration, String logLevel);
}
