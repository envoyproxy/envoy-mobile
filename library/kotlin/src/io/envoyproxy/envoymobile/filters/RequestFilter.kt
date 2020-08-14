package io.envoyproxy.envoymobile

import java.nio.ByteBuffer

/*
 * Filter executed for outbound requests, providing the ability to observe and mutate streams.
 */
interface RequestFilter : Filter {
  /**
   * Called once when the request is initiated.
   *
   * Filters may mutate or delay the request headers.
   *
   * @param headers:   The current request headers.
   * @param endStream: Whether this is a headers-only request.
   *
   * @return: The header status containing headers with which to continue or buffer.
   */
  fun onRequestHeaders(headers: RequestHeaders, endStream: Boolean):
    FilterHeadersStatus<RequestHeaders>

  /**
   * Called any number of times whenever body data is sent.
   *
   * Filters may mutate or buffer (defer and concatenate) the data.
   *
   * @param body:      The outbound body data chunk.
   * @param endStream: Whether this is the last data frame.
   *
   * @return: The data status containing body with which to continue or buffer.
   */
  fun onRequestData(body: ByteBuffer, endStream: Boolean): FilterDataStatus<RequestHeaders>

  /**
   * Called at most once when the request is closed from the client with trailers.
   *
   * Filters may mutate or delay the trailers. Note trailers imply the stream has ended.
   *
   * @param trailers: The outbound trailers.
   *
   * @return: The trailer status containing body with which to continue or buffer.
   */
  fun onRequestTrailers(trailers: RequestTrailers): FilterTrailersStatus<RequestHeaders, RequestTrailers>
}
