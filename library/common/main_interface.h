#pragma once
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

// NOLINT(namespace-envoy)

/**
 * Handle to an Envoy engine instance. Valid only for the lifetime of the engine and not intended
 * for any external interpretation or use.
 */
typedef int envoy_engine_t;

/**
 * Handle to an outstanding Envoy HTTP request. Valid only for the duration of the request and not
 * intended for any external interpretation or use.
 */
typedef int envoy_request_t;

/**
 * Handle to an outstanding Envoy HTTP stream. Valid only for the duration of the stream and not
 * intended for any external interpretation or use.
 */
typedef int envoy_stream_t;

/**
 * Result codes returned by all calls made to this interface.
 */
typedef enum { ENVOY_SUCCESS, ENVOY_FAILURE } envoy_status_t;

/**
 * Error code associated with terminal status of a HTTP stream or request.
 */
typedef enum {} envoy_error_code_t;

/**
 * Holds data about an HTTP request.
 */
typedef struct {
  // Status of the Envoy HTTP request. Note that the request might have failed inline.
  // Thus the status should be checked before pursuing other operations on the request.
  envoy_status_t status;
  // Handle to the Envoy HTTP request.
  envoy_request_t request;
} envoy_request;

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
  char* name;
  // Multiple header values for the same header name are supported via a comma-delimited string.
  char* value;
} envoy_header;

/**
 * Holds an HTTP header map as an array of envoy_header structs.
 */
typedef struct {
  // Size of the array.
  size_t length;
  // Array of headers.
  envoy_header* headers;
} envoy_headers;

/**
 * Holds raw binary data as an array of bytes.
 */
typedef struct {
  size_t length;
  uint8_t* bytes;
} envoy_data;

// Convenience constant to pass to function calls with no data.
// For example when sending a headers-only request.
const envoy_data envoy_nodata = {0, NULL};

/**
 * Error struct.
 */
typedef struct {
  envoy_error_code_t error_code;
  size_t length;
  char* message;
} envoy_error;

#ifdef __cplusplus
extern "C" { // function pointers
#endif
/**
 * Called when all headers get received on the async HTTP stream.
 * @param headers, the headers received.
 * @param end_stream, whether the response is headers-only.
 */
typedef void (*on_headers)(envoy_headers headers, bool end_stream);
/**
 * Called when a data frame gets received on the async HTTP stream.
 * This callback can be invoked multiple times if the data gets streamed.
 * @param data, the data received.
 * @param end_stream, whether the data is the last data frame.
 */
typedef void (*on_data)(envoy_data data, bool end_stream);
/**
 * Called when all trailers get received on the async HTTP stream.
 * Note that end stream is implied when on_trailers is called.
 * @param trailers, the trailers received.
 */
typedef void (*on_trailers)(envoy_headers headers);
/**
 * Called when the async HTTP stream has an error.
 * @return envoy_error, the error received/caused by the async HTTP stream.
 */
typedef envoy_error (*on_error)();

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

#ifdef __cplusplus
extern "C" { // functions
#endif

/**
 * Submit a request.
 */
envoy_request send_request(envoy_headers headers, envoy_data data, envoy_observer observer);

/**
 * Cancel a request.
 */
envoy_status_t cancel_request(envoy_request_t request);

/**
 * Open an underlying HTTP stream.
 */
envoy_stream start_stream(envoy_headers headers, envoy_data data, envoy_observer observer);

/**
 * Send headers over an open HTTP stream.
 */
envoy_status_t send_headers(envoy_stream_t stream, envoy_headers header, bool end_stream);

/**
 * Send metadata over an HTTP stream.
 */
envoy_status_t send_metadata(envoy_stream_t stream, envoy_headers metadata, bool end_stream);

/**
 * Send trailers over an open HTTP stream.
 */
envoy_status_t send_trailers(envoy_stream_t stream, envoy_headers trailers, bool end_stream);

/**
 * Half-close an HTTP stream. The stream will be observable and may return further data
 * via the observer callbacks. However, nothing further may be sent.
 */
envoy_status_t close_stream(envoy_stream_t stream);

/**
 * Detach all observers from a stream and send an interrupt upstream if supported by transport.
 */
envoy_status_t evict_stream(envoy_stream_t stream);

/**
 * External entrypoint for library.
 */
envoy_status_t run_engine(const char* config, const char* log_level);

#ifdef __cplusplus
} // functions
#endif
