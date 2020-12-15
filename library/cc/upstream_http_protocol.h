#pragma once

// NOLINT(namespace-envoy)

#include <string>

enum UpstreamHttpProtocol {
  HTTP1,
  HTTP2,
};

std::string upstream_http_protocol_to_string(UpstreamHttpProtocol method);
UpstreamHttpProtocol upstream_http_protocol_from_string(const std::string& str);
