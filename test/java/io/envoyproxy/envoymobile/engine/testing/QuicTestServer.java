package io.envoyproxy.envoymobile.engine.testing;

import java.util.concurrent.atomic.AtomicBoolean;

/**
 * Wrapper class to start a Quic test server.
 */
public final class QuicTestServer {

  private static final String CERT_USED = "upstreamcert.pem";
  private static final String KEY_USED = "upstreamkey.pem";

  private static final AtomicBoolean sServerRunning = new AtomicBoolean();

  /*
   * Starts the server.
   */
  public static void startQuicTestServer() {
    if (!sServerRunning.compareAndSet(false, true)) {
      throw new IllegalStateException("Quic server is already running");
    }
    nativeStartQuicTestServer();
  }

  /**
   * Shuts down the server. No-op if the server is already shut down.
   */
  public static void shutdownQuicTestServer() {
    if (!sServerRunning.compareAndSet(true, false)) {
      return;
    }
    nativeShutdownQuicTestServer();
  }

  public static String getServerURL() {
    return "https://" + getServerHost() + ":" + getServerPort() + "/";
  }

  public static String getServerHost() { return "test.example.com"; }

  public static int getServerPort() { return nativeGetServerPort(); }

  public static final String getServerCert() { return CERT_USED; }

  public static final String getServerCertKey() { return KEY_USED; }

  public static long createMockCertVerifier() {
    // to be implemented
    return 0L;
  }

  private static native void nativeStartQuicTestServer();

  private static native void nativeShutdownQuicTestServer();

  private static native int nativeGetServerPort();

  private QuicTestServer() {}
}
