#pragma once

#include <optional>
#include <vector>

#include "headers.h"
#include "request_headers.h"

class RequestHeaders;

enum RetryRule {
  Status5xx,
  GatewayFailure,
  ConnectFailure,
  RefusedStream,
  Retriable4xx,
  RetriableHeaders,
  Reset,
};

struct RetryPolicy {
  int max_retry_count;
  std::vector<RetryRule> retry_on;
  std::vector<int> retry_status_codes;
  std::optional<int> per_try_timeout_ms;
  std::optional<int> total_upstream_timeout_ms;

  RawHeaders output_headers() const;
  static RetryPolicy from(const RequestHeaders& headers);
};
