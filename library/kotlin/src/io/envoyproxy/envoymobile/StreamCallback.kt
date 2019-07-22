package io.envoyproxy.envoymobile

import java.nio.ByteBuffer

interface StreamCallback {
  /**
   * Called when a response headers is received by the stream.
   *
   * @param headers the headers of the response.
   * @param statusCode the status code of the response.
   */
  fun onHeaders(headers: Map<String, List<String>>, statusCode: Int)

  /**
   * Called when a data frame is received by the stream.
   *
   * @param byteBuffer the byte buffer of the response.
   * @param endStream true if the stream is complete.
   */
  fun onData(byteBuffer: ByteBuffer, endStream: Boolean)

  /**
   * Called when a response metadata has been received by the stream.
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
   * Called when an internal Envoy exception occurs with the stream.
   *
   * @param envoyException the exception associated with the stream.
   */
  fun onError(envoyException: EnvoyException)

  /**
   * Called when the stream is cancelled.
   *
   */
  fun onCanceled()

  /**
   * Called when the stream has been completed.
   *
   */
  fun onCompletion()
}
