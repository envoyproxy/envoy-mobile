package org.chromium.net.impl;

import android.util.Log;
import androidx.annotation.LongDef;
import java.lang.annotation.Retention;
import java.lang.annotation.RetentionPolicy;
import java.util.Collections;
import java.util.HashMap;
import java.util.Map;
import org.chromium.net.NetworkException;

public class Errors {
  private static final Map<Long, NetError> EnvoyMobileErrorToNetError = createErrorMapping();

  /**Subset of errors defined in
   * https://github.com/envoyproxy/envoy/blob/main/envoy/stream_info/stream_info.h */
  @LongDef({EnvoyMobileError.DNS_RESOLUTION_FAILED, EnvoyMobileError.DURATION_TIMEOUT,
            EnvoyMobileError.STREAM_IDLE_TIMEOUT, EnvoyMobileError.UPSTREAM_CONNECTION_FAILURE,
            EnvoyMobileError.UPSTREAM_CONNECTION_TERMINATION,
            EnvoyMobileError.UPSTREAM_REMOTE_RESET})
  @Retention(RetentionPolicy.SOURCE)
  public @interface EnvoyMobileError {
    long DNS_RESOLUTION_FAILED = 0x4000000;
    long DURATION_TIMEOUT = 0x400000;
    long STREAM_IDLE_TIMEOUT = 0x10000;
    long UPSTREAM_CONNECTION_FAILURE = 0x20;
    long UPSTREAM_CONNECTION_TERMINATION = 0x40;
    long UPSTREAM_REMOTE_RESET = 0x10;
  }

  /** Subset of errors defined in chromium/src/net/base/net_error_list.h */
  public enum NetError {
    ERR_NETWORK_CHANGED(-21),
    ERR_HTTP2_PING_FAILED(-352),
    ERR_QUIC_PROTOCOL_ERROR(-356),
    ERR_QUIC_HANDSHAKE_FAILED(-358),
    ERR_NAME_NOT_RESOLVED(-105),
    ERR_INTERNET_DISCONNECTED(-106),
    ERR_TIMED_OUT(-7),
    ERR_CONNECTION_CLOSED(-100),
    ERR_CONNECTION_TIMED_OUT(-118),
    ERR_CONNECTION_REFUSED(-102),
    ERR_CONNECTION_RESET(-101),
    ERR_ADDRESS_UNREACHABLE(-109),
    ERR_OTHER(-1000);

    private final int errorCode;

    NetError(int errorCode) { this.errorCode = errorCode; }

    public int getErrorCode() { return errorCode; }

    public String getModifiedString() { return "net::" + this.toString(); }
  }

  /**
   * returns the NetError that the EnvoyMobileError maps to
   */
  public static NetError mapEnvoyMobileErrorToNetError(long responseFlag) {
    /* Todo(https://github.com/envoyproxy/envoy-mobile/issues/1594):
     * if (EnvoyMobileError.DNS_RESOLUTION_FAILED || EnvoyMobileError.UPSTREAM_CONNECTION_FAILURE)
     * && NetworkChangeNotifier.isOffline return NetError.ERR_INTERNET_DISCONNECTED
     *
     * if negotiated_protocol is quic return QUIC_PROTOCOL_FAILED
     */
    return EnvoyMobileErrorToNetError.getOrDefault(responseFlag, NetError.ERR_OTHER);
  }

  public static int mapNetErrorToCronetApiErrorCode(NetError netError) {
    switch (netError) {
    case ERR_NAME_NOT_RESOLVED:
      return NetworkException.ERROR_HOSTNAME_NOT_RESOLVED;
    case ERR_TIMED_OUT:
      return NetworkException.ERROR_TIMED_OUT;
    case ERR_CONNECTION_CLOSED:
      return NetworkException.ERROR_CONNECTION_CLOSED;
    case ERR_CONNECTION_RESET:
      return NetworkException.ERROR_CONNECTION_RESET;
    case ERR_CONNECTION_REFUSED:
      return NetworkException.ERROR_CONNECTION_REFUSED;
    case ERR_OTHER:
      return NetworkException.ERROR_OTHER;
    case ERR_INTERNET_DISCONNECTED:
      return NetworkException.ERROR_INTERNET_DISCONNECTED;
    case ERR_NETWORK_CHANGED:
      return NetworkException.ERROR_NETWORK_CHANGED;
    case ERR_QUIC_PROTOCOL_ERROR:
      return NetworkException.ERROR_QUIC_PROTOCOL_FAILED;
    }
    Log.e(CronetUrlRequestContext.LOG_TAG, "Unknown error code: " + netError);
    return NetworkException.ERROR_OTHER;
  }

  private static Map<Long, NetError> createErrorMapping() {
    Map<Long, NetError> errorMap = new HashMap<>();
    errorMap.put(EnvoyMobileError.DNS_RESOLUTION_FAILED, NetError.ERR_NAME_NOT_RESOLVED);
    errorMap.put(EnvoyMobileError.DURATION_TIMEOUT, NetError.ERR_TIMED_OUT);
    errorMap.put(EnvoyMobileError.STREAM_IDLE_TIMEOUT, NetError.ERR_TIMED_OUT);
    errorMap.put(EnvoyMobileError.UPSTREAM_CONNECTION_TERMINATION, NetError.ERR_CONNECTION_CLOSED);
    errorMap.put(EnvoyMobileError.UPSTREAM_REMOTE_RESET, NetError.ERR_CONNECTION_RESET);
    errorMap.put(EnvoyMobileError.UPSTREAM_CONNECTION_FAILURE, NetError.ERR_CONNECTION_REFUSED);
    return Collections.unmodifiableMap(errorMap);
  }

  private Errors() {}
}
