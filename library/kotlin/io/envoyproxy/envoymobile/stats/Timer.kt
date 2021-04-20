package io.envoyproxy.envoymobile

/** A time-series distribution of duration measurements. */
interface Timer : Stats {

  /** Records a new duration to add to the timer. */
  fun completeWithDuration(durationMs: Int)

  /** Gets a timer with the list of tags attached. */
  fun attach(tags: List<Tag>): Timer
}
