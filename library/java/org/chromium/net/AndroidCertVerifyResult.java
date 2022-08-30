package org.chromium.net;

import java.security.cert.CertificateEncodingException;
import java.security.cert.X509Certificate;
import java.util.ArrayList;
import java.util.Collections;
import java.util.List;

/**
 * The result of a certification verification.
 */
public final class AndroidCertVerifyResult {

  /**
   * The verification status. One of the values in CertVerifyStatusAndroid.
   */
  private final int mStatus;

  /**
   * True if the root CA in the chain is in the system store.
   */
  private final boolean mIsIssuedByKnownRoot;
  private int mIndicator = 11111;

  /**
   * The properly ordered certificate chain used for verification.
   */
  private final List<X509Certificate> mCertificateChain;

  public AndroidCertVerifyResult(int status, boolean isIssuedByKnownRoot,
                                 List<X509Certificate> certificateChain) {
    System.out.println("=========== AndroidCertVerifyResult1 " + status);
    mStatus = status;
    mIsIssuedByKnownRoot = isIssuedByKnownRoot;
    mCertificateChain = new ArrayList<X509Certificate>(certificateChain);
    mIndicator = 12345;
  }

  public AndroidCertVerifyResult(int status) {
    System.out.println("=========== AndroidCertVerifyResult2 " + status);
    mStatus = status;
    mIsIssuedByKnownRoot = false;
    mCertificateChain = Collections.<X509Certificate>emptyList();
    mIndicator = 54321;
  }

  // TODO(stefanoduo): Hook envoy-mobile JNI.
  //@CalledByNative
  public int getStatus() {
    System.out.println("========== getStatus: " + mStatus + " indicator " + mIndicator);
    return mStatus;
  }

  // TODO(stefanoduo): Hook envoy-mobile JNI.
  //@CalledByNative
  public boolean isIssuedByKnownRoot() { return mIsIssuedByKnownRoot; }

  // TODO(stefanoduo): Hook envoy-mobile JNI.
  //@CalledByNative
  public byte[][] getCertificateChainEncoded() {
    System.out.println("========= getCertificateChainEncoded: size " + mCertificateChain.size());
    byte[][] verifiedChainArray = new byte[mCertificateChain.size()][];
    try {
      for (int i = 0; i < mCertificateChain.size(); i++) {
        verifiedChainArray[i] = mCertificateChain.get(i).getEncoded();
      }
    } catch (CertificateEncodingException e) {
      return new byte[0][];
    }
    return verifiedChainArray;
  }
}
