package io.envoyproxy.envoymobile

/**
 * The engine for downstream clients to integrate Envoy Mobile
 */
interface Engine {

  /**
   *  @return a {@link StreamClient} instance
   */
  fun getStreamClient(): StreamClient

  /**
   *  @return a {@link StatsClient} instance
   */
  fun getStatsClient(): StatsClient
}
