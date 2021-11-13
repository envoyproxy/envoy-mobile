#include "request_headers_builder.h"

namespace Envoy {
namespace Platform {

RequestHeadersBuilder::RequestHeadersBuilder(RequestMethod request_method,
                                             const std::string& scheme,
                                             const std::string& authority,
                                             const std::string& path) {
  internalSet(":method", std::vector<std::string>{requestMethodToString(request_method)});
  internalSet(":scheme", std::vector<std::string>{scheme});
  internalSet(":authority", std::vector<std::string>{authority});
  internalSet(":path", std::vector<std::string>{path});
}

RequestHeadersBuilder& RequestHeadersBuilder::addRetryPolicy(const RetryPolicy& retry_policy) {
  const RawHeaderMap retry_policy_headers = retry_policy.asRawHeaderMap();
  for (const auto& pair : retry_policy_headers) {
    internalSet(pair.first, pair.second);
  }
  return *this;
}

RequestHeadersBuilder&
RequestHeadersBuilder::addUpstreamHttpProtocol(UpstreamHttpProtocol upstream_http_protocol) {
  internalSet("x-envoy-mobile-upstream-protocol",
              std::vector<std::string>{upstreamHttpProtocolToString(upstream_http_protocol)});
  return *this;
}

RequestHeaders RequestHeadersBuilder::build() const { return RequestHeaders(allHeaders()); }

} // namespace Platform
} // namespace Envoy
