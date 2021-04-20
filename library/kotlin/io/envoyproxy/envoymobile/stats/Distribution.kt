package io.envoyproxy.envoymobile

/** A time-series distribution that tracks quantile/sum/average stats. */
interface Distribution : Stats {

  /** Records a new value to add to the distribution. */
  fun recordValue(value: Int)

  /** Gets a distribution with tags attached. */
  fun attach(tags: List<Tag>): Distribution
}
