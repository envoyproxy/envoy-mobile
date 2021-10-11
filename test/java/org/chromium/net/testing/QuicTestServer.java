package org.chromium.net.testing;

import android.content.Context;
import android.os.ConditionVariable;
import android.util.Log;

/**
 * Wrapper class to start a Quic test server.
 */

public final class QuicTestServer {
  private static final ConditionVariable sBlock = new ConditionVariable();
  private static final String TAG = QuicTestServer.class.getSimpleName();

  private static final String CERT_USED = "upstreamcert.pem";
  private static final String KEY_USED = "upstreamkey.pem";
  private static final String[] CERTS_USED = {CERT_USED};

  private static boolean sServerRunning;

  /*
   * Starts the server.
   */
  public static void startQuicTestServer(Context context) {
    if (sServerRunning) {
      throw new IllegalStateException("Quic server is already running");
    }
    TestFilesInstaller.installIfNeeded(context);
    nativeStartQuicTestServer(TestFilesInstaller.getInstalledPath(context),
                              UrlUtils.getIsolatedTestRoot());
    sServerRunning = true;
  }

  /**
   * Shuts down the server. No-op if the server is already shut down.
   */
  public static void shutdownQuicTestServer() {
    if (!sServerRunning) {
      return;
    }
    nativeShutdownQuicTestServer();
    sServerRunning = false;
  }

  public static String getServerURL() {
    return "https://" + getServerHost() + ":" + getServerPort() + "/";
  }

  public static String getServerHost() { return "test.example.com"; }

  public static int getServerPort() { return nativeGetServerPort(); }

  public static final String getServerCert() { return CERT_USED; }

  public static final String getServerCertKey() { return KEY_USED; }

  public static long createMockCertVerifier() {
    TestFilesInstaller.installIfNeeded(ContextUtils.getApplicationContext());
    return MockCertVerifier.createMockCertVerifier(CERTS_USED, true);
  }

  // @CalledByNative
  private static void onServerStarted() {
    Log.i(TAG, "Quic server started.");
    sBlock.open();
  }

  private static native void nativeStartQuicTestServer(String filePath, String testDataDir);
  private static native void nativeShutdownQuicTestServer();
  private static native int nativeGetServerPort();
}
