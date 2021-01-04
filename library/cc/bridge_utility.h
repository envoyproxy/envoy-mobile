#pragma once

#include <string>
#include <vector>

#include "headers.h"
#include "library/common/types/c_types.h"

namespace Envoy {
namespace Platform {

envoy_data buffer_as_envoy_data(const std::vector<uint8_t>& data);
envoy_data string_as_envoy_data(const std::string& data);
envoy_headers raw_header_map_as_envoy_headers(const RawHeaderMap& headers);

std::vector<uint8_t> envoy_data_as_buffer(envoy_data raw_data);
std::string envoy_data_as_string(envoy_data raw_data);
RawHeaderMap envoy_headers_as_raw_header_map(envoy_headers raw_headers);

} // namespace Platform
} // namespace Envoy
