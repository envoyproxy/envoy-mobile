package org.chromium.net.testing;

import static junit.framework.Assert.assertTrue;

import org.chromium.net.CronetEngine;
import org.chromium.net.ExperimentalCronetEngine;

/**
 * Helper class to set up url interceptors for testing purposes.
 * TODO("https://github.com/envoyproxy/envoy-mobile/issues/1549")
 */
public final class MockUrlRequestJobFactory {

  private static final String TEST_URL = "http://0.0.0.0:10000";

  private final CronetEngine mCronetEngine;

  /**
   * Sets up URL interceptors.
   */
  public MockUrlRequestJobFactory(CronetEngine cronetEngine) {
    mCronetEngine = cronetEngine;

    // Add a filter to immediately return a response
  }

  /**
   * Sets up URL interceptors.
   */
  public MockUrlRequestJobFactory(ExperimentalCronetEngine.Builder builder) {
    // Add a filter to immediately return a response
    mCronetEngine =
        CronetTestUtil.getCronetEngineBuilderImpl(builder).addUrlInterceptorsForTesting().build();
  }

  /**
   * Remove URL Interceptors.
   */
  public void shutdown() {
    // Remove the filter;
    mCronetEngine.shutdown();
  }

  public CronetEngine getCronetEngine() { return mCronetEngine; }

  /**
   * Constructs a mock URL that hangs or fails at certain phase.
   *
   * @param phase at which request fails. It should be a value in
   *              org.chromium.net.test.FailurePhase.
   * @param @param envoyMobileError reported by the engine.
   */
  public static String getMockUrlWithFailure(long envoyMobileError) {
    return TEST_URL + "/failed?error=" + envoyMobileError;
  }

  public static String getMockQuicUrlWithFailure(long envoyMobileError) {
    return TEST_URL + "/failed?quic=1&error=" + envoyMobileError;
  }

  /**
   * Constructs a mock URL that synchronously responds with data repeated many
   * times.
   *
   * @param data to return in response.
   * @param dataRepeatCount number of times to repeat the data.
   */
  public static String getMockUrlForData(String data, int dataRepeatCount) {
    return "To be implemented";
  }

  /**
   * Constructs a mock URL that will request client certificate and return
   * the string "data" as the response.
   */
  public static String getMockUrlForClientCertificateRequest() { return "To be implemented"; }

  /**
   * Constructs a mock URL that will fail with an SSL certificate error.
   */
  public static String getMockUrlForSSLCertificateError() { return "To be implemented"; }

  /**
   * Constructs a mock URL that will hang when try to read response body from the remote.
   */
  public static String getMockUrlForHangingRead() { return "To be implemented"; }
}
