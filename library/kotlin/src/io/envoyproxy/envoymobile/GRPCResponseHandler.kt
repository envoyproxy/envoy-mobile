package io.envoyproxy.envoymobile

import io.envoyproxy.envoymobile.engine.types.EnvoyError
import io.envoyproxy.envoymobile.engine.types.EnvoyErrorCode
import java.io.ByteArrayOutputStream
import java.nio.ByteBuffer
import java.util.concurrent.Executor


class GRPCResponseHandler(
    val executor: Executor
) {

  internal val underlyingHandler: ResponseHandler = ResponseHandler(executor)

  private var errorClosure: (error: EnvoyError) -> Unit = { }
  /**
   * Specify a callback for when response headers are received by the stream.
   * If `endStream` is `true`, the stream is complete.
   *
   * @param closure: Closure which will receive the headers, status code,
   *                 and flag indicating if the stream is complete.
   * @return ResponseHandler, this ResponseHandler.
   */
  fun onHeaders(closure: (headers: Map<String, List<String>>, statusCode: Int, endStream: Boolean) -> Unit): GRPCResponseHandler {
    underlyingHandler.onHeaders { headers, _, endStream ->
      val grpcStatus = headers["grpc-status"]?.first()?.toIntOrNull() ?: 0
      closure(headers, grpcStatus, endStream)
    }
    return this
  }

  /**
   * Specify a callback for when a data frame is received by the stream.
   * If `endStream` is `true`, the stream is complete.
   *
   * @param closure: Closure which will receive the data,
   *                 and flag indicating if the stream is complete.
   * @return ResponseHandler, this ResponseHandler.
   */
  fun onMessage(closure: (byteBuffer: ByteBuffer) -> Unit): GRPCResponseHandler {

    val byteBufferedOutputStream = ByteArrayOutputStream()
    var messageLength = -1
    underlyingHandler.onData { byteBuffer, _ ->
      val byteBufferArray = if (byteBuffer.hasArray()) {
        byteBuffer.array()
      } else {
        val array = ByteArray(byteBuffer.remaining())
        byteBuffer.get(array)
        array
      }

      byteBufferedOutputStream.write(byteBufferArray)
      val array = byteBufferedOutputStream.toByteArray()

      // We have a new message to be streamed through
      if (messageLength == -1) {
        if (array.size < MESSAGE_HEADING_OFFSET) {
          // We don't have enough information so we'll just return
          return@onData
        }

        val compressionFlag = array[0]
        // TODO: Support gRPC compression https://github.com/lyft/envoy-mobile/issues/501
        if (compressionFlag.compareTo(0) != 0) {
          errorClosure(
              EnvoyError(
                  EnvoyErrorCode.ENVOY_UNDEFINED_ERROR,
                  "Unable to accept compression enabled messages for gRPC"))

          // no op the current onData and clean up
          underlyingHandler.onData { _, _ -> }
          byteBufferedOutputStream.reset()
        }

        messageLength = ByteBuffer.wrap(array.sliceArray(1..4)).int

        if (messageLength == 0) {
          // No message return empty array
          messageLength = -1
          closure(ByteBuffer.wrap(ByteArray(0)))
        }

        val byteArray = byteBufferedOutputStream.toByteArray()
        val slicedArray: ByteArray = byteArray.slice(MESSAGE_HEADING_OFFSET until byteArray.size).toByteArray()
        byteBufferedOutputStream.reset()
        byteBufferedOutputStream.write(slicedArray)
      }

      if (array.size < messageLength) {
        // Current buffered data doesn't contain the whole message
        // return and away for the next data call
        return@onData
      } else {
        // Current buffered data has enough bytes
        val byteArray = byteBufferedOutputStream.toByteArray()
        val messageByteArray = byteArray.slice(0 until messageLength).toByteArray()
        closure(ByteBuffer.wrap(messageByteArray))

        byteBufferedOutputStream.reset()

        // Add remaining bytes back to the output stream
        if (byteArray.size > messageLength) {
          val remainingByteArray = byteArray.slice(messageLength until byteArray.size).toByteArray()
          byteBufferedOutputStream.write(remainingByteArray)
        }

        // Reset
        messageLength = -1
      }
    }

    return this
  }

  /**
   * Called when response metadata is received by the stream.
   *
   * @param metadata the metadata of a response.
   * @param endStream true if the stream is complete.
   * @return ResponseHandler, this ResponseHandler.
   */
  fun onMetadata(closure: (metadata: Map<String, List<String>>) -> Unit): GRPCResponseHandler {
    underlyingHandler.onMetadata(closure)
    return this
  }

  /**
   * Specify a callback for when trailers are received by the stream.
   * If the closure is called, the stream is complete.
   *
   * @param closure: Closure which will receive the trailers.
   * @return ResponseHandler, this ResponseHandler.
   */
  fun onTrailers(closure: (trailers: Map<String, List<String>>) -> Unit): GRPCResponseHandler {
    underlyingHandler.onTrailers(closure)
    return this
  }

  /**
   * Specify a callback for when an internal Envoy exception occurs with the stream.
   * If the closure is called, the stream is complete.
   *
   * @param closure: Closure which will be called when an error occurs.
   * @return ResponseHandler, this ResponseHandler.
   */
  fun onError(closure: (error: EnvoyError) -> Unit): GRPCResponseHandler {
    this.errorClosure = closure
    underlyingHandler.onError(closure)
    return this
  }

  /**
   * Specify a callback for when the stream is canceled.
   * If the closure is called, the stream is complete.
   *
   * @param closure: Closure which will be called when the stream is canceled.
   * @return ResponseHandler, this ResponseHandler.
   */
  fun onCanceled(closure: () -> Unit): GRPCResponseHandler {
    underlyingHandler.onCanceled(closure)
    return this
  }
}
