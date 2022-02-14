package io.envoyproxy.envoymobile.engine.testing;

import java.util.concurrent.atomic.AtomicBoolean;

/**
 * Wrapper class to start a Quic test server.
 */
public final class QuicTestServer {

  private static final String ECHO_HEADER_PATH = "/echo_header";
  private static final String ECHO_METHOD_PATH = "/echo_method";
  private static final String ECHO_ALL_HEADERS_PATH = "/echo_all_headers";
  private static final String REDIRECT_TO_ECHO_BODY_PATH = "/redirect_to_echo_body";
  private static final String ECHO_BODY_PATH = "/echo_body";

  private static final AtomicBoolean sServerRunning = new AtomicBoolean();

  /*
   * Starts the server. Throws an {@link IllegalStateException} if already started.
   */
  public static void startQuicTestServer() {
    if (!sServerRunning.compareAndSet(false, true)) {
      throw new IllegalStateException("Quic server is already running");
    }
    nativeStartQuicTestServer();
  }

  /**
   * Shutdowns the server. No-op if the server is already shutdown.
   */
  public static void shutdownQuicTestServer() {
    if (!sServerRunning.compareAndSet(true, false)) {
      return;
    }
    nativeShutdownQuicTestServer();
  }

  public static String getServerURL() {
    return "https://" + getServerHost() + ":" + getServerPort();
  }

  public static String getServerHost() { return "test.example.com"; }

  /**
   * Returns the server attributed port. Throws an {@link IllegalStateException} if not started.
   */
  public static int getServerPort() {
    if (!sServerRunning.get()) {
      throw new IllegalStateException("Quic server not started.");
    }
    return nativeGetServerPort();
  }

  public static String getEchoBodyURL() { return getUrl(ECHO_BODY_PATH); }

  public static String getEchoHeaderURL(String header) {
    return getUrl(ECHO_HEADER_PATH + "?" + header);
  }

  public static String getEchoAllHeadersURL() { return getUrl(ECHO_ALL_HEADERS_PATH); }

  public static String getEchoMethodURL() { return getUrl(ECHO_METHOD_PATH); }

  public static String getRedirectToEchoBody() { return getUrl(REDIRECT_TO_ECHO_BODY_PATH); }

  private static String getUrl(String path) { return getServerURL() + path; }

  private static native void nativeStartQuicTestServer();

  private static native void nativeShutdownQuicTestServer();

  private static native int nativeGetServerPort();

  private QuicTestServer() {}
}
