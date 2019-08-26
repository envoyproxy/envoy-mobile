package io.envoyproxy.envoymobile.engine;

import io.envoyproxy.envoymobile.engine.types.EnvoyObserver;

public class EnvoyEngineImpl implements EnvoyEngine {

  private final long engineHandle;

  public EnvoyEngineImpl() {
    JniLibrary.load();
    this.engineHandle = JniLibrary.initEngine();
  }

  /**
   * Creates a new stream with the provided observer.
   *
   * @param observer The observer for receiving callbacks from the stream.
   * @return A stream that may be used for sending data.
   */
  @Override
  public EnvoyHTTPStream startStream(EnvoyObserver observer) {
    long streamHandle = JniLibrary.initStream(engineHandle);
    return new EnvoyHTTPStream(streamHandle, observer);
  }

  /**
   * Run the Envoy engine with the provided envoyConfiguration and log level.
   *
   * @param envoyConfiguration The configuration file with which to start Envoy.
   * @return A status indicating if the action was successful.
   */
  @Override
  public int runWithConfig(EnvoyConfiguration envoyConfiguration) {
    return runWithConfig(envoyConfiguration, "info");
  }

  /**
   * Run the Envoy engine with the provided envoyConfiguration and log level.
   *
   * @param envoyConfiguration   The configuration file with which to start Envoy.
   * @param logLevel The log level to use when starting Envoy.
   * @return int A status indicating if the action was successful.
   */
  @Override
  public int runWithConfig(EnvoyConfiguration envoyConfiguration, String logLevel) {
    try {
      String resolvedConfig = envoyConfiguration.resolve(JniLibrary.templateString());
      return JniLibrary.runEngine(resolvedConfig, logLevel);
    } catch (Throwable throwable) {
      // TODO: Need to have a way to log the exception somewhere
      return 1;
    }
  }
}
