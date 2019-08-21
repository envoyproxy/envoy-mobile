package io.envoyproxy.envoymobile

import java.nio.ByteBuffer

interface Client {
  /**
   * For starting a stream.
   *
   * @param request the request for opening a stream.
   * @param responseHandler the callback for receiving stream events.
   * @return the emitter for streaming data outward.
   */
  fun startStream(request: Request, responseHandler: ResponseHandler): StreamEmitter


  /**
   * Convenience function for sending a unary request.
   *
   * @param request  The request to send.
   * @param body Serialized data to send as the body of the request.
   * @param trailers Trailers to send with the request.
   * @param responseHandler the callback for receiving stream events.
   * @return CancellableStream, a cancelable request.
   */
  fun sendUnary(request: Request, body: ByteBuffer?, trailers: Map<String, List<String>>,
                responseHandler: ResponseHandler): CancellableStream

  /**
   * Convenience function for sending a unary request.
   *
   * @param request The request to send.
   * @param body Serialized data to send as the body of the request.
   * @param responseHandler the callback for receiving stream events.
   * @return CancellableStream, a cancelable request.
   */
  fun sendUnary(request: Request, body: ByteBuffer?,
                responseHandler: ResponseHandler): CancellableStream
}
