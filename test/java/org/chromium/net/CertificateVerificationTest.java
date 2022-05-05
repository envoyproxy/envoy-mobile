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
 * Simple test for Certificate verification.
 */
@RunWith(RobolectricTestRunner.class)
public class CertificateVerificationTest {
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
  public void testVerifyCertificateCall() throws Exception {
    String[] fakeCertChain1 = new String[] {"fake cert"};
    byte[][] certChain1 = new byte[][] {fakeCertChain1[0].getBytes()};

    String[] fakeCertChain2 = new String[] {"fake cert", "another fake cert"};
    byte[][] certChain2 = new byte[][] {fakeCertChain2[0].getBytes(), fakeCertChain2[1].getBytes()};

    AndroidCertVerifyResult result =
        (AndroidCertVerifyResult)JniLibrary.callCertificateVerificationFromNative(certChain1, "",
                                                                                  "");
    assertEquals(result.getStatus(), CertVerifyStatusAndroid.NO_TRUSTED_ROOT);
    result = (AndroidCertVerifyResult)JniLibrary.callCertificateVerificationFromNative(certChain2,
                                                                                       "", "");
    assertEquals(result.getStatus(), CertVerifyStatusAndroid.NO_TRUSTED_ROOT);

    JniLibrary.callAddTestRootCertificateFromNative(certChain1[0]);
    result = (AndroidCertVerifyResult)JniLibrary.callCertificateVerificationFromNative(certChain1,
                                                                                       "", "");
    assertEquals(result.getStatus(), CertVerifyStatusAndroid.OK);
    result = (AndroidCertVerifyResult)JniLibrary.callCertificateVerificationFromNative(certChain2,
                                                                                       "", "");
    assertEquals(result.getStatus(), CertVerifyStatusAndroid.NO_TRUSTED_ROOT);

    JniLibrary.callAddTestRootCertificateFromNative(certChain2[1]);
    result = (AndroidCertVerifyResult)JniLibrary.callCertificateVerificationFromNative(certChain2,
                                                                                       "", "");
    assertEquals(result.getStatus(), CertVerifyStatusAndroid.OK);

    JniLibrary.callClearTestRootCertificateFromNative();
    result = (AndroidCertVerifyResult)JniLibrary.callCertificateVerificationFromNative(certChain1,
                                                                                       "", "");
    assertEquals(result.getStatus(), CertVerifyStatusAndroid.NO_TRUSTED_ROOT);
    result = (AndroidCertVerifyResult)JniLibrary.callCertificateVerificationFromNative(certChain2,
                                                                                       "", "");
    assertEquals(result.getStatus(), CertVerifyStatusAndroid.NO_TRUSTED_ROOT);
  }
}
