package io.envoyproxy.envoymobile

import java.net.URL


/**
 * Builder used for constructing instances of `Request` types.
 *
 * @param url URL for the request.
 * @param method Method for the request.
 */
class RequestBuilder(
    val url: URL,
    val method: RequestMethod
) {
  // Headers to send with the request.
  // Multiple values for a given name are valid, and will be sent as comma-separated values.
  private val headers: MutableMap<String, MutableList<String>> = mutableMapOf()

  // Trailers to send with the request.
  // Multiple values for a given name are valid, and will be sent as comma-separated values.
  private val trailers: MutableMap<String, MutableList<String>> = mutableMapOf()

  // Serialized data to send as the body of the request.
  private var body: ByteArray? = null

  // Retry policy to use for this request.
  private var retryPolicy: RetryPolicy? = null

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