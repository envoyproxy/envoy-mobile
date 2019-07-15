package io.envoyproxy.envoymobile

class Response internal constructor(
    val body: ByteArray?,
    val headers: Map<String, List<String>>,
    val trailers: Map<String, List<String>>
) {

  override fun equals(other: Any?): Boolean {
    if (this === other) return true
    if (javaClass != other?.javaClass) return false

    other as Response

    if (body != null) {
      if (other.body == null) return false
      if (!body.contentEquals(other.body)) return false
    } else if (other.body != null) return false
    if (headers != other.headers) return false
    if (trailers != other.trailers) return false

    return true
  }

  override fun hashCode(): Int {
    var result = body?.contentHashCode() ?: 0
    result = 31 * result + headers.hashCode()
    result = 31 * result + trailers.hashCode()
    return result
  }
}