#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

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

/**
 * Common abstraction for strings.
 */
typedef struct {
  uint64_t length;
  char* data;
} envoy_string;

/**
 * Holds raw binary data as an array of bytes.
 */
typedef struct {
  uint64_t length;
  uint8_t* bytes;
} envoy_data;

/**
 * Holds data about an HTTP stream.
 */
typedef struct {
  // Status of the Envoy HTTP stream. Note that the stream might have failed inline.
  // Thus the status should be checked before pursuing other operations on the stream.
  envoy_status_t status;
  // Handle to the Envoy HTTP Stream.
  envoy_stream_t stream;
} envoy_stream;

/**
 * Holds a single name/value header.
 */
typedef struct {
  envoy_string name;
  // Multiple header values for the same header name are supported via a comma-delimited string.
  envoy_string value;
} envoy_header;

/**
 * Holds an HTTP header map as an array of envoy_header structs.
 */
typedef struct {
  // Number of header elements in the array.
  uint64_t length;
  // Array of headers.
  envoy_header* headers;
} envoy_headers;

// Convenience constant to pass to function calls with no data.
// For example when sending a headers-only request.
const envoy_data envoy_nodata = {0, NULL};

/**
 * Error struct.
 */
typedef struct {
  envoy_error_code_t error_code;
  envoy_string message;
} envoy_error;

#ifdef __cplusplus
extern "C" { // function pointers
#endif
/**
 * Called when all headers get received on the async HTTP stream.
 * @param headers, the headers received.
 * @param end_stream, whether the response is headers-only.
 */
typedef void (*on_headers)(envoy_stream_t stream, envoy_headers headers, bool end_stream);
/**
 * Called when a data frame gets received on the async HTTP stream.
 * This callback can be invoked multiple times if the data gets streamed.
 * @param data, the data received.
 * @param end_stream, whether the data is the last data frame.
 */
typedef void (*on_data)(envoy_stream_t stream, envoy_data data, bool end_stream);
/**
 * Called when all trailers get received on the async HTTP stream.
 * Note that end stream is implied when on_trailers is called.
 * @param trailers, the trailers received.
 */
typedef void (*on_trailers)(envoy_stream_t stream, envoy_headers headers);
/**
 * Called when the async HTTP stream has an error.
 * @param envoy_error, the error received/caused by the async HTTP stream.
 */
typedef void (*on_error)(envoy_stream_t stream, envoy_error error);

#ifdef __cplusplus
} // function pointers
#endif

/**
 * Interface that can handle HTTP callbacks.
 */
typedef struct {
  on_headers h;
  on_data d;
  on_trailers t;
  on_error e;
} envoy_observer;
