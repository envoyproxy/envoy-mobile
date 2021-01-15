package io.envoyproxy.envoymobile

/** A time-series histogram. */
interface Histogram {

  /** Record a new value to add to the histogram. */
  fun record(value: Int)
}
