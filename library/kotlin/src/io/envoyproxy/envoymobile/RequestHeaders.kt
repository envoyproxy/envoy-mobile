
package io.envoyproxy.envoymobile

/*
 * Headers representing an outbound request.
 */
class RequestHeaders: Headers {
  /**
   * Internal constructor used by builders.
   *
   * @param headers: Headers to set.
   */
  internal constructor(headers: Map<String, List<String>>) : super(headers)

  /**
   * Method for the request.
   */
  val method = RequestMethod.enumValue(value(":method")?.first()!!)

  /**
   * The URL scheme for the request (i.e., "https").
   */
  val scheme = value(":scheme")?.first()!!

  /**
   * The URL authority for the request (i.e., "api.foo.com").
   */
  val authority = value(":authority")?.first()!!

  /**
   * The URL path for the request (i.e., "/foo").
   */
  val path = value(":path")?.first()!!

  /**
   * Retry policy to use for this request.
   */
  // TODO: retry policy mapper
  val retryPolicy = RetryPolicy(this)

  /**
   * The protocol version to use for upstream requests.
   */
  val upstreamHttpProtocol =
    UpstreamHttpProtocol.enumValue(value("x-envoy-mobile-upstream-protocol")?.first()!!)

  /**
   * Convert the headers back to a builder for mutation.
   *
   * @return RequestHeadersBuilder, The new builder.
   */
  fun toBuilder(): RequestHeadersBuilder {
    return RequestHeadersBuilder(this.headers.mapValues { it.value.toMutableList() }.toMutableMap())
  }
}
