package io.envoyproxy.envoymobile.engine;

import android.annotation.TargetApi;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.net.ConnectivityManager;
import android.net.Proxy;
import android.net.ProxyInfo;
import android.os.Build;

@TargetApi(Build.VERSION_CODES.LOLLIPOP)
public class AndroidProxyMonitor extends BroadcastReceiver {
  private static volatile AndroidProxyMonitor instance = null;
  private ConnectivityManager connectivityManager;
  private EnvoyEngine envoyEngine;

  public static void load(Context context, EnvoyEngine envoyEngine) {
    if (instance != null) {
      return;
    }

    synchronized (AndroidProxyMonitor.class) {
      if (instance != null) {
        return;
      }
      instance = new AndroidProxyMonitor(context, envoyEngine);
    }
  }

  private AndroidProxyMonitor(Context context, EnvoyEngine envoyEngine) {
    this.envoyEngine = envoyEngine;
    this.connectivityManager =
        (ConnectivityManager)context.getSystemService(Context.CONNECTIVITY_SERVICE);
    registerReceiver(context);
    envoyEngine.setProxySettings("info.getHost()", "info.getHost()2");
  }

  private void registerReceiver(Context context) {
    context.getApplicationContext().registerReceiver(this, new IntentFilter() {
      { addAction(Proxy.PROXY_CHANGE_ACTION); }
    });
  }

  @Override
  public void onReceive(Context context, Intent intent) {
    handleProxyChange();
  }

  private void handleProxyChange() { 
    // print("")
    // ENVOY_LOG_EVENT(debug, "RAF: identifier", "Proxy change");
    ProxyInfo info = connectivityManager.getDefaultProxy();
    envoyEngine.setProxySettings(info.getHost(), info.getHost());
 }
}
