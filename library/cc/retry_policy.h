#pragma once

#include "headers.h"
#include "request_headers.h"


class RequestHeaders;

class RetryRule {
public:
  enum _RetryRule {
    Status5xx,
    GatewayFailure,
    ConnectFailure,
    RefusedStream,
    Retriable4xx,
    RetriableHeaders,
    Reset,
  };


  static _RetryRule from_string(std::string retry_rule);
  static std::string to_string(_RetryRule retry_rule);

private:
  RetryRule() {}
};

struct RetryPolicy {
  int max_retry_count;
  int retry_on;
  std::vector<int> retry_status_codes;
  int per_try_timeout_ms;
  int total_upstream_timeout_ms;

  RawHeaders output_headers() const;
  static RetryPolicy from(const RequestHeaders& headers);
};
