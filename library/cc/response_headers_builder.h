#pragma once

#include "headers_builder.h"
#include "response_headers.h"

class ResponseHeaders;

class ResponseHeadersBuilder : HeadersBuilder {
public:
  ResponseHeadersBuilder() {}

  using HeadersBuilder::add;
  using HeadersBuilder::remove;
  using HeadersBuilder::set;

  ResponseHeadersBuilder& add_http_status(int status);
  ResponseHeaders build() const;

private:
  using HeadersBuilder::internal_set;
};
