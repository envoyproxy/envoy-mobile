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
import android.os.Bundle;

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
  }

  private void registerReceiver(Context context) {
    context.getApplicationContext().registerReceiver(this, new IntentFilter() {
      { addAction(Proxy.PROXY_CHANGE_ACTION); }
    });
  }

  @Override
  public void onReceive(Context context, Intent intent) {
    handleProxyChange(intent);
  }

  private void handleProxyChange(final Intent intent) {
    ProxyInfo info = this.extractProxyInfo(intent);

    if (info == null) {
      envoyEngine.setProxySettings("", 0);
    } else {
      envoyEngine.setProxySettings(info.getHost(), info.getPort());
    }
  }

  private ProxyInfo extractProxyInfo(final Intent intent) {
    ProxyInfo info = connectivityManager.getDefaultProxy();

    if (info.getPacFileUrl() != null) {
      Bundle extras = intent.getExtras();
      if (extras == null) {
        return null;
      }

      info = (ProxyInfo)extras.get("android.intent.extra.PROXY_INFO");
    }

    return info;
  }
}
