package io.envoyproxy.envoymobile.filter

import io.envoyproxy.envoymobile.EnvoyError
import io.envoyproxy.envoymobile.Request
import java.nio.ByteBuffer

interface Filter

interface RequestResponseFilter : RequestFilter, ResponseFilter

interface RequestFilter : Filter {

  /**
   * Called by the filter manager once to initialize the filter callbacks that the filter should
   * use. Callbacks will not be invoked by the filter after onDestroy() is called.
   */
  fun setRequestFilterCallbacks(callbacks: RequestFilterCallback)

  /**
   * Called once when the request is initiated.
   *
   * Filters may mutate or delay the request.
   *
   * For the request to continue, each filter MUST call `callback.onRequest()` at some point.
   * Alternatively, filters may cancel the request by calling one or more response callbacks.
   *
   * @param request The outbound request.
   * @return The header status.
   */
  fun onRequest(request: Request): HeaderStatus

  /**
   * Called any number of times whenever body data is sent.
   *
   * Filters may mutate or buffer (defer and concatenate) the data.
   *
   * For the request to continue, each filter MUST call `callback.onRequestData()` at some point.
   * Alternatively, filters may end the stream by calling one or more response callbacks.
   *
   * @param body      The outbound body data chunk.
   * @param endStream Whether this represents the end of a stream/request. If false, the stream will
   *                        continue to remain open after this data is sent.
   * @return The data status.
   */
  fun onRequestData(body: ByteBuffer, endStream: Boolean): DataStatus

  /**
   * Called at most once when the request is closed from the client with trailers.
   *
   * Filters may mutate or delay the trailers.
   *
   * For the request to continue, each filter MUST call `callback.onTrailers()` at some point.
   * Alternatively, filters may cancel the request by calling one or more response callbacks.
   *
   * @param trailers The outbound trailers.
   * @return The trailer status
   */
  fun onRequestTrailers(trailers: Map<String, List<String>>?): TrailerStatus
}

interface ResponseFilter : Filter {

  /**
   * Called by the filter manager once to initialize the filter callbacks that the filter should
   * use. Callbacks will not be invoked by the filter after onDestroy() is called.
   */
  fun setResponseFilterCallbacks(callbacks: ResponseFilterCallback)

  /**
   * Called once when headers are received.
   *
   * Filters may mutate or delay the headers.
   *
   * For the response to continue, each filter MUST call `callback.onHeaders()` at some point.
   * Alternatively, filters may end the stream by calling one or more response callbacks.
   *
   * @param headers    The inbound headers.
   * @param statusCode The HTTP status code.
   * @param endStream  Whether this response is headers-only. If true, no body should be expected.
   * @return The header status.
   */
  fun onResponseHeaders(headers: Map<String, List<String>>, statusCode: Int, endStream: Boolean): HeaderStatus

  /**
   * Called any number of times whenever body data is received.
   *
   * Filters may mutate or buffer (defer and concatenate) the data.
   *
   * For the response to continue, each filter MUST call `callback.onResponseData()` at some point.
   * Alternatively, filters may end the stream by calling one or more response callbacks.
   *
   * @param body      The inbound body data chunk.
   * @param endStream Whether this represents the end of a stream/request. If false, the stream will
   *                        continue to remain open.
   * @return The data status.
   */
  fun onResponseData(body: ByteBuffer, endStream: Boolean): DataStatus

  /**
   * Called at most once when the request is closed from the server with trailers.
   *
   * Filters may mutate or delay the trailers.
   *
   * For the response to continue, each filter MUST call `callback.onTrailers()` at some point.
   * Alternatively, filters may call one or more other response callbacks instead.
   *
   * @param trailers The inbound trailers.
   * @return The trailer status.
   */
  fun onResponseTrailers(trailers: Map<String, List<String>>): TrailerStatus

  /**
   * Called at most once when an error within Envoy occurs.
   *
   * For the response to continue, each filter MUST call `callback.onError()` at some point.
   * Alternatively, filters may call one or more other response callbacks instead.
   *
   * @param error The error that occurred within Envoy.
   */
  fun onError(error: EnvoyError)

  /**
   * Called at most once when the client cancels a request.
   *
   * For the response to continue, each filter MUST call `callback.onCancel()` at some point.
   * Alternatively, filters may call one or more other response callbacks instead.
   */
  fun onCanceled()
}