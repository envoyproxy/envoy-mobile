package io.envoyproxy.envoymobile.engine.types;

public enum EnvoyErrorCode {
  ENVOY_STREAM_RESET, ENVOY_INVALID_VALUE;

  public static EnvoyErrorCode fromInt(int val) {
    switch (val) {
    case 0:
      return ENVOY_STREAM_RESET;
    default:
      return ENVOY_INVALID_VALUE;
    }
  }
}
