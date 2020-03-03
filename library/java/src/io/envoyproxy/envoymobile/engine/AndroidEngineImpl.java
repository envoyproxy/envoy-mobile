package io.envoyproxy.envoymobile.engine;

import android.content.Context;
import io.envoyproxy.envoymobile.engine.types.EnvoyHTTPCallbacks;

public class AndroidEngineImpl implements EnvoyEngine {

  private final EnvoyEngine envoyEngine;

  public AndroidEngineImpl(Context context) {
    envoyEngine = new EnvoyEngineImpl();
    AndroidJniLibrary.load(context);
    AndroidNetworkMonitor.load(context);
  }

  @Override
  public EnvoyHTTPStream startStream(EnvoyHTTPCallbacks callbacks) {
    return envoyEngine.startStream(callbacks);
  }

  @Override
  public int runWithConfig(String configurationYAML, String logLevel) {
    return envoyEngine.runWithConfig(configurationYAML, logLevel);
  }

  @Override
  public int runWithConfig(EnvoyConfiguration envoyConfiguration, String logLevel) {
    return envoyEngine.runWithConfig(envoyConfiguration, logLevel);
  }

  public int runWithConfig(AndroidEnvoyConfiguration envoyConfiguration, String logLevel) {
    return envoyEngine.runWithConfig(envoyConfiguration, logLevel);
  }
}
