package io.envoyproxy.envoymobile

import java.io.IOException

/**
 * Error type containing information on failures reported by Envoy.
 *
 * @param errorCode internal error code associated with the exception that occurred.
 * @param message a description of what exception that occurred.
 * @param cause an optional cause for the exception.
 */
class EnvoyError(
    val errorCode: Int,
    override val message: String,
    override val cause: Throwable? = null
) : IOException(message, cause)
