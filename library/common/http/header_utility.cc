#include "library/common/http/header_utility.h"

#include "common/http/header_map_impl.h"

namespace Envoy {
namespace Http {
namespace Utility {

static inline std::string convertString(envoy_string s) { return std::string(s.data, s.length); }

HeaderMapPtr transformHeaders(envoy_headers headers) {
  Http::HeaderMapPtr transformed_headers = std::make_unique<HeaderMapImpl>();
  for (uint64_t i = 0; i < headers.length; i++) {
    transformed_headers->addCopy(LowerCaseString(convertString(headers.headers[i].name)),
                                 convertString(headers.headers[i].value));
  }
  return transformed_headers;
}

envoy_headers transformHeaders(HeaderMapPtr&& header_map) {
  envoy_header* headers = new envoy_header[header_map->size()];
  envoy_headers transformed_headers;
  transformed_headers.length = 0;
  transformed_headers.headers = headers;

  header_map->iterate(
      [](const HeaderEntry& header, void* context) -> HeaderMap::Iterate {
        envoy_headers* transformed_headers = static_cast<envoy_headers*>(context);

        const absl::string_view header_name = header.key().getStringView();
        const absl::string_view header_value = header.value().getStringView();

        envoy_string name = {header_name.size(), strdup(header_name.data())};
        envoy_string value = {header_value.size(), strdup(header_value.data())};

        transformed_headers->headers[transformed_headers->length] = {name, value};
        transformed_headers->length++;

        return HeaderMap::Iterate::Continue;
      },
      &transformed_headers);
  return transformed_headers;
}

} // namespace Utility
} // namespace Http
} // namespace Envoy
