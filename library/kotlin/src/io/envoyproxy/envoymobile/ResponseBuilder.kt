package io.envoyproxy.envoymobile

/**
 * Builder used for constructing instances of `Response` types.
 *
 */
class ResponseBuilder {

  // Headers to send with the response.
  // Multiple values for a given name are valid, and will be sent as comma-separated values.
  private val headers: MutableMap<String, MutableList<String>> = mutableMapOf()

  // Trailers to send with the response.
  // Multiple values for a given name are valid, and will be sent as comma-separated values.
  private val trailers: MutableMap<String, MutableList<String>> = mutableMapOf()

  // Serialized data of the response.
  private var body: ByteArray? = null

  private var status: Int = 200

  /**
   * Add an http status code for the response
   *
   * @param status the http status code
   * @return this builder
   */
  fun addStatus(status: Int): ResponseBuilder {
    this.status = status
    return this
  }

  /**
   * Add serialized data of the response.
   *
   * @param body serialized data of the response
   * @return this builder
   */
  fun addBody(body: ByteArray?): ResponseBuilder {
    this.body = body
    return this
  }

  /**
   * Append a value to the header key.
   *
   * @param name the header key.
   * @param value the value associated to the header key.
   * @return this builder.
   */
  fun addHeader(name: String, value: String): ResponseBuilder {
    if (headers.containsKey(name)) {
      headers[name]!!.add(value)
    } else {
      headers[name] = mutableListOf(value)
    }
    return this
  }

  /**
   * Remove the value in the specified header.
   *
   * @param name the header key to remove.
   * @param value the value to be removed.
   * @return this builder.
   */
  fun removeHeader(name: String, value: String): ResponseBuilder {
    if (headers.containsKey(name)) {
      headers[name]!!.remove(value)
      if (headers[name]!!.isEmpty()) {
        headers.remove(name)
      }
    }
    return this
  }

  /**
   * Remove all headers with this name.
   *
   * @param name the header key to remove.
   * @return this builder.
   */
  fun removeHeaders(name: String): ResponseBuilder {
    headers.remove(name)
    return this
  }

  /**
   * Append a value to the trailer key.
   *
   * @param name the trailer key.
   * @param value the value associated to the trailer key.
   * @return this builder.
   */
  fun addTrailer(name: String, value: String): ResponseBuilder {
    if (trailers.containsKey(name)) {
      trailers[name]!!.add(value)
    } else {
      trailers[name] = mutableListOf(value)
    }
    return this
  }

  /**
   * Remove the value in the specified trailer.
   *
   * @param name the trailer key to remove.
   * @param value the value to be removed.
   * @return this builder.
   */
  fun removeTrailers(name: String): ResponseBuilder {
    trailers.remove(name)
    return this
  }

  /**
   * Remove the value in the specified trailer.
   *
   * @param name the trailer key to remove.
   * @param value the value to be removed.
   * @return this builder.
   */
  fun removeTrailer(name: String, value: String): ResponseBuilder {
    if (trailers.containsKey(name)) {
      trailers[name]!!.remove(value)

      if (trailers[name]!!.isEmpty()) {
        trailers.remove(name)
      }
    }
    return this
  }
}