package io.envoyproxy.envoymobile.engine;

/* Android-specific wrapper layer for calling into Envoy's C/++ API. */
public interface AndroidEnvoyEngine extends EnvoyEngine {
  /**
   * Run the Envoy engine with the provided EnvoyConfiguration and log level.
   *
   * @param envoyConfiguration The EnvoyConfiguration used to start Envoy.
   * @param logLevel           The log level to use when starting Envoy.
   * @return A status indicating if the action was successful.
   */
  int runWithConfig(AndroidEnvoyConfiguration envoyConfiguration, String logLevel);
}
