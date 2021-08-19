package io.envoyproxy.envoymobile

import io.envoyproxy.envoymobile.engine.types.EnvoyHTTPFilter
import io.envoyproxy.envoymobile.engine.types.EnvoyHTTPFilterCallbacks
import io.envoyproxy.envoymobile.engine.types.EnvoyHTTPFilterFactory
import io.envoyproxy.envoymobile.engine.types.EnvoyStreamIntel
import java.nio.ByteBuffer

/*
 * Interface representing a filter. See `RequestFilter` and `ResponseFilter` for more details.
 */
@Suppress("EmptyClassBlock")
interface Filter

internal class FilterFactory(
  private val filterName: String,
  private val factory: () -> Filter
) : EnvoyHTTPFilterFactory {
  override fun getFilterName(): String {
    return filterName
  }

  override fun create(): EnvoyHTTPFilter { return EnvoyHTTPFilterAdapter(factory()) }
}

internal class EnvoyHTTPFilterAdapter(
  private val filter: Filter
) : EnvoyHTTPFilter {
  private val nullIntel = StreamIntel(0L, 0L, 0L)

  override fun onRequestHeaders(headers: Map<String, List<String>>, endStream: Boolean, streamIntel: EnvoyStreamIntel): Array<Any?> {
    (filter as? RequestFilter)?.let { requestFilter ->
      val result = requestFilter.onRequestHeaders(RequestHeaders(headers), endStream, nullIntel)
      return when (result) {
        is FilterHeadersStatus.Continue -> arrayOf(result.status, result.headers.headers)
        is FilterHeadersStatus.StopIteration -> arrayOf(result.status, emptyMap<String, List<String>>())
      }
    }
    return arrayOf(0, headers)
  }

  override fun onResponseHeaders(headers: Map<String, List<String>>, endStream: Boolean, streamIntel: EnvoyStreamIntel): Array<Any?> {
    (filter as? ResponseFilter)?.let { responseFilter ->
      val result = responseFilter.onResponseHeaders(ResponseHeaders(headers), endStream, nullIntel)
      return when (result) {
        is FilterHeadersStatus.Continue -> arrayOf(result.status, result.headers.headers)
        is FilterHeadersStatus.StopIteration -> arrayOf(result.status, emptyMap<String, List<String>>())
      }
    }
    return arrayOf(0, headers)
  }

  override fun onRequestData(data: ByteBuffer, endStream: Boolean, streamIntel: EnvoyStreamIntel): Array<Any?> {
    (filter as? RequestFilter)?.let { requestFilter ->
      val result = requestFilter.onRequestData(data, endStream, nullIntel)
      return when (result) {
        is FilterDataStatus.Continue<*> -> arrayOf(result.status, result.data)
        is FilterDataStatus.StopIterationAndBuffer<*> -> arrayOf(result.status, ByteBuffer.allocate(0))
        is FilterDataStatus.StopIterationNoBuffer<*> -> arrayOf(result.status, ByteBuffer.allocate(0))
        is FilterDataStatus.ResumeIteration<*> -> arrayOf(result.status, result.data, result.headers?.headers)
      }
    }
    return arrayOf(0, data)
  }

  override fun onResponseData(data: ByteBuffer, endStream: Boolean, streamIntel: EnvoyStreamIntel): Array<Any?> {
    (filter as? ResponseFilter)?.let { responseFilter ->
      val result = responseFilter.onResponseData(data, endStream, nullIntel)
      return when (result) {
        is FilterDataStatus.Continue<*> -> arrayOf(result.status, result.data)
        is FilterDataStatus.StopIterationAndBuffer<*> -> arrayOf(result.status, ByteBuffer.allocate(0))
        is FilterDataStatus.StopIterationNoBuffer<*> -> arrayOf(result.status, ByteBuffer.allocate(0))
        is FilterDataStatus.ResumeIteration<*> -> arrayOf(result.status, result.data, result.headers?.headers)
      }
    }
    return arrayOf(0, data)
  }

  override fun onRequestTrailers(trailers: Map<String, List<String>>, streamIntel: EnvoyStreamIntel): Array<Any?> {
    (filter as? RequestFilter)?.let { requestFilter ->
      val result = requestFilter.onRequestTrailers(RequestTrailers(trailers), nullIntel)
      return when (result) {
        is FilterTrailersStatus.Continue<*, *> -> arrayOf(result.status, result.trailers.headers)
        is FilterTrailersStatus.StopIteration<*, *> -> arrayOf(result.status, emptyMap<String, List<String>>())
        is FilterTrailersStatus.ResumeIteration<*, *> -> arrayOf(result.status, result.trailers.headers, result.headers?.headers, result.data)
      }
    }
    return arrayOf(0, trailers)
  }

  override fun onResponseTrailers(trailers: Map<String, List<String>>, streamIntel: EnvoyStreamIntel): Array<Any?> {
    (filter as? ResponseFilter)?.let { responseFilter ->
      val result = responseFilter.onResponseTrailers(ResponseTrailers(trailers), nullIntel)
      return when (result) {
        is FilterTrailersStatus.Continue<*, *> -> arrayOf(result.status, result.trailers.headers)
        is FilterTrailersStatus.StopIteration<*, *> -> arrayOf(result.status, emptyMap<String, List<String>>())
        is FilterTrailersStatus.ResumeIteration<*, *> -> arrayOf(result.status, result.trailers.headers, result.headers?.headers, result.data)
      }
    }
    return arrayOf(0, trailers)
  }

  override fun onError(errorCode: Int, message: String, attemptCount: Int, streamIntel: EnvoyStreamIntel) {
    (filter as? ResponseFilter)?.let { responseFilter ->
      responseFilter.onError(EnvoyError(errorCode, message, attemptCount), nullIntel)
    }
  }

  override fun onCancel(streamIntel: EnvoyStreamIntel) {
    (filter as? ResponseFilter)?.let { responseFilter ->
      responseFilter.onCancel(nullIntel)
    }
  }

  override fun setRequestFilterCallbacks(callbacks: EnvoyHTTPFilterCallbacks) {
    (filter as? AsyncRequestFilter)?.let { asyncRequestFilter ->
      asyncRequestFilter.setRequestFilterCallbacks(RequestFilterCallbacksImpl(callbacks))
    }
  }

  override fun onResumeRequest(headers: Map<String, List<String>>?, data: ByteBuffer?, trailers: Map<String, List<String>>?, endStream: Boolean, streamIntel: EnvoyStreamIntel): Array<Any?> {
    (filter as? AsyncRequestFilter)?.let { asyncRequestFilter ->
      val result = asyncRequestFilter.onResumeRequest(
        headers?.let(::RequestHeaders),
        data,
        trailers?.let(::RequestTrailers),
        endStream,
        nullIntel
      )
      return when (result) {
        is FilterResumeStatus.ResumeIteration<*, *> -> arrayOf(result.status, result.headers?.headers, result.data, result.trailers?.headers)
      }
    }
    return arrayOf(-1, headers, data, trailers)
  }

  override fun setResponseFilterCallbacks(callbacks: EnvoyHTTPFilterCallbacks) {
    (filter as? AsyncResponseFilter)?.let { asyncResponseFilter ->
      asyncResponseFilter.setResponseFilterCallbacks(ResponseFilterCallbacksImpl(callbacks))
    }
  }

  override fun onResumeResponse(headers: Map<String, List<String>>?, data: ByteBuffer?, trailers: Map<String, List<String>>?, endStream: Boolean, streamIntel: EnvoyStreamIntel): Array<Any?> {
    (filter as? AsyncResponseFilter)?.let { asyncResponseFilter ->
      val result = asyncResponseFilter.onResumeResponse(
        headers?.let(::ResponseHeaders),
        data,
        trailers?.let(::ResponseTrailers),
        endStream,
        nullIntel
      )
      return when (result) {
        is FilterResumeStatus.ResumeIteration<*, *> -> arrayOf(result.status, result.headers?.headers, result.data, result.trailers?.headers)
      }
    }
    return arrayOf(-1, headers, data, trailers)
  }
}
