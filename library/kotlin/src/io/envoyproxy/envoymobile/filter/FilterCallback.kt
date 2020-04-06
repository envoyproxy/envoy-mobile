package io.envoyproxy.envoymobile.filter

import java.nio.ByteBuffer

interface FilterCallback

interface RequestResponseFilterCallback : RequestFilterCallback, ResponseFilterCallback

interface RequestFilterCallback : FilterCallback {
  /**
   * Continue iterating through the filter chain with buffered headers and body data. This routine
   * can only be called if the filter has previously returned StopIteration from onHeaders() AND
   * one of StopIterationAndBuffer, StopIterationAndWatermark, or StopIterationNoBuffer
   * from each previous call to onData().
   *
   * The connection manager will dispatch headers and any buffered body data to the next filter in
   * the chain. Further note that if the response is not complete, this filter will still receive
   * onData() calls and must return an appropriate status code depending on what the filter
   * needs to do.
   */
  fun continueRequest()

  /**
   * @return const ByteBuffer the currently buffered data as buffered by this filter or
   *         previous ones in the filter chain. May be nullptr if nothing has been buffered yet.
   */
  fun requestBuffer(): ByteBuffer

  /**
   * Allows modifying the encoding buffer. May only be called before any data has been continued
   * past the calling filter.
   */
  fun modifyRequestBuffer(callback: (ByteBuffer) -> Unit)

  /**
   * Adds request trailers. May only be called in onData when end_stream is set to true.
   * If called in any other context, an assertion will be triggered.
   *
   * When called in onData, the trailers map will be initialized to an empty map and returned by
   * reference. Calling this function more than once is invalid.
   *
   * @return a reference to the newly created trailers map.
   */
  fun addRequestTrailers(): Map<String, List<String>>
}

interface ResponseFilterCallback {

  /**
   * Continue iterating through the filter chain with buffered headers and body data. This routine
   * can only be called if the filter has previously returned StopIteration from onHeaders() AND
   * one of StopIterationAndBuffer, StopIterationAndWatermark, or StopIterationNoBuffer
   * from each previous call to onData().
   *
   * The connection manager will dispatch headers and any buffered body data to the next filter in
   * the chain. Further note that if the response is not complete, this filter will still receive
   * onData() calls and must return an appropriate status code depending on what the filter
   * needs to do.
   */
  fun continueResponse()

  /**
   * @return const ByteBuffer the currently buffered data as buffered by this filter or
   *         previous ones in the filter chain. May be nullptr if nothing has been buffered yet.
   */
  fun responseBuffer(): ByteBuffer

  /**
   * Allows modifying the encoding buffer. May only be called before any data has been continued
   * past the calling filter.
   */
  fun modifyResponseBuffer(callback: (ByteBuffer) -> Unit)

  /**
   * Adds response trailers. May only be called in onData when end_stream is set to true.
   * If called in any other context, an assertion will be triggered.
   *
   * When called in onData, the trailers map will be initialized to an empty map and returned by
   * reference. Calling this function more than once is invalid.
   *
   * @return a reference to the newly created trailers map.
   */
  fun addResponseTrailers(): Map<String, List<String>>
}