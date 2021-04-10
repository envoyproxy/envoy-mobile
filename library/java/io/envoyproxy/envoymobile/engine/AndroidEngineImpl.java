package io.envoyproxy.envoymobile.engine;

import android.content.Context;
import io.envoyproxy.envoymobile.engine.types.EnvoyHTTPCallbacks;
import io.envoyproxy.envoymobile.engine.types.EnvoyOnEngineRunning;
import io.envoyproxy.envoymobile.engine.types.EnvoyStringAccessor;

/* Android-specific implementation of the `EnvoyEngine` interface. */
public class AndroidEngineImpl implements EnvoyEngine {
  private final EnvoyEngine envoyEngine;

  /**
   * @param runningCallback Called when the engine finishes its async startup and begins running.
   */
  public AndroidEngineImpl(Context context, EnvoyOnEngineRunning runningCallback) {
    this.envoyEngine = new EnvoyEngineImpl(runningCallback);
    AndroidJniLibrary.load(context);
    AndroidNetworkMonitor.load(context);
  }

  @Override
  public EnvoyHTTPStream startStream(EnvoyHTTPCallbacks callbacks) {
    return envoyEngine.startStream(callbacks);
  }

  @Override
  public int runWithConfig(String configurationYAML, String logLevel) {
    // re-enable lifecycle-based stat flushing when https://github.com/lyft/envoy-mobile/issues/748
    // gets fixed. AndroidAppLifecycleMonitor monitor = new AndroidAppLifecycleMonitor();
    // application.registerActivityLifecycleCallbacks(monitor);
    return envoyEngine.runWithConfig(configurationYAML, logLevel);
  }

  @Override
  public int runWithConfig(EnvoyConfiguration envoyConfiguration, String logLevel) {
    // re-enable lifecycle-based stat flushing when https://github.com/lyft/envoy-mobile/issues/748
    // gets fixed. AndroidAppLifecycleMonitor monitor = new AndroidAppLifecycleMonitor();
    // application.registerActivityLifecycleCallbacks(monitor);
    return envoyEngine.runWithConfig(envoyConfiguration, logLevel);
  }

  public void terminate() {
    envoyEngine.terminate();
  }

  @Override
  public int recordCounterInc(String elements, int count) {
    return envoyEngine.recordCounterInc(elements, count);
  }

  @Override
  public int recordGaugeSet(String elements, int value) {
    return envoyEngine.recordGaugeSet(elements, value);
  }

  @Override
  public int recordGaugeAdd(String elements, int amount) {
    return envoyEngine.recordGaugeAdd(elements, amount);
  }

  @Override
  public int recordGaugeSub(String elements, int amount) {
    return envoyEngine.recordGaugeSub(elements, amount);
  }

  @Override
  public int recordHistogramDuration(String elements, int durationMs) {
    return envoyEngine.recordHistogramDuration(elements, durationMs);
  }

  @Override
  public int recordHistogramValue(String elements, int value) {
    return envoyEngine.recordHistogramValue(elements, value);
  }

  @Override
  public int registerStringAccessor(String accessorName, EnvoyStringAccessor accessor) {
    return envoyEngine.registerStringAccessor(accessorName, accessor);
  }
}
