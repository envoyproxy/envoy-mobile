package io.envoyproxy.envoymobile

import java.nio.ByteBuffer

/*
 * Status returned by filters when transmitting or receiving data.
 */
sealed class FilterDataStatus {
  /**
   * Continue filter chain iteration. If headers have not yet been sent to the next filter, they
   * will be sent first via `onRequestHeaders()`/`onResponseHeaders()`.
   *
   * If data has previously been buffered, the data returned will be added to the buffer
   * before the entirety is sent to the next filter.
   * TODO: add param docs. name param in swift.
   */
  class Continue(val data: ByteBuffer) : FilterDataStatus()

  /**
   * Do not iterate to any of the remaining filters in the chain, and buffer body data for later
   * dispatching. The data returned here will be added to the buffer.
   *
   * This filter will continue to be called with new chunks of data.
   *
   * Returning `Continue` from `onRequestData()`/`onResponseData()` or calling
   * `continueRequest()`/`continueResponse()` MUST be called when continued filter iteration is
   * desired.
   *
   * This should be called by filters which must parse a larger block of the incoming data before
   * continuing processing.
   */
  class StopIterationAndBuffer(val data: ByteBuffer) : FilterDataStatus()

  /**
   * Do not iterate to any of the remaining filters in the chain, and do not internally buffer
   * data.
   *
   * This filter will continue to be called with new chunks of data.
   *
   * Returning `Continue` from `onRequestData()`/`onResponseData()` or calling
   * `continueRequest()`/`continueResponse()` MUST be called when continued filter iteration is
   * desired.
   *
   * This may be called by filters which must parse a larger block of the incoming data before
   * continuing processing, and will handle their own buffering.
  class StopIterationNoBuffer : FilterDataStatus()
}
