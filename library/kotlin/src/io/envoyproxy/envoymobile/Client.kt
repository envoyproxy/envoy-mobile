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

  fun onCanceled() // TODO: onInterrupt()?
  fun onCompletion()

}

interface StreamEmitter {
  fun sendHeaders(headers: Map<String, List<String>>): Boolean
  fun sendData(byteBuffer: ByteBuffer, endStream: Boolean): Boolean
  fun sendMetadata(metadata: Map<String, List<String>>, endStream: Boolean): Boolean
  fun sendTrailers(trailers: Map<String, List<String>>): Boolean

  fun close(): Boolean
  fun interrupt(): Boolean
}

class Error(
    val code: Int,
    val message: String,
    val cause: Throwable? = null
)

fun demo(client: Client) {
  val streamEmitter = client.startStream(request(), object : StreamCallback {
    override fun onHeaders(statusCode: Int, headers: Map<String, List<String>>) {
      TODO("not implemented")
    }

    override fun onData(byteBuffer: ByteBuffer, endStream: Boolean) {
      TODO("not implemented")
    }

    override fun onMetadata(metadata: Map<String, List<String>>, endStream: Boolean) {
      TODO("not implemented")
    }

    override fun onTrailers(trailers: Map<String, List<String>>) {
      TODO("not implemented")
    }

    override fun onError(error: Error) {
      TODO("not implemented")
    }

    override fun onCanceled() {
      TODO("not implemented")
    }

    override fun onCompletion() {
      TODO("not implemented")
    }
  })


}

fun request(): Request {
  TODO("lazily done for demo purposes")
}