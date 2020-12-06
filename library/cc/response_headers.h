#pragma once

#include "headers.h"
#include "response_headers_builder.h"

class ResponseHeaders : public Headers {
public:
  ResponseHeaders(RawHeaders headers) : Headers(headers) {}

  int http_status() const;

  ResponseHeadersBuilder to_response_headers_builder();
};
