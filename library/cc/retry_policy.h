#pragma once

#include <optional>
#include <vector>

#include "absl/types/optional.h"
#include "headers.h"
#include "request_headers.h"

namespace Envoy {
namespace Platform {

class RequestHeaders;

enum RetryRule {
  Status5xx,
  GatewayError,
  ConnectFailure,
  RefusedStream,
  Retriable4xx,
  RetriableHeaders,
  Reset,
};

std::string retryRuleToString(RetryRule retry_rule);
RetryRule retryRuleFromString(const std::string& str);

struct RetryPolicy {
  int max_retry_count_;
  std::vector<RetryRule> retry_on_;
  std::vector<int> retry_status_codes_;
  absl::optional<int> per_try_timeout_ms_;
  absl::optional<int> total_upstream_timeout_ms_;

  RawHeaderMap asRawHeaderMap() const;
  static RetryPolicy fromRawHeaderMap(const RawHeaderMap& headers);
};

using RetryPolicySharedPtr = std::shared_ptr<RetryPolicy>;

} // namespace Platform
} // namespace Envoy
