package io.envoyproxy.envoymobile

import java.nio.ByteBuffer

interface StreamCallback {
  /**
   * Invoked whenever the response headers have been read.
   *
   * @param headers the headers of the response.
   * @param statusCode the status code of the response.
   */
  fun onHeaders(headers: Map<String, List<String>>, statusCode: Int)

  /**
   * Invoked whenever the response body is read.
   *
   * @param byteBuffer the byte buffer of the response.
   * @param endStream true if the stream is complete.
   */
  fun onData(byteBuffer: ByteBuffer, endStream: Boolean)

  /**
   * Invoked whenever a response metadata has been read.
   *
   * @param metadata the metadata of a response.
   * @param endStream true if the stream is complete.
   */
  fun onMetadata(metadata: Map<String, List<String>>, endStream: Boolean)

  /**
   * Invoked whenever a response trailers have been read.
   *
   * @param trailers the trailers of the response.
   */
  fun onTrailers(trailers: Map<String, List<String>>)

  /**
   * Invoked when there is an internal Envoy exception associated with the stream.
   *
   * @param envoyException the exception associated with the stream.
   */
  fun onError(envoyException: EnvoyException)

  /**
   * Invoked when the stream is cancelled.
   *
   */
  fun onCanceled()

  /**
   * Invoked when the stream has been completed.
   *
   */
  fun onCompletion()
}
