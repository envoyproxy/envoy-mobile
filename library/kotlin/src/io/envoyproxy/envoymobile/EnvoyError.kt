package io.envoyproxy.envoymobile

/**
 * Error type containing information on failures reported by Envoy.
 *
 * @param errorCode internal error code associated with the exception that occurred.
 * @param message a description of what exception that occurred.
 * @param attemptCount the number of times an operation was attempted before firing this error.
 * @param cause an optional cause for the exception.
 */
class EnvoyError internal constructor(
    val errorCode: Int,
    val message: String,
    val attemptCount: Int,
    val cause: Throwable? = null
)
