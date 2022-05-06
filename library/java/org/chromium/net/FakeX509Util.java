package org.chromium.net;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.HashSet;
import java.util.Set;

/**
 * Fake utility functions to verify X.509 certificates.
 *
 * FakeX509Util is not particularly clever: from its perspective a certificate is just a string and
 * its contents have no particular meaning. For a verification to succeed all certificates in a
 * chain must have been previously registered as root certificates. This doesn't make much sense
 * w.r.t. how X.509 certificates are really validated, but we're not interested in mimicking that,
 * we just want something to confirm that JNI calls have taken place.
 */
public final class FakeX509Util {
  private static final Set<String> validFakeCerts = new HashSet<String>();

  public static void addTestRootCertificate(byte[] rootCertBytes) {
    String fakeCertificate = new String(rootCertBytes);
    validFakeCerts.add(fakeCertificate);
  }

  public static void clearTestRootCertificates() { validFakeCerts.clear(); }

  /* Fake certificate chain verification. Returns CertVerifyStatusAndroid.OK only if all
   * certificates in the chain have been previously registered as root certificates,
   * CertVerifyStatusAndroid.NO_TRUSTED_ROOT otherwise.
   */
  public static AndroidCertVerifyResult verifyServerCertificates(byte[][] certChain,
                                                                 String authType, String host) {
    if (certChain == null || certChain.length == 0 || certChain[0] == null) {
      throw new IllegalArgumentException(
          "Expected non-null and non-empty certificate "
          + "chain passed as |certChain|. |certChain|=" + Arrays.deepToString(certChain));
    }

    for (byte[] cert : certChain) {
      String fakeCert = new String(cert);
      if (!validFakeCerts.contains(fakeCert)) {
        return new AndroidCertVerifyResult(CertVerifyStatusAndroid.NO_TRUSTED_ROOT);
      }
    }

    return new AndroidCertVerifyResult(CertVerifyStatusAndroid.OK);
  }
}
