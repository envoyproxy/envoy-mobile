package io.envoyproxy.envoymobile

import io.envoyproxy.envoymobile.engine.EnvoyEngine
import io.envoyproxy.envoymobile.engine.types.EnvoyObserver
import java.nio.ByteBuffer

/**
 * Available logging levels for an Envoy instance. Note some levels may be compiled out.
 */
enum class LogLevel(internal val level: String) {
  TRACE("trace"),
  DEBUG("debug"),
  INFO("info"),
  WARN("warn"),
  ERROR("error"),
  CRITICAL("critical"),
  OFF("off");
}

/**
 * Wrapper class that allows for easy calling of Envoy's JNI interface in native Java.
 */
class Envoy constructor(
    private val engine: EnvoyEngine,
    internal val config: String,
    internal val logLevel: LogLevel = LogLevel.INFO
) : Client {

  constructor(engine: EnvoyEngine, config: String) : this(engine, config, LogLevel.INFO)

  // Dedicated thread for running this instance of Envoy.
  private val runner: Thread = Thread(Runnable {
    engine.runWithConfig(config.trim(), logLevel.level)
  })

  /**
   * Create a new Envoy instance. The Envoy runner Thread is started as part of instance
   * initialization with the configuration provided. If the Envoy native library and its
   * dependencies haven't been loaded and initialized yet, this will happen lazily when
   * the first instance is created.
   */
  init {
    runner.start()
  }

  /**
   * Returns whether the Envoy instance is currently active and running.
   */
  fun isRunning(): Boolean {
    val state = runner.state
    return state != Thread.State.NEW && state != Thread.State.TERMINATED
  }

  /**
   * Returns whether the Envoy instance is terminated.
   */
  fun isTerminated(): Boolean {
    return runner.state == Thread.State.TERMINATED
  }

  override fun send(request: Request, responseHandler: ResponseHandler): StreamEmitter {
    val stream = engine.startStream(responseHandler.underlyingObserver)
    stream.sendHeaders(request.headers, false)
    return EnvoyStreamEmitter(stream)
  }

  override fun send(request: Request, data: ByteBuffer?, trailers: Map<String, List<String>>, responseHandler: ResponseHandler): CancelableStream {
    val stream = engine.startStream(responseHandler.underlyingObserver)
    stream.sendHeaders(request.headers, false)
    stream.sendData(data, false)
    stream.sendTrailers(trailers)
    return EnvoyStreamEmitter(stream)
  }

  override fun send(request: Request, body: ByteBuffer?, responseHandler: ResponseHandler): CancelableStream {
    return send(request, body, emptyMap(), responseHandler)
  }
}

// For some reason we have an issue building Envoy.kt when the below interfaces/classes are not in this file
// Issue tracked:

interface Client {
  /**
   * For starting a stream.
   *
   * @param request the request for opening a stream.
   * @param responseHandler the callback for receiving stream events.
   * @return the emitter for streaming data outward.
   */
  fun send(request: Request, responseHandler: ResponseHandler): StreamEmitter


  /**
   * Convenience function for sending a unary request.
   *
   * @param request  The request to send.
   * @param data Serialized data to send as the body of the request.
   * @param trailers Trailers to send with the request.
   * @param responseHandler the callback for receiving stream events.
   * @return CancelableStream, a cancelable request.
   */
  fun send(request: Request, data: ByteBuffer?, trailers: Map<String, List<String>>,
           responseHandler: ResponseHandler): CancelableStream

  /**
   * Convenience function for sending a unary request.
   *
   * @param request The request to send.
   * @param body Serialized data to send as the body of the request.
   * @param responseHandler the callback for receiving stream events.
   * @return CancelableStream, a cancelable request.
   */
  fun send(request: Request, body: ByteBuffer?,
           responseHandler: ResponseHandler): CancelableStream
}


/**
 * Callback interface for receiving stream events.
 */
class ResponseHandler {

  class EnvoyObserverAdapter(
      internal val responseHandler: ResponseHandler
  ) : EnvoyObserver {

    override fun onHeaders(headers: Map<String, List<String>>?, endStream: Boolean) {
      val statusCode = headers!![":status"]?.first()?.toIntOrNull() ?: 0
      responseHandler.onHeadersClosure(headers, statusCode)
    }

    override fun onData(byteBuffer: ByteBuffer?, endStream: Boolean) {
      responseHandler.onDataClosure(byteBuffer, endStream)
    }

    override fun onMetadata(metadata: Map<String, List<String>>?) {
      responseHandler.onMetadataClosure(metadata!!)
    }

    override fun onTrailers(trailers: Map<String, List<String>>?) {
      responseHandler.onTrailersClosure(trailers!!)
    }

    override fun onError() {
      responseHandler.onErrorClosure()
    }

    override fun onCancel() {
      responseHandler.onCancelClosure()
    }
  }

  internal val underlyingObserver = EnvoyObserverAdapter(this)

  private var onHeadersClosure: (headers: Map<String, List<String>>, statusCode: Int) -> Unit = { _, _ -> Unit }
  private var onDataClosure: (byteBuffer: ByteBuffer?, endStream: Boolean) -> Unit = { _, _ -> Unit }
  private var onMetadataClosure: (metadata: Map<String, List<String>>) -> Unit = { Unit }
  private var onTrailersClosure: (trailers: Map<String, List<String>>) -> Unit = { Unit }
  private var onErrorClosure: () -> Unit = { Unit }
  private var onCancelClosure: () -> Unit = { Unit }

  /**
   * Specify a callback for when response headers are received by the stream.
   * If `endStream` is `true`, the stream is complete.
   *
   * @param closure: Closure which will receive the headers, status code,
   *                 and flag indicating if the stream is headers-only.
   * @param statusCode the status code of the response.
   * @return ResponseHandler, this ResponseHandler.
   */
  fun onHeaders(closure: (headers: Map<String, List<String>>, statusCode: Int) -> Unit): ResponseHandler {
    this.onHeadersClosure = closure
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
  fun onData(closure: (byteBuffer: ByteBuffer?, endStream: Boolean) -> Unit): ResponseHandler {
    this.onDataClosure = closure
    return this
  }

  /**
   * Called when response metadata is received by the stream.
   *
   * @param metadata the metadata of a response.
   * @param endStream true if the stream is complete.
   * @return ResponseHandler, this ResponseHandler.
   */
  fun onMetadata(closure: (metadata: Map<String, List<String>>) -> Unit): ResponseHandler {
    this.onMetadataClosure = closure
    return this
  }

  /**
   * Specify a callback for when trailers are received by the stream.
   * If the closure is called, the stream is complete.
   *
   * @param closure: Closure which will receive the trailers.
   * @return ResponseHandler, this ResponseHandler.
   */
  fun onTrailers(closure: (trailers: Map<String, List<String>>) -> Unit): ResponseHandler {
    this.onTrailersClosure = closure
    return this
  }

  /**
   * Specify a callback for when an internal Envoy exception occurs with the stream.
   * If the closure is called, the stream is complete.
   *
   * @param closure: Closure which will be called when an error occurs.
   * @return ResponseHandler, this ResponseHandler.
   */
  fun onError(closure: () -> Unit): ResponseHandler {
    this.onErrorClosure = closure
    return this
  }

  /**
   * Specify a callback for when the stream is canceled.
   * If the closure is called, the stream is complete.
   *
   * @param closure: Closure which will be called when the stream is canceled.
   * @return ResponseHandler, this ResponseHandler.
   */
  fun onCanceled(closure: () -> Unit): ResponseHandler {
    this.onCancelClosure = closure
    return this
  }
}


/**
 * Interface for a stream that may be canceled.
 */
interface CancelableStream {
  /**
   * Cancel and end the associated stream.
   * @throws EnvoyException when there is an exception canceling the stream or sending trailers.
   */
  @Throws(EnvoyException::class)
  fun cancel()
}

/**
 * Interface allowing for sending/emitting data on an Envoy stream.
 */
interface StreamEmitter : CancelableStream {

  /**
   * For sending data to an associated stream.
   *
   * @param byteBuffer the byte buffer data to send to the stream.
   * @throws IllegalStateException when the stream is not active
   * @throws EnvoyException when there is an exception sending data.
   * @return this stream emitter.
   */
  @Throws(EnvoyException::class)
  fun sendData(byteBuffer: ByteBuffer): StreamEmitter

  /**
   * For sending a map of metadata to an associated stream.
   *
   * @param metadata the metadata to send over the stream.
   * @throws IllegalStateException when the stream is not active.
   * @throws EnvoyException when there is an exception sending metadata.
   * @return this stream emitter.
   */
  @Throws(EnvoyException::class)
  fun sendMetadata(metadata: Map<String, List<String>>): StreamEmitter

  /**
   * For ending an associated stream and sending trailers.
   *
   * @param trailers to send with ending a stream. If no trailers are needed, empty map will be the default.
   * @throws IllegalStateException when the stream is not active.
   * @throws EnvoyException when there is an exception ending the stream or sending trailers.
   */
  @Throws(EnvoyException::class)
  fun close(trailers: Map<String, List<String>> = emptyMap())
}
