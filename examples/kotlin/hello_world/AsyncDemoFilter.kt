package io.envoyproxy.envoymobile.helloenvoykotlin

import io.envoyproxy.envoymobile.AsyncResponseFilter
import io.envoyproxy.envoymobile.EnvoyError
import io.envoyproxy.envoymobile.FilterDataStatus
import io.envoyproxy.envoymobile.FilterHeadersStatus
import io.envoyproxy.envoymobile.FilterResumeStatus
import io.envoyproxy.envoymobile.FilterTrailersStatus
import io.envoyproxy.envoymobile.ResponseFilterCallbacks
import io.envoyproxy.envoymobile.ResponseHeaders
import io.envoyproxy.envoymobile.ResponseTrailers
import java.nio.ByteBuffer

/**
 * Example of a more complex HTTP filter that pauses processing on the response filter chain,
 * buffers until the response is complete, then asynchronously triggers filter chain resumption
 * while setting a new header. Also demonstrates safety of re-entrancy in async callbacks.
 */
class AsyncDemoFilter : AsyncResponseFilter {
  private lateinit var callbacks: ResponseFilterCallbacks

  override fun onResponseHeaders(
    headers: ResponseHeaders,
    endStream: Boolean
  ): FilterHeadersStatus<ResponseHeaders> {
    return FilterHeadersStatus.StopIteration()
  }

  override fun onResponseData(
    body: ByteBuffer,
    endStream: Boolean
  ): FilterDataStatus<ResponseHeaders> {
    // If this is the end of the stream, asynchronously resume response processing via callback.
    if (endStream) {
      callbacks.resumeResponse()
    }
    return FilterDataStatus.StopIterationAndBuffer()
  }

  override fun onResponseTrailers(
    trailers: ResponseTrailers
  ): FilterTrailersStatus<ResponseHeaders, ResponseTrailers> {
    // Trailers imply end of stream, so asynchronously resume response processing via callbacka
    // Note this call is re-entrant (but legal/safe).
    callbacks.resumeResponse()
    return FilterTrailersStatus.StopIteration()
  }

  override fun setResponseFilterCallbacks(callbacks: ResponseFilterCallbacks) {
    this.callbacks = callbacks
  }

  override fun onResumeResponse(
    headers: ResponseHeaders?,
    data: ByteBuffer?,
    trailers: ResponseTrailers?,
    endStream: Boolean
  ): FilterResumeStatus<ResponseHeaders, ResponseTrailers> {
    val builder = headers!!.toResponseHeadersBuilder()
      .add("async-filter-demo", "1")
    return FilterResumeStatus.ResumeIteration(builder.build(), data, trailers)
  }

  @Suppress("EmptyFunctionBlock")
  override fun onError(error: EnvoyError) {
  }

  @Suppress("EmptyFunctionBlock")
  override fun onCancel() {
  }
}
