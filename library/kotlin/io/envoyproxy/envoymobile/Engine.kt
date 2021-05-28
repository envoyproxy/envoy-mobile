package io.envoyproxy.envoymobile

/**
 * Engine represents a running instance of Envoy Mobile, and provides client interfaces that run on
 * that instance.
 */
interface Engine {

  /**
   *  @return a {@link StreamClient} for opening and managing HTTP streams.
   */
  fun streamClient(): StreamClient

  /**
   *  @return a {@link PulseClient} for recording time series metrics.
   */
  fun pulseClient(): PulseClient

  /**
   * Terminates the running engine.
   */
  fun terminate()

  /**
   * Flush the stats sinks outside of a flushing interval.
   * Note: stats flushing may not be synchronous.
   * Therefore, this function may return prior to flushing taking place.
   */
  fun flushStats()
}
