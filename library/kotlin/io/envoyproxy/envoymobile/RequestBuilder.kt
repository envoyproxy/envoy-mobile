package io.envoyproxy.envoymobile

import java.net.URL


class RequestBuilder(
    val method: RequestMethod,
    val url: URL
) {
  private var body: ByteArray? = null
  private var retryPolicy: RetryPolicy? = null

  private val headers: MutableMap<String, MutableList<String>> = mutableMapOf()
  private val trailers: MutableMap<String, MutableList<String>> = mutableMapOf()

  fun addBody(body: ByteArray?): RequestBuilder {
    this.body = body
    return this
  }

  fun addRetryPolicy(retryPolicy: RetryPolicy?): RequestBuilder {
    this.retryPolicy = retryPolicy
    return this
  }

  fun addHeader(header: String, value: String): RequestBuilder {
    if (headers.containsKey(header)) {
      headers[header]!!.add(value)
    }

    headers[header] = mutableListOf(value)
    return this
  }

  fun removeHeader(header: String): RequestBuilder {
    headers.remove(header)
    return this
  }

  fun addTrailer(trailer: String, value: String): RequestBuilder {
    if (trailers.containsKey(value)) {
      trailers[trailer]!!.add(value)
    }

    trailers[trailer] = mutableListOf(value)
    return this
  }

  fun removeTrailer(trailer: String): RequestBuilder {
    trailers.remove(trailer)
    return this
  }

  fun build(): Request {
    return Request(
        method,
        url,
        headers,
        trailers,
        body,
        retryPolicy
    )
  }

  internal fun setHeaders(headers: Map<String, List<String>>): RequestBuilder {
    this.headers.clear()
    headers.map { entry -> this.headers[entry.key] = entry.value.toMutableList() }
    return this
  }

  internal fun setTrailers(trailers: Map<String, List<String>>): RequestBuilder {
    this.trailers.clear()
    trailers.map { entry -> this.trailers[entry.key] = entry.value.toMutableList() }
    return this
  }
}