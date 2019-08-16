package io.envoyproxy.envoymobile.engine;

import android.content.Context;
import android.net.ConnectivityManager;

public class AndroidEngineImpl extends EnvoyEngineImpl {

  // Internal reference to helper object used to load and initialize the native library.
  // Volatile to ensure double-checked locking works correctly.
  private static volatile AndroidLoader loader = null;

  // Private helper class used by the load method to ensure the native library and its
  // dependencies are loaded and initialized at most once.
  public AndroidEngineImpl(Context context) {
    load(context);
  }

  protected void load(Context context) {
    EnvoyEngineImpl.load();

    if (loader != null) {
      return;
    }

    synchronized (AndroidLoader.class) {
      if (loader != null) {
        return;
      }

      loader = new AndroidLoader(context);
    }
  }

  private static class AndroidLoader {
    private AndroidLoader(Context context) {
      initialize((ConnectivityManager) context.getSystemService(Context.CONNECTIVITY_SERVICE));
    }

    /**
     * Native binding to register the ConnectivityManager to C-Ares
     *
     * @param connectivityManager Android's ConnectivityManager
     * @return int for successful initialization
     */
    private static native int initialize(ConnectivityManager connectivityManager);
  }

}
