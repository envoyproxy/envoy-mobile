#pragma once

#include <optional>

#include "headers.h"
#include "request_headers_builder.h"
#include "request_method.h"
#include "retry_policy.h"
#include "upstream_http_protocol.h"


class RequestHeaders : public Headers {
public:
  RequestMethod request_method() const;
  std::string scheme() const;
  std::string authority() const;
  std::string path() const;
  std::optional<RetryPolicy> retry_policy() const;
  UpstreamHttpProtocol::_UpstreamHttpProtocol upstream_http_protocol() const;

  RequestHeadersBuilder to_request_headers_builder() const;

private:
  RequestHeaders(RawHeaders headers) : Headers(headers) {}

  friend class RequestHeadersBuilder;
};
