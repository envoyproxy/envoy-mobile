package io.envoyproxy.envoymobile

/**
 * Available upstream http protocols.
 */
enum class UpstreamHttpProtocol(internal val stringValue: String) {
  HTTP1("http1"),
  HTTP2("http2"),
}
