package org.chromium.net;

import static org.junit.Assert.assertEquals;

import io.envoyproxy.envoymobile.engine.AndroidJniLibrary;
import io.envoyproxy.envoymobile.engine.JniLibrary;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.robolectric.RobolectricTestRunner;

/**
 * Simple test for Certificate verification JNI layer.
 * The objective is not to test the certificate verification logic (which is faked) but instead to
 * confirm that all JNI calls go through (confirmed by checking for the fake implementation side
 * effects).
 */
@RunWith(RobolectricTestRunner.class)
public final class CertificateVerificationTest {
  static {
    AndroidJniLibrary.loadTestLibrary();
    JniLibrary.load();
  }

  @Before
  public void setUp() throws Exception {
    AndroidNetworkLibrary.setFakeCertificateVerificationForTesting(true);
  }

  @After
  public void tearDown() throws Exception {
    JniLibrary.callClearTestRootCertificateFromNative();
    AndroidNetworkLibrary.setFakeCertificateVerificationForTesting(true);
  }

  @Test
  public void testChainWithNonRootCertificate() throws Exception {
    final String[] fakeCertChain = new String[] {"fake cert"};
    final byte[][] certChain = new byte[][] {fakeCertChain[0].getBytes()};
    final String authType = "";
    final String host = "";

    AndroidCertVerifyResult result =
        (AndroidCertVerifyResult)JniLibrary.callCertificateVerificationFromNative(certChain,
                                                                                  authType, host);
    assertEquals(result.getStatus(), CertVerifyStatusAndroid.NO_TRUSTED_ROOT);
  }

  @Test
  public void testChainWithRootCertificate() throws Exception {
    final String[] fakeCertChain = new String[] {"fake cert"};
    final byte[][] certChain = new byte[][] {fakeCertChain[0].getBytes()};
    final String authType = "";
    final String host = "";

    JniLibrary.callAddTestRootCertificateFromNative(certChain[0]);
    AndroidCertVerifyResult result =
        (AndroidCertVerifyResult)JniLibrary.callCertificateVerificationFromNative(certChain,
                                                                                  authType, host);
    assertEquals(result.getStatus(), CertVerifyStatusAndroid.OK);
  }

  @Test
  public void testClearTestRootCertificate() throws Exception {
    final String[] fakeCertChain = new String[] {"fake cert"};
    final byte[][] certChain = new byte[][] {fakeCertChain[0].getBytes()};
    final String authType = "";
    final String host = "";

    JniLibrary.callAddTestRootCertificateFromNative(certChain[0]);
    JniLibrary.callClearTestRootCertificateFromNative();
    AndroidCertVerifyResult result =
        (AndroidCertVerifyResult)JniLibrary.callCertificateVerificationFromNative(certChain,
                                                                                  authType, host);
    assertEquals(result.getStatus(), CertVerifyStatusAndroid.NO_TRUSTED_ROOT);
  }

  @Test
  public void testChainWithMultipleNonRootCertificates() throws Exception {
    final String[] fakeCertChain = new String[] {"fake cert", "another fake cert"};
    final byte[][] certChain =
        new byte[][] {fakeCertChain[0].getBytes(), fakeCertChain[1].getBytes()};
    final String authType = "";
    final String host = "";

    AndroidCertVerifyResult result =
        (AndroidCertVerifyResult)JniLibrary.callCertificateVerificationFromNative(certChain,
                                                                                  authType, host);
    assertEquals(result.getStatus(), CertVerifyStatusAndroid.NO_TRUSTED_ROOT);
  }

  @Test
  public void testChainWithMixedCertificates() throws Exception {
    final String[] fakeCertChain = new String[] {"fake cert", "another fake cert"};
    final byte[][] certChain =
        new byte[][] {fakeCertChain[0].getBytes(), fakeCertChain[1].getBytes()};
    final String authType = "";
    final String host = "";

    JniLibrary.callAddTestRootCertificateFromNative(certChain[0]);
    AndroidCertVerifyResult result =
        (AndroidCertVerifyResult)JniLibrary.callCertificateVerificationFromNative(certChain,
                                                                                  authType, host);
    assertEquals(result.getStatus(), CertVerifyStatusAndroid.NO_TRUSTED_ROOT);
  }

  @Test
  public void testChainWithMultipleRootCertificates() throws Exception {
    final String[] fakeCertChain = new String[] {"fake cert", "another fake cert"};
    final byte[][] certChain =
        new byte[][] {fakeCertChain[0].getBytes(), fakeCertChain[1].getBytes()};
    final String authType = "";
    final String host = "";

    JniLibrary.callAddTestRootCertificateFromNative(certChain[0]);
    JniLibrary.callAddTestRootCertificateFromNative(certChain[1]);
    AndroidCertVerifyResult result =
        (AndroidCertVerifyResult)JniLibrary.callCertificateVerificationFromNative(certChain,
                                                                                  authType, host);
    assertEquals(result.getStatus(), CertVerifyStatusAndroid.OK);
  }
}
