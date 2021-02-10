package io.envoyproxy.envoymobile

/**
 * Client for Envoy Mobile's stats library, Pulse, used to record client time series metrics.
 *
 * Note: this is an experimental interface and is subject to change The implementation has not been
 * optimized, and there may be performance implications in production usage.
 */
interface PulseClient {

  /**
   * @return A counter based on the joined elements.
   */
  fun counter(vararg elements: Element): Counter

  /** @return A gauge based on the joined elements. */
  fun gauge(vararg elements: Element): Gauge

  /** @return A timer based on the joined elements that can track distribution of durations. */
  fun timer(vararg elements: Element): Timer

  /** @return A distribution based on the joined elements tracking the quantile stats of values. */
  fun distribution(vararg elements: Element): Distribution
}
