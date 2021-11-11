package io.envoyproxy.envoymobile

import io.envoyproxy.envoymobile.engine.types.EnvoyStreamMetrics

/**
 * Exposes internal HTTP stream metrics, context, and other details.
 * TODO(alyssawilk) document
 */
class StreamMetrics constructor(
  val requestStartMs: Long,
  val dnsStartMs: Long,
  val dnsEndMs: Long,
  val connectStartMs: Long,
  val connectEndMs: Long,
  val sslStartMs: Long,
  val sslEndMs: Long,
  val sendingStartMs: Long,
  val sendingEndMs: Long,
  val responseStartMs: Long,
  val requestEndMs: Long,
  val socketReused: Boolean,
  val sentByteCount: Long,
  val receivedByteCount: Long
) {
  constructor(base: EnvoyStreamMetrics) : this(base.requestStartMs, base.dnsStartMs, base.dnsEndMs, base.connectStartMs, base.connectEndMs, base.sslStartMs, base.sslEndMs, base.sendingStartMs, base.sendingEndMs, base.responseStartMs, base.requestEndMs, base.socketReused, base.sentByteCount, base.receivedByteCount)
}
