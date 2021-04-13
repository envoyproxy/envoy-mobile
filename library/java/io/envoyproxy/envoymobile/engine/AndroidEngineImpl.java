package io.envoyproxy.envoymobile.engine;

import android.content.Context;
import io.envoyproxy.envoymobile.engine.types.EnvoyHTTPCallbacks;
import io.envoyproxy.envoymobile.engine.types.EnvoyOnEngineRunning;
import io.envoyproxy.envoymobile.engine.types.EnvoyStringAccessor;

import java.util.Map;

/* Android-specific implementation of the `EnvoyEngine` interface. */
public class AndroidEngineImpl implements EnvoyEngine {
  private final EnvoyEngine envoyEngine;

  public AndroidEngineImpl(Context context) {
    this.envoyEngine = new EnvoyEngineImpl();
    AndroidJniLibrary.load(context);
    AndroidNetworkMonitor.load(context);
  }

  @Override
  public EnvoyHTTPStream startStream(EnvoyHTTPCallbacks callbacks) {
    return envoyEngine.startStream(callbacks);
  }

  @Override
  public void terminate() {
    envoyEngine.terminate();
  }

  @Override
  public int runWithConfig(String configurationYAML, String logLevel,
                           EnvoyOnEngineRunning onEngineRunning) {
    // re-enable lifecycle-based stat flushing when https://github.com/lyft/envoy-mobile/issues/748
    // gets fixed. AndroidAppLifecycleMonitor monitor = new AndroidAppLifecycleMonitor();
    // application.registerActivityLifecycleCallbacks(monitor);
    return envoyEngine.runWithConfig(configurationYAML, logLevel, onEngineRunning);
  }

  @Override
  public int runWithConfig(EnvoyConfiguration envoyConfiguration, String logLevel,
                           EnvoyOnEngineRunning onEngineRunning) {
    // re-enable lifecycle-based stat flushing when https://github.com/lyft/envoy-mobile/issues/748
    // gets fixed. AndroidAppLifecycleMonitor monitor = new AndroidAppLifecycleMonitor();
    // application.registerActivityLifecycleCallbacks(monitor);
    return envoyEngine.runWithConfig(envoyConfiguration, logLevel, onEngineRunning);
  }

  @Override
  public int recordCounterInc(String elements, Map<String, String> tags, int count) {
    return envoyEngine.recordCounterInc(elements, tags, count);
  }

  @Override
  public int recordGaugeSet(String elements, Map<String, String> tags, int value) {
    return envoyEngine.recordGaugeSet(elements, tags, value);
  }

  @Override
  public int recordGaugeAdd(String elements, Map<String, String> tags, int amount) {
    return envoyEngine.recordGaugeAdd(elements, tags, amount);
  }

  @Override
  public int recordGaugeSub(String elements, Map<String, String> tags, int amount) {
    return envoyEngine.recordGaugeSub(elements, tags, amount);
  }

  @Override
  public int recordHistogramDuration(String elements, Map<String, String> tags, int durationMs) {
    return envoyEngine.recordHistogramDuration(elements, tags, durationMs);
  }

  @Override
  public int recordHistogramValue(String elements, Map<String, String> tags, int value) {
    return envoyEngine.recordHistogramValue(elements, tags, value);
  }

  @Override
  public int registerStringAccessor(String accessorName, EnvoyStringAccessor accessor) {
    return envoyEngine.registerStringAccessor(accessorName, accessor);
  }
}
