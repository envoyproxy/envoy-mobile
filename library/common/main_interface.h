#pragma once
#include <stdbool.h>

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
typedef enum {} envoy_status_t;

/**
 * Error code associated with terminal status of a HTTP stream or request.
 */
typedef enum {} envoy_error_code_t;

typedef struct {
  envoy_status_t status;
  envoy_request_t request;
} envoy_request_pair;

typedef struct {
  envoy_status_t status;
  envoy_stream_t stream;
} envoy_stream_pair;

typedef struct {
  char* name;
  char* value;
} envoy_header;

typedef struct {
  size_t length;
  envoy_header* headers;
} envoy_headers;

typedef struct {
  size_t length;
  uint8_t* bytes;
} envoy_data;

const envoy_data envoy_nodata = {0, NULL};

typedef struct {
  envoy_error_code_t error_code;
  size_t length;
  char* string;
} envoy_error;

#ifdef __cplusplus
extern "C" { // function pointers
#endif

typedef void (*on_headers)(envoy_headers headers, bool end_stream);
typedef void (*on_data)(envoy_data data, bool end_stream);
typedef void (*on_trailers)(envoy_headers headers);
typedef void (*on_error)(envoy_error error);

#ifdef __cplusplus
} // function pointers
#endif

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
envoy_request_pair submit_request(envoy_headers headers, envoy_data data, envoy_observer observer);

/**
 * Cancel a request.
 */
envoy_status_t cancel_request(envoy_request_t request);

/**
 * Open an underlying HTTP stream.
 */
envoy_stream_pair open_stream(envoy_headers headers, envoy_data data, envoy_observer observer);

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
 * Half-close an HTTP stream (stream will be observable and may return further data, but nothing
 * further may be sent).
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
