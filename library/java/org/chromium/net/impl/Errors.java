package org.chromium.net.impl;

import android.util.Log;
import androidx.annotation.StringDef;
import java.lang.annotation.Retention;
import java.lang.annotation.RetentionPolicy;
import org.chromium.net.NetworkException;

public class Errors {
  /**Subset of errors defined in
   * https://github.com/envoyproxy/envoy/blob/main/envoy/stream_info/stream_info.h */
  @StringDef({EnvoyMobileError.DNS_RESOLUTION_FAILED, EnvoyMobileError.DURATION_TIMEOUT,
              EnvoyMobileError.STREAM_IDLE_TIMEOUT, EnvoyMobileError.UPSTREAM_CONNECTION_FAILURE,
              EnvoyMobileError.UPSTREAM_CONNECTION_TERMINATION,
              EnvoyMobileError.UPSTREAM_REMOTE_RESET})
  @Retention(RetentionPolicy.SOURCE)
  public @interface EnvoyMobileError {
    String DNS_RESOLUTION_FAILED = "0x4000000";
    String DURATION_TIMEOUT = "0x400000";
    String STREAM_IDLE_TIMEOUT = "0x10000";
    String UPSTREAM_CONNECTION_FAILURE = "0x20";
    String UPSTREAM_CONNECTION_TERMINATION = "0x40";
    String UPSTREAM_REMOTE_RESET = "0x10";
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

  public static NetError mapEnvoyMobileErrorToNetError(long responseFlag) {
    String errorCode = "0x" + Long.toHexString(responseFlag);
    switch (errorCode) {
    case EnvoyMobileError.DNS_RESOLUTION_FAILED:
      // Todo(colibie): if NetworkChangeNofitier.isOffline, return
      // NetError.ERR_INTERNET_DISCONNECTED
      return NetError.ERR_NAME_NOT_RESOLVED;
    case EnvoyMobileError.DURATION_TIMEOUT:
    case EnvoyMobileError.STREAM_IDLE_TIMEOUT:
      return NetError.ERR_TIMED_OUT;
    case EnvoyMobileError.UPSTREAM_CONNECTION_TERMINATION:
      return NetError.ERR_CONNECTION_CLOSED;
    case EnvoyMobileError.UPSTREAM_REMOTE_RESET:
      return NetError.ERR_CONNECTION_RESET;
    case EnvoyMobileError.UPSTREAM_CONNECTION_FAILURE:
      // Todo(colibie) if NetworkChangeNofitier.isOffline, return NetError.ERR_INTERNET_DISCONNECTED
      return NetError
          .ERR_CONNECTION_REFUSED; // or ERR_ADDRESS_UNREACHABLE or ERR_CONNECTION_TIMED_OUT
    default:
      return NetError.ERR_OTHER;
      // case EnvoyMobileError.NETWORK_CHANGED:
      //   return NetError.ERR_NETWORK_CHANGED;
      //  Todo(colibie): if negotiated_protocol is quic
      // case net::ERR_QUIC_PROTOCOL_ERROR:
      //   return QUIC_PROTOCOL_FAILED;
    }
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

  private Errors() {}
}
