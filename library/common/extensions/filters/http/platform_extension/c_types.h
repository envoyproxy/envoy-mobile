#pragma once

#include "library/common/types/c_types.h"

// NOLINT(namespace-envoy)

/**
 * Return codes for header filter invocations. See envoy/http/filter.h
 */
typedef enum {
  ENVOY_FILTER_HEADERS_STATUS_CONTINUE,
  ENVOY_FILTER_HEADERS_STATUS_STOP_ITERATION,
  ENVOY_FILTER_HEADERS_STATUS_CONTINUE_AND_END_STREAM,
  ENVOY_FILTER_HEADERS_STATUS_STOP_ALL_ITERATION_AND_BUFFER,
} envoy_filter_headers_status_t;

/**
 * Return codes for data filter invocations. See envoy/http/filter.h
 */
typedef enum {
  ENVOY_FILTER_DATA_STATUS_CONTINUE,
  ENVOY_FILTER_DATA_STATUS_STOP_ITERATION_AND_BUFFER,
  ENVOY_FILTER_DATA_STATUS_STOP_ITERATION_NO_BUFFER,
} envoy_filter_data_status_t;

/**
 * Function signature for header filter invocations.
 */
typedef envoy_filter_headers_status_t (*envoy_filter_on_headers_f)(envoy_headers headers,
                                                                   bool end_stream, void* context);

/**
 * Function signature for data filter invocations.
 */
typedef envoy_filter_data_status_t (*envoy_filter_on_data_f)(envoy_data data, bool end_stream,
                                                             void* context);

/**
 * Raw datatype containing dispatch functions for a platform-native HTTP filter. Leveraged by the
 * PlatformHarnessFilter
 */
typedef struct {
  envoy_filter_on_headers_f on_request_headers;
  envoy_filter_on_data_f on_request_data;
  envoy_filter_on_headers_f on_response_headers;
  envoy_filter_on_data_f on_response_data;
  void* context;
} envoy_http_filter;
