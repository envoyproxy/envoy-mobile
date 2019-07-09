package io.envoyproxy.envoymobile

data class RetryPolicy(val maxRetryCount: Int, val retryOnCodes: List<Int>, val perRetryTimeoutMs: Long)
