package org.chromium.net.impl;

import org.chromium.net.impl.Errors.NetError;

/**
 * Used in {@link CronetBidirectionalStream}. Implements {@link NetworkExceptionImpl}.
 */
public final class BidirectionalStreamNetworkException extends NetworkExceptionImpl {
  public BidirectionalStreamNetworkException(String message, int errorCode,
                                             int cronetInternalErrorCode) {
    super(message, errorCode, cronetInternalErrorCode);
  }

  @Override
  public boolean immediatelyRetryable() {
    if (mCronetInternalErrorCode == NetError.ERR_HTTP2_PING_FAILED.getValue() ||
        mCronetInternalErrorCode == NetError.ERR_QUIC_HANDSHAKE_FAILED.getValue()) {
      assert mErrorCode == ERROR_OTHER;
      return true;
    }
    return super.immediatelyRetryable();
  }
}
