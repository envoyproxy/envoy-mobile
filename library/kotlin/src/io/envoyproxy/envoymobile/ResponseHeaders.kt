package io.envoyproxy.envoymobile

/*
 * Headers representing an inbound response.
 */
class ResponseHeaders : Headers {
  /**
   * Internal constructor used by builders.
   *
   * @param headers: Headers to set.
   */
  internal constructor(headers: Map<String, List<String>>) : super(headers)

  /**
   * HTTP status code received with the response.
   */
  val httpStatus: Int? by lazy { value(":status")?.first()?.toIntOrNull() }

  /**
   * Convert the headers back to a builder for mutation.
   *
   * @return ResponseHeadersBuilder, The new builder.
   */
  fun toResponseHeadersBuilder(): ResponseHeadersBuilder {
    return ResponseHeadersBuilder(this.headers.mapValues { it.value.toMutableList() }
      .toMutableMap())
  }
}
