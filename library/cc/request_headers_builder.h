#pragma once

#include <string>

#include "headers_builder.h"
#include "request_headers.h"
#include "request_method.h"
#include "retry_policy.h"
#include "upstream_http_protocol.h"

class RequestHeaders;

class RequestHeadersBuilder : public HeadersBuilder {
public:
  RequestHeadersBuilder(RequestMethod::_RequestMethod request_method, const std::string& scheme,
                        const std::string& authority, const std::string& path);

  using HeadersBuilder::add;
  using HeadersBuilder::remove;
  using HeadersBuilder::set;

  RequestHeadersBuilder& add_retry_policy(const RetryPolicy& retry_policy);
  RequestHeadersBuilder&
  add_upstream_http_protocol(UpstreamHttpProtocol::_UpstreamHttpProtocol upstream_http_protocol);

  RequestHeaders build() const;

private:
  using HeadersBuilder::internal_set;
};
