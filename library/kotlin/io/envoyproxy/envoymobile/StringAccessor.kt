package io.envoyproxy.envoymobile

import io.envoyproxy.envoymobile.engine.types.EnvoyStringAccessor
import java.nio.ByteBuffer

/**
 *
 * `StringAccessor` is bridged through to `EnvoyStringAccessor` to communicate with the engine.
 */
class StringAccessor (val getEnvoyString: (() -> ByteBuffer)) {}

/**
 * Class responsible for bridging between the platform-level `StringAccessor` and the
 * engine's `EnvoyStringAccessor`.
 */
internal class EnvoyStringAccessorAdapter(
  private val callbacks: StringAccessor
) : EnvoyStringAccessor {
  override fun getEnvoyString(): ByteBuffer {
    return callbacks.getEnvoyString()
  }
}
