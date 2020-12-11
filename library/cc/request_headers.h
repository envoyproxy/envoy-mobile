#pragma once

// NOLINT(namespace-envoy)

#include <optional>

#include "absl/types/optional.h"
#include "headers.h"
#include "request_headers_builder.h"
#include "request_method.h"
#include "retry_policy.h"
#include "upstream_http_protocol.h"

class RequestHeaders : public Headers {
public:
  RequestMethod request_method() const;
  const std::string& scheme() const;
  const std::string& authority() const;
  const std::string& path() const;
  const absl::optional<RetryPolicy> retry_policy() const;
  UpstreamHttpProtocol upstream_http_protocol() const;

  RequestHeadersBuilder to_request_headers_builder() const;

private:
  RequestHeaders(RawHeaders headers) : Headers(std::move(headers)) {}

  friend class RequestHeadersBuilder;
};

using RequestHeadersSharedPtr = std::shared_ptr<RequestHeaders>;
