package org.chromium.net.testing;

/**
 * A Java wrapper to supply a net::MockCertVerifier which can be then passed
 * into {@link CronetEngine.Builder#setMockCertVerifierForTesting}.
 * The native pointer will be freed when the CronetEngine is torn down.
 */
public class MockCertVerifier {
  private MockCertVerifier() {}

  /**
   * Creates a new net::MockCertVerifier, and returns a pointer to it.
   * @param certs a String array of certificate filenames in
   *         net::GetTestCertsDirectory() to accept in testing.
   * @return a pointer to the newly created net::MockCertVerifier.
   */
  public static long createMockCertVerifier(String[] certs, boolean knownRoot) {
    // TODO: to be implemented
    return 0L;
  }
}
