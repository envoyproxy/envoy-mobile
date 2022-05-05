package org.chromium.net;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.HashSet;
import java.util.Set;

/**
 * Fake utility functions for verifying X.509 certificates.
 */
public final class FakeX509Util {
  private static final String TAG = "FakeX509Util";

  private static final Set<String> validFakeCerts = new HashSet<String>();

  public static void addTestRootCertificate(byte[] rootCertBytes) {
    String fakeCertificate = new String(rootCertBytes);
    validFakeCerts.add(fakeCertificate);
  }

  public static void clearTestRootCertificates() { validFakeCerts.clear(); }

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
