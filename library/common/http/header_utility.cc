#include "library/common/http/header_utility.h"

namespace Envoy {
namespace Http {
namespace Utility {

static inline std::string ConvertString(envoy_string s) {
  return std::string(s.data, s.length);
}

HeaderMapPtr transformHeaders(envoy_headers headers) {
  Http::HeaderMapPtr transformed_headers(new Http::HeaderMapImpl());
  for (int i = 0; i < headers.size; i++) {
    transformed_headers->addCopy(LowerCaseString(ConvertString(headers[i].name)),
                                 ConvertString(headers[i].value));
  }
  return transformed_headers;
}

envoy_headers transformHeaders(HeaderMapPtr&& header_map) {
  auto headers = new envoy_header[header_map.size()];
  int i = 0;
  header_map.iterate(
      [&headers, &i](const HeaderEntry& header, void*) -> HeaderMap::Iterate {
        const absl::string_view header_name = header.key().getStringView();
        const absl::string_view header_value = header.value().getStringView();
        envoy_string name = { strdup(header_name.data()), header_name.size() };
        envoy_string value = { strdup(header_value.data)), header_value.size() };
        headers[i++] = { name, value };
        return HeaderMap::Iterate::Continue;
      },
      nullptr
  );
  return { header_map.size(), headers };
}

} // namespace Utility
} // namespace Http
} // namespace Envoy
