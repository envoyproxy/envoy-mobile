#include "bridge_utility.h"

#include <sstream>

namespace Envoy {
namespace Platform {

// TODO(crockeo): we always copy memory across boundaries; consider allowing for moves and/or
// shared ownership w/ reference counting via envoy-mobile's release callbacks

envoy_data string_as_envoy_data(const std::string& str) {
  uint8_t* bytes = static_cast<uint8_t*>(safe_malloc(str.size()));
  memcpy(bytes, &str[0], str.size());
  return envoy_data{
      .length = str.size(),
      .bytes = bytes,
      .release = free,
      .context = bytes,
  };
}

std::string envoy_data_as_string(envoy_data data) {
  std::string str;
  str.resize(data.length);
  memcpy(&str[0], data.bytes, data.length);
  data.release(data.context);
  return str;
}

envoy_headers raw_header_map_as_envoy_headers(const RawHeaderMap& headers) {
  size_t header_count = 0;
  for (const auto& pair : headers) {
    header_count += pair.second.size();
  }

  envoy_header* headers_list =
      static_cast<envoy_header*>(safe_malloc(sizeof(envoy_header) * header_count));

  size_t i = 0;
  for (const auto& pair : headers) {
    const auto& key = pair.first;
    for (const auto& value : pair.second) {
      envoy_header& header = headers_list[i++];
      header.key = string_as_envoy_data(key);
      header.value = string_as_envoy_data(value);
    }
  }

  envoy_headers raw_headers{
      .length = static_cast<envoy_header_size_t>(header_count),
      .headers = headers_list,
  };
  return raw_headers;
}

RawHeaderMap envoy_headers_as_raw_headers(envoy_headers raw_headers) {
  RawHeaderMap headers;
  for (auto i = 0; i < raw_headers.length; i++) {
    auto key = envoy_data_as_string(raw_headers.headers[i].key);
    auto value = envoy_data_as_string(raw_headers.headers[i].value);

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
