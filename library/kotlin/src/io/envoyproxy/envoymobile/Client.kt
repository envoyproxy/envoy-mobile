package io.envoyproxy.envoymobile

import java.nio.ByteBuffer

interface Client {
  fun startStream(request: Request, streamCallback: StreamCallback): StreamEmitter
}

interface StreamCallback {
  fun onHeaders(statusCode: Int, headers: Map<String, List<String>>)
  fun onData(byteBuffer: ByteBuffer, endStream: Boolean)
  fun onMetadata(metadata: Map<String, List<String>>, endStream: Boolean)
  fun onTrailers(trailers: Map<String, List<String>>)
  fun onError(error: Error)
  fun onCanceled()
  fun onCompletion()
}

interface StreamEmitter {
  fun isActive(): Boolean
  fun sendData(byteBuffer: ByteBuffer, endStream: Boolean): StreamEmitter
  fun sendMetadata(metadata: Map<String, List<String>>, endStream: Boolean): StreamEmitter
  
  fun close(trailers: Map<String, List<String>>)
  fun cancel()
}

class Error(
    val code: Int,
    val message: String,
    val cause: Throwable? = null
)