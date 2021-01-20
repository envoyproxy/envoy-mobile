package io.envoyproxy.envoymobile

import io.envoyproxy.envoymobile.engine.types.HistogramUnit

/** A time-series histogram. */
interface Histogram {

  /** Record a new value to add to the histogram distribution. */
  fun recordValue(value: Int)
}
