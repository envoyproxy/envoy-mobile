package io.envoyproxy.envoymobile

internal fun Request.outboundHeaders(): Map<String, String> {
  val validHeaders = headers
    .filter { entry -> !entry.key.startsWith(":") }
    .mapValues { entry -> entry.value.joinToString(separator = ",") }

  val retryPolicyHeaders = retryPolicy?.outboundHeaders() ?: emptyMap()

  val result = mutableMapOf<String, String>()
  result.putAll(validHeaders)
  result.putAll(retryPolicyHeaders)

  return result
}