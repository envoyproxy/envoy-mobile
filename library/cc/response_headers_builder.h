#pragma once

#include "headers_builder.h"
#include "response_headers.h"

namespace Envoy {
namespace Platform {

class ResponseHeaders;

class ResponseHeadersBuilder : public HeadersBuilder {
public:
  ResponseHeadersBuilder() {}

  ResponseHeadersBuilder& add_http_status(int status);
  ResponseHeaders build() const;
};

using ResponseHeadersBuilderSharedPtr = std::shared_ptr<ResponseHeadersBuilder>;

} // namespace Platform
} // namespace Envoy
