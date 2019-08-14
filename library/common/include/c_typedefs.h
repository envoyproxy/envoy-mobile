#pragma once

// NOLINT(namespace-envoy)

/**
 * Handle to an Envoy engine instance. Valid only for the lifetime of the engine and not intended
 * for any external interpretation or use.
 */
typedef uint64_t envoy_engine_t;

/**
 * Handle to an outstanding Envoy HTTP stream. Valid only for the duration of the stream and not
 * intended for any external interpretation or use.
 */
typedef uint64_t envoy_stream_t;


/**
 * Result codes returned by all calls made to this interface.
 */
typedef enum { ENVOY_SUCCESS, ENVOY_FAILURE } envoy_status_t;

/**
 * Error code associated with terminal status of a HTTP stream.
 */
typedef enum { ENVOY_STREAM_RESET } envoy_error_code_t;

