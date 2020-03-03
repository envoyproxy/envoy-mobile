package io.envoyproxy.envoymobile.engine;

public interface AndroidEnvoyEngine extends EnvoyEngine {
    /**
     * Run the Envoy engine with the provided EnvoyConfiguration and log level.
     *
     * @param envoyConfiguration The EnvoyConfiguration used to start Envoy.
     * @param logLevel           The log level to use when starting Envoy.
     * @return int A status indicating if the action was successful.
     */
    int runWithConfig(EnvoyConfiguration envoyConfiguration, String logLevel);
}
