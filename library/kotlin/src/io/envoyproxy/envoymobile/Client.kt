package io.envoyproxy.envoymobile

interface Client {
  /**
   * For starting a stream
   *
   * @param request the request for opening a stream
   * @param streamCallback the callback for receiving stream events
   * @return the emitter for streaming data outward
   */
  fun startStream(request: Request, streamCallback: StreamCallback): StreamEmitter
}
