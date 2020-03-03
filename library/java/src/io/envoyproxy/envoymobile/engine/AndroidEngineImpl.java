package io.envoyproxy.envoymobile.engine;

import android.content.Context;
import io.envoyproxy.envoymobile.engine.types.EnvoyHTTPCallbacks;

public class AndroidEngineImpl implements AndroidEnvoyEngine {
  private final EnvoyEngine envoyEngine;

  public AndroidEngineImpl(Context context) {
    envoyEngine = new EnvoyEngineImpl();
    AndroidJniLibrary.load(context);
    AndroidNetworkMonitor.load(context);
  }

  public EnvoyHTTPStream startStream(EnvoyHTTPCallbacks callbacks) {
    return envoyEngine.startStream(callbacks);
  }

  public int runWithConfig(String configurationYAML, String logLevel) {
    return envoyEngine.runWithConfig(configurationYAML, logLevel);
  }

  public int runWithConfig(EnvoyConfiguration envoyConfiguration, String logLevel) {
    return envoyEngine.runWithConfig(envoyConfiguration, logLevel);
  }

  public int runWithConfig(AndroidEnvoyConfiguration envoyConfiguration, String logLevel) {
    if (envoyConfiguration.appForLifecycleHandling != null) {
      AndroidAppLifecycleMonitor monitor = new AndroidAppLifecycleMonitor();
      envoyConfiguration.appForLifecycleHandling.registerActivityLifecycleCallbacks(monitor);
    }

    return envoyEngine.runWithConfig(envoyConfiguration, logLevel);
  }
}
