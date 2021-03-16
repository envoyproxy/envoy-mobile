package io.envoyproxy.envoymobile.engine;

import io.envoyproxy.envoymobile.engine.types.EnvoyStringAccessor;

class JvmStringAccessorContext {
  private final EnvoyStringAccessor accessor;

  public JvmStringAccessorContext(EnvoyStringAccessor accessor) { this.accessor = accessor; }

  /**
   * Invokes getString callback.
   *
   * @return ByteBuffer, the string retrieved from the platform.
   */
  public String getEnvoyString() { return accessor.getEnvoyString(); }
}
