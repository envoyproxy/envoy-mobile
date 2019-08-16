package io.envoyproxy.envoymobile.engine;

import io.envoyproxy.envoymobile.engine.types.EnvoyObserver;

public class EnvoyEngineImpl implements EnvoyEngine {

  // Internal reference to helper object used to load and initialize the native library.
  // Volatile to ensure double-checked locking works correctly.
  private static volatile JavaLoader loader = null;

  private final long engineHandle;

  public EnvoyEngineImpl() {
    load();
    this.engineHandle = JniLibrary.initEngine();
  }

  /**
   * Creates a new stream with the provided observer.
   *
   * @param observer The observer for receiving callbacks from the stream.
   * @return A stream that may be used for sending data.
   */
  @Override
  public EnvoyStream startStream(EnvoyObserver observer) {
    long streamHandle = JniLibrary.initStream(engineHandle);
    return new EnvoyStream(streamHandle, observer);
  }

  /**
   * Run the Envoy engine with the provided config and log level.
   *
   * @param config The configuration file with which to start Envoy.
   * @return A status indicating if the action was successful.
   */
  @Override
  public int runWithConfig(String config) {
    return runWithConfig(config, "info");
  }

  /**
   * Run the Envoy engine with the provided config and log level.
   *
   * @param config   The configuration file with which to start Envoy.
   * @param logLevel The log level to use when starting Envoy.
   * @return int A status indicating if the action was successful.
   */
  @Override
  public int runWithConfig(String config, String logLevel) {
    try {
      return JniLibrary.runEngine(config, logLevel);
    } catch (Throwable throwable) {
      // TODO: Need to have a way to log the exception somewhere
      return 1;
    }
  }

  // Load and initialize Envoy and its dependencies, but only once.
  protected static void load() {
    if (loader != null) {
      return;
    }

    synchronized (JavaLoader.class) {
      if (loader != null) {
        return;
      }

      loader = new JavaLoader();
    }
  }

  private static class JavaLoader {

    private JavaLoader() {
      System.loadLibrary("envoy_jni");
    }
  }
}
