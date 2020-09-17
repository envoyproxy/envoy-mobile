package io.envoyproxy.envoymobile

/**
 * A time series counter.
 */
interface Counter {
  /**
   * Increment the counter by the given count.
   */
  // TODO: return A status indicating if the action was successful.
  fun increment(count: Int = 1)
}
