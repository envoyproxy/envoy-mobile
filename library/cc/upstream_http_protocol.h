#pragma once

#include <string>

#include "absl/strings/string_view.h"

namespace Envoy {
namespace Platform {

enum UpstreamHttpProtocol {
  HTTP1,
  HTTP2,
};

std::string upstreamHttpProtocolToString(UpstreamHttpProtocol method);
UpstreamHttpProtocol upstreamHttpProtocolFromString(absl::string_view str);

} // namespace Platform
} // namespace Envoy
