package io.envoyproxy.envoymobile

import io.envoyproxy.envoymobile.engine.types.EnvoyHTTPFilter
import io.envoyproxy.envoymobile.engine.types.EnvoyHTTPFilterFactory
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
  override fun onRequestHeaders(headers: Map<String, List<String>>, endStream: Boolean): Array<Any> {
    (filter as? RequestFilter)?.let { requestFilter ->
      val result = requestFilter.onRequestHeaders(RequestHeaders(headers), endStream)
      return when (result) {
        is FilterHeadersStatus.Continue -> arrayOf(result.status, result.headers.headers)
        is FilterHeadersStatus.StopIteration -> arrayOf(result.status, headers)
      }
    }
    return arrayOf(0, headers)
  }

  override fun onResponseHeaders(headers: Map<String, List<String>>, endStream: Boolean): Array<Any> {
    (filter as? ResponseFilter)?.let { responseFilter ->
      val result = responseFilter.onResponseHeaders(ResponseHeaders(headers), endStream)
      return when (result) {
        is FilterHeadersStatus.Continue -> arrayOf(result.status, result.headers.headers)
        is FilterHeadersStatus.StopIteration -> arrayOf(result.status, headers)
      }
    }
    return arrayOf(0, headers)
  }

  override fun onRequestData(data: ByteBuffer, endStream: Boolean): Array<Any> {
    (filter as? RequestFilter)?.let { requestFilter ->
      val result = requestFilter.onRequestData(data, endStream)
      return when (result) {
        is FilterDataStatus.Continue<*> -> arrayOf(result.status, result.data)
        is FilterDataStatus.StopIterationAndBuffer<*> -> arrayOf(result.status, data)
        is FilterDataStatus.StopIterationNoBuffer<*> -> arrayOf(result.status, data)
        is FilterDataStatus.ResumeIteration<*> -> arrayOf(result.status, result.data)
      }
    }
    return arrayOf(0, data)
  }

  override fun onResponseData(data: ByteBuffer, endStream: Boolean): Array<Any> {
    (filter as? ResponseFilter)?.let { responseFilter ->
      val result = responseFilter.onResponseData(data, endStream)
      return when (result) {
        is FilterDataStatus.Continue<*> -> arrayOf(result.status, result.data)
        is FilterDataStatus.StopIterationAndBuffer<*> -> arrayOf(result.status, data)
        is FilterDataStatus.StopIterationNoBuffer<*> -> arrayOf(result.status, data)
        is FilterDataStatus.ResumeIteration<*> -> arrayOf(result.status, result.data)
      }
    }
    return arrayOf(0, data)
  }

  override fun onRequestTrailers(trailers: Map<String, List<String>>): Array<Any> {
    (filter as? RequestFilter)?.let { requestFilter ->
      val result = requestFilter.onRequestTrailers(RequestTrailers(trailers))
      return when (result) {
        is FilterTrailersStatus.Continue<*, *> -> arrayOf(result.status, result.trailers.headers)
        is FilterTrailersStatus.StopIteration<*, *> -> arrayOf(result.status, trailers)
        is FilterTrailersStatus.ResumeIteration<*, *> -> arrayOf(result.status, result.trailers!!.headers)
      }
    }
    return arrayOf(0, trailers)
  }

  override fun onResponseTrailers(trailers: Map<String, List<String>>): Array<Any> {
    (filter as? ResponseFilter)?.let { responseFilter ->
      val result = responseFilter.onResponseTrailers(ResponseTrailers(trailers))
      return when (result) {
        is FilterTrailersStatus.Continue<*, *> -> arrayOf(result.status, result.trailers.headers)
        is FilterTrailersStatus.StopIteration<*, *> -> arrayOf(result.status, trailers)
        is FilterTrailersStatus.ResumeIteration<*, *> -> arrayOf(result.status, result.trailers!!.headers)
      }
    }
    return arrayOf(0, trailers)
  }
}
