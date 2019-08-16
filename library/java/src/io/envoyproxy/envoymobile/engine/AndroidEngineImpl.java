package io.envoyproxy.envoymobile.engine;

import android.content.Context;
import android.net.ConnectivityManager;
import io.envoyproxy.envoymobile.engine.types.EnvoyObserver;

public class AndroidEngineImpl implements EnvoyEngine {

  // Internal reference to helper object used to load and initialize the native library.
  // Volatile to ensure double-checked locking works correctly.
  private static volatile AndroidEngineImpl loader = null;

  private final EnvoyEngine envoyEngine;

  // Private helper class used by the load method to ensure the native library and its
  // dependencies are loaded and initialized at most once.
  public AndroidEngineImpl(Context context) {
    initialize((ConnectivityManager) context.getSystemService(Context.CONNECTIVITY_SERVICE));
    envoyEngine = new EnvoyEngineImpl();
  }

  // Load and initialize Envoy and its dependencies, but only once.
  public static void load(Context context) {
    if (loader != null) {
      return;
    }

    synchronized (AndroidEngineImpl.class) {
      if (loader != null) {
        return;
      }

      loader = new AndroidEngineImpl(context);
    }
  }

  @Override
  public EnvoyStream startStream(EnvoyObserver observer) {
    return envoyEngine.startStream(observer);
  }

  @Override
  public int runWithConfig(String config) {
    return envoyEngine.runWithConfig(config);
  }

  @Override
  public int runWithConfig(String config, String logLevel) {
    return envoyEngine.runWithConfig(config, logLevel);
  }

  /**
   * Native binding to register the ConnectivityManager to C-Ares
   *
   * @param connectivityManager Android's ConnectivityManager
   * @return int for successful initialization
   */
  private static native int initialize(ConnectivityManager connectivityManager);
}
