#pragma once

#include <string>

namespace Envoy {
namespace Platform {

enum UpstreamHttpProtocol {
  HTTP1,
  HTTP2,
};

std::string upstreamHttpProtocolToString(UpstreamHttpProtocol method);
UpstreamHttpProtocol upstreamHttpProtocolFromString(std::string str);

} // namespace Platform
} // namespace Envoy
