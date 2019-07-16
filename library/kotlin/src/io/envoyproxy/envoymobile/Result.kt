package io.envoyproxy.envoymobile

/**
 * The result of a request
 */
sealed class Result {

  /**
   * This method transforms a given response or failure to a specified type which
   * the user can define. Usages of this can be:
   *
   * val result: Result = ...
   *
   * // To throw an exception when a failure occurs
   * result.fold({ response -> response.body }, { failure -> throw Exception(failure.cause) })
   *
   * // To return a default value of 0 when a failure occurs
   * result.fold({ response -> 1 }, { failure -> 0 })
   *
   * @param response a function for returning a generic value based on a response
   * @param failure a function for returning a generic value based on a failure
   *
   * @return a generic type which the user defines
   */
  fun <E> fold(response: (Response) -> E, failure: (NetworkError) -> E): E {
    return when (this) {
      is Response -> response(this)
      is NetworkError -> failure(this)
    }
  }

  /**
   * A side effect function for successful Results
   *
   * @param response a function which is called on a response for a request
   */
  fun response(response: (Response) -> Unit) = fold(response, {})


  /**
   * A side effect function for successful Results
   *
   * @param failure a function which is called on an failure for a request
   */
  fun failure(failure: (NetworkError) -> Unit) = fold({}, failure)
}

/**
 * The failure when we are unable to get a response
 */
class NetworkError(
    val message: String,
    val cause: Throwable?
) : Result()

/**
 * The response from a network call
 */
class Response internal constructor(
    val status: Int,
    val body: ByteArray?,
    val headers: Map<String, List<String>>,
    val trailers: Map<String, List<String>>
) : Result() {

  /**
   * Transforms this Response to the {@link io.envoyproxy.envoymobile.ResponseBuilder} for modification using the
   * current properties
   *
   * @return the builder
   */
  fun toBuilder(): ResponseBuilder {
    return ResponseBuilder()
        .addStatus(status)
        .setHeaders(headers)
        .setTrailers(trailers)
        .addBody(body)
  }

  override fun equals(other: Any?): Boolean {
    if (this === other) return true
    if (javaClass != other?.javaClass) return false

    other as Response

    if (status != other.status) return false
    if (body != null) {
      if (other.body == null) return false
      if (!body.contentEquals(other.body)) return false
    } else if (other.body != null) return false
    if (headers != other.headers) return false
    if (trailers != other.trailers) return false

    return true
  }

  override fun hashCode(): Int {
    var result = status
    result = 31 * result + (body?.contentHashCode() ?: 0)
    result = 31 * result + headers.hashCode()
    result = 31 * result + trailers.hashCode()
    return result
  }

}
