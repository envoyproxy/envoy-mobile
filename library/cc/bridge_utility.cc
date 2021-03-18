#include "bridge_utility.h"

#include <sstream>

#include "library/common/buffer/utility.h"

namespace Envoy {
namespace Platform {

// TODO(crockeo): we always copy memory across boundaries; consider allowing for moves and/or
// shared ownership w/ reference counting via envoy-mobile's release callbacks
envoy_headers raw_header_map_as_envoy_headers(const RawHeaderMap& headers) {
  size_t header_count = 0;
  for (const auto& pair : headers) {
    header_count += pair.second.size();
  }

  envoy_map_entry* headers_list =
      static_cast<envoy_map_entry*>(safe_malloc(sizeof(envoy_map_entry) * header_count));

  size_t i = 0;
  for (const auto& pair : headers) {
    const auto& key = pair.first;
    for (const auto& value : pair.second) {
      envoy_map_entry& header = headers_list[i++];
      header.key = Buffer::Utility::copyToBridgeData(key);
      header.value = Buffer::Utility::copyToBridgeData(value);
    }
  }

  envoy_headers raw_headers{
      .length = static_cast<envoy_map_size_t>(header_count),
      .entries = headers_list,
  };
  return raw_headers;
}

RawHeaderMap envoy_headers_as_raw_headers(envoy_headers raw_headers) {
  RawHeaderMap headers;
  for (auto i = 0; i < raw_headers.length; i++) {
    auto key = Buffer::Utility::copyToString(raw_headers.entries[i].key);
    auto value = Buffer::Utility::copyToString(raw_headers.entries[i].value);

    if (!headers.contains(key)) {
      headers.emplace(key, std::vector<std::string>());
    }
    headers[key].push_back(value);
  }
  release_envoy_headers(raw_headers);
  return headers;
}

} // namespace Platform
} // namespace Envoy
