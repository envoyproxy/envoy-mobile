#pragma once

// NOLINT(namespace-envoy)

#include "headers.h"
#include "response_headers_builder.h"

class ResponseHeadersBuilder;

class ResponseHeaders : public Headers {
public:
  ResponseHeaders(RawHeaders headers) : Headers(std::move(headers)) {}

  int http_status() const;

  ResponseHeadersBuilder to_response_headers_builder();
};

using ResponseHeadersSharedPtr = std::shared_ptr<ResponseHeaders>;
