package io.envoyproxy.envoymobile

/** A time-series distribution of duration measurements. */
interface Timer {

  /** Record a new duration to add to the timer histogram distribution. */
  fun recordDuration(durationMs: Int)
}
