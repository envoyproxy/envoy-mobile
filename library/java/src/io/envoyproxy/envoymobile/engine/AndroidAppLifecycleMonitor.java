package io.envoyproxy.envoymobile.engine;

import android.app.Activity;
import android.app.Application.ActivityLifecycleCallbacks;
import android.annotation.TargetApi;
import android.os.Build;
import android.os.Bundle;

@TargetApi(Build.VERSION_CODES.LOLLIPOP)
public class AndroidAppLifecycleMonitor implements ActivityLifecycleCallbacks {
  @Override
  public void onActivityCreated(Activity activity, Bundle savedInstanceState) {
    /* Required override for interface implementation */
  }

  @Override
  public void onActivityStarted(Activity activity) {
    /* Required override for interface implementation */
  }

  @Override
  public void onActivityResumed(Activity activity) {
    /* Required override for interface implementation */
  }

  @Override
  public void onActivitySaveInstanceState(Activity activity, Bundle outState) {
    /* Required override for interface implementation */
  }

  @Override
  public void onActivityPaused(Activity activity) {
    AndroidJniLibrary.flushStats();
  }

  @Override
  public void onActivityStopped(Activity activity) {
    AndroidJniLibrary.flushStats();
  }

  @Override
  public void onActivityDestroyed(Activity activity) {
    AndroidJniLibrary.flushStats();
  }
}
