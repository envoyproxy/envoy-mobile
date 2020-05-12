package io.envoyproxy.envoymobile
import java.lang.IllegalArgumentException
/**
 * Represents an HTTP request method.
 */
enum class RequestMethod {
  DELETE,
  GET,
  HEAD,
  OPTIONS,
  PATCH,
  POST,
  PUT,
  TRACE;

  fun stringValue(): String {
    return when (this) {
      RequestMethod.DELETE -> "DELETE"
      RequestMethod.GET -> "GET"
      RequestMethod.HEAD -> "HEAD"
      RequestMethod.OPTIONS -> "OPTIONS"
      RequestMethod.PATCH -> "PATCH"
      RequestMethod.POST -> "POST"
      RequestMethod.PUT -> "PUT"
      RequestMethod.TRACE -> "TRACE"
    }
  }

  companion object {
    fun enumValue(stringRepresentation: String): RequestMethod {
      return when (stringRepresentation) {
        "DELETE" -> RequestMethod.DELETE
        "GET" -> RequestMethod.GET
        "HEAD" -> RequestMethod.HEAD
        "OPTIONS" -> RequestMethod.OPTIONS
        "PATCH" -> RequestMethod.PATCH
        "POST" -> RequestMethod.POST
        "PUT" -> RequestMethod.PUT
        "TRACE" -> RequestMethod.TRACE
        else -> throw IllegalArgumentException("Unable to find value for $stringRepresentation")
      }
    }
  }
}
