package io.envoyproxy.envoymobile

/**
 * A time series counter.
 */
interface Counter : Stats {
  /**
   * Increment the counter by the given count.
   */
  fun increment(count: Int = 1)

  /** Gets a counter with tags attached. */
  fun attach(tags: List<Tag>): Counter
}
