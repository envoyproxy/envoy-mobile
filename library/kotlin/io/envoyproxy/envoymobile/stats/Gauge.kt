package io.envoyproxy.envoymobile

/** A time series gauge. */
interface Gauge : Stats {

  /** Sets the gauge by the given value. */
  fun set(value: Int)

  /** Adds the given amount to the gauge. */
  fun add(amount: Int)

  /** Subtracts the given amount from the gauge. */
  fun sub(amount: Int)

  /** Gets a gauge with the list of tags attached. */
  fun attach(tags: List<Tag>): Gauge
}
