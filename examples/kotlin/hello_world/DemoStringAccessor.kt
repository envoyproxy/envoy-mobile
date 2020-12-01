package io.envoyproxy.envoymobile.helloenvoykotlin

import io.envoyproxy.envoymobile.engine.types.EnvoyStringAccessor;

import java.nio.ByteBuffer
import java.nio.charset.Charset

class DemoStringAccessor : EnvoyStringAccessor {
  override fun getString(): ByteBuffer {
      return ByteBuffer.wrap("PlatformString".toByteArray(Charsets.UTF_8))
    }
}
