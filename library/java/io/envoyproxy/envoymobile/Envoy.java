package io.envoyproxy.envoymobile;

import android.content.Context;
import io.envoyproxy.envoymobile.EnvoyEngine;

// Wrapper class that allows for easy calling of Envoy's JNI interface in native Java.
public class Envoy {

  // Dedicated thread for running this instance of Envoy.
  private final Thread runner;

  // Create a new Envoy instance. The Envoy runner Thread is started as part of instance
  // initialization with the configuration provided. If the Envoy native library and its
  // dependencies haven't been loaded and initialized yet, this will happen lazily when
  // the first instance is created.
  public Envoy(final Context context, final String config) {
    // Lazily initialize Envoy and its dependencies, if necessary.
    load(context);

    runner = new Thread(new Runnable() {
      @Override
      public void run() {
        EnvoyEngine.run(config.trim());
      }
    });
    runner.start();
  }

  public static void load(Context context) {
    EnvoyEngine.load(context);
  }

  // Returns whether the Envoy instance is currently active and running.
  public boolean isRunning() {
    final Thread.State state = runner.getState();
    return state != Thread.State.NEW && state != Thread.State.TERMINATED;
  }

  // Returns whether the Envoy instance is terminated.
  public boolean isTerminated() {
    return runner.getState() == Thread.State.TERMINATED;
  }
}
