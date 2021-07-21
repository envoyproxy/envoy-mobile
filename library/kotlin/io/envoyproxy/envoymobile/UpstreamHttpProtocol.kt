package io.envoyproxy.envoymobile

import java.lang.IllegalArgumentException

/**
 * Available upstream HTTP protocols.
 */
enum class UpstreamHttpProtocol(internal val stringValue: String) {
  ALPN("alpn"),
  HTTP1("http1"),
  HTTP2("http2");

  companion object {
    internal fun enumValue(stringRepresentation: String): UpstreamHttpProtocol {
      return when (stringRepresentation) {
        "alpn"  -> UpstreamHttpProtocol.ALPN
        "http1" -> UpstreamHttpProtocol.HTTP1
        "http2" -> UpstreamHttpProtocol.HTTP2
        else -> throw IllegalArgumentException("invalid value $stringRepresentation")
      }
    }
  }
}
