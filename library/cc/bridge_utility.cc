#include "bridge_utility.h"

#include <sstream>

namespace Envoy {
namespace Platform {

// TODO(crockeo): we always copy memory across boundaries; consider allowing for moves and/or
// shared ownership w/ reference counting via envoy-mobile's release callbacks

envoy_data buffer_as_envoy_data(const std::vector<uint8_t>& data) {
  size_t byte_len = sizeof(uint8_t) * data.size();

  uint8_t* bytes = static_cast<uint8_t*>(safe_malloc(byte_len));
  memcpy(bytes, &data[0], byte_len);
  envoy_data raw_data{
      .length = byte_len,
      .bytes = bytes,
      .release = envoy_noop_release,
      .context = nullptr,
  };

  return raw_data;
}

envoy_data string_as_envoy_data(const std::string& data) {
  size_t byte_len = sizeof(uint8_t) * data.size();

  uint8_t* bytes = static_cast<uint8_t*>(safe_malloc(byte_len));
  memcpy(bytes, &data[0], byte_len);
  envoy_data raw_data{
      .length = byte_len,
      .bytes = bytes,
      .release = envoy_noop_release,
      .context = nullptr,
  };

  return raw_data;
}

envoy_headers raw_headers_as_envoy_headers(const RawHeaders& headers) {
  envoy_header* headers_list =
      static_cast<envoy_header*>(safe_malloc(sizeof(envoy_header) * headers.size()));

  std::ostringstream value_fmt;
  size_t i = 0;
  for (const auto& pair : headers) {
    const auto& key = pair.first;
    const auto& values = pair.second;

    for (size_t i = 0; i < values.size(); i++) {
      value_fmt << values[i];
      if (i != values.size() - 1) {
        value_fmt << ",";
      }
    }

    envoy_header& header = headers_list[i++];
    header.key = string_as_envoy_data(key);
    header.value = string_as_envoy_data(value_fmt.str());

    value_fmt.str("");
    value_fmt.clear();
  }

  envoy_headers raw_headers{
      .length = static_cast<envoy_header_size_t>(headers.size()),
      .headers = headers_list,
  };
  return raw_headers;
}

std::vector<uint8_t> envoy_data_as_buffer(envoy_data raw_data) {
  size_t len = raw_data.length / sizeof(uint8_t);

  std::vector<uint8_t> buffer;
  buffer.resize(len);
  memcpy(&buffer[0], raw_data.bytes, raw_data.length);

  return buffer;
}

std::string envoy_data_as_string(envoy_data raw_data) {
  size_t len = raw_data.length / sizeof(uint8_t);

  std::string str;
  str.resize(len);
  memcpy(&str[0], raw_data.bytes, raw_data.length);

  return str;
}

RawHeaders envoy_headers_as_raw_headers(envoy_headers raw_headers) {
  RawHeaders headers;
  for (auto i = 0; i < raw_headers.length; i++) {
    auto key = envoy_data_as_string(raw_headers.headers[i].key);
    auto value = envoy_data_as_string(raw_headers.headers[i].value);

    // split the header value by ","
    std::vector<std::string> values;
    size_t last_pos = 0;
    size_t pos = 0;
    while ((pos = value.find(",", last_pos)) != std::string::npos) {
      values.push_back(value.substr(last_pos, pos - last_pos));
      last_pos = pos + 1;
    }
    values.push_back(value.substr(last_pos, pos - last_pos));

    headers[key] = values;
  }
  return headers;
}

} // namespace Platform
} // namespace Envoy
