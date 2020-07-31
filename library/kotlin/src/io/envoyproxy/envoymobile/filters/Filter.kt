package io.envoyproxy.envoymobile

import io.envoyproxy.envoymobile.engine.types.EnvoyHTTPFilter
import io.envoyproxy.envoymobile.engine.types.EnvoyHTTPFilterFactory

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
        is FilterHeadersStatus.Continue -> arrayOf(0 /*EnvoyHTTPHeadersStatusContinue*/, result.headers.headers)
        is FilterHeadersStatus.StopIteration -> arrayOf(1 /*EnvoyHTTPHeadersStatusStopIteration*/, result.headers.headers)
      }
    }
    return arrayOf(0, headers)
  }

  override fun onResponseHeaders(headers: Map<String, List<String>>, endStream: Boolean): Array<Any> {
    (filter as? ResponseFilter)?.let { responseFilter ->
      val result = responseFilter.onResponseHeaders(ResponseHeaders(headers), endStream)
      return when (result) {
        is FilterHeadersStatus.Continue -> arrayOf(0 /*EnvoyHTTPHeadersStatusContinue*/, result.headers.headers)
        is FilterHeadersStatus.StopIteration -> arrayOf(1 /*EnvoyHTTPHeadersStatusStopIteration*/, result.headers.headers)
      }
    }
    return arrayOf(0, headers)
  }
}
