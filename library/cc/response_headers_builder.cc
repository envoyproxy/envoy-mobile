#include "response_headers_builder.h"

// NOLINT(namespace-envoy)

ResponseHeadersBuilder& ResponseHeadersBuilder::add_http_status(int status) {
  this->internal_set(":status", std::vector<std::string>{std::to_string(status)});
  return *this;
}

ResponseHeaders ResponseHeadersBuilder::build() const {
  return ResponseHeaders(this->all_headers());
}
