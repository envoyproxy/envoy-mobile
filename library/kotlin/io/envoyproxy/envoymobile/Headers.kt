package io.envoyproxy.envoymobile

/*
 * Base class that is used to represent header/trailer data structures.
 * To instantiate new instances, see `{Request|Response}HeadersBuilder`.
 */
open class Headers {
  @Suppress("MemberNameEqualsClassName")
  val container: HeadersContainer

  /**
   * Internal constructor used by builders.
   *
   * @param headers: Headers to set.
   */
  internal constructor(container: HeadersContainer) {
    this.container = container
  }

  /**
   * Get the value for the provided header name.
   *
   * @param name: Header name for which to get the current value.
   *
   * @return List<String>?, The current headers specified for the provided name.
   */
  fun value(name: String): List<String>? {
    return container.value(name)
  }

  /**
   * Accessor for all underlying headers as a map.
   *
   * @return Map<String, List<String>>, The underlying headers.
   */
  fun caseSensitiveHeaders(): Map<String, List<String>> {
    return container.allHeaders()
  }
}
