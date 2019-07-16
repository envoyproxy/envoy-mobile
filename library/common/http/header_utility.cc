#include "library/common/http/header_utility.h"

#include "common/http/header_map_impl.h"

namespace Envoy {
namespace Http {
namespace Utility {

static inline std::string convertString(envoy_string s) { return std::string(s.data, s.length); }

HeaderMapPtr transformHeaders(envoy_headers headers) {
  Http::HeaderMapPtr transformed_headers = std::make_unique<HeaderMapImpl>();
  for (uint64_t i = 0; i < headers.length; i++) {
    // FIXME: advance the pointer.
    transformed_headers->addCopy(LowerCaseString(convertString(headers.headers->name)),
                                 convertString(headers.headers->value));
  }
  return transformed_headers;
}

envoy_headers transformHeaders(HeaderMapPtr&& header_map) {
  auto headers = new envoy_header[header_map->size()];
  int i = 0;
  // FIXME: need to pass the lambda capture via the context void*.
  // We need to advance the headers by the length to append to the end.
  // That way we don't need a local counter we can just pass the envoy_headers struct.
  header_map->iterate(
      [&headers, &i](const HeaderEntry& header, void*) -> HeaderMap::Iterate {
        const absl::string_view header_name = header.key().getStringView();
        const absl::string_view header_value = header.value().getStringView();
        envoy_string name = {header_name.size(), strdup(header_name.data())};
        envoy_string value = {header_value.size(), strdup(header_value.data())};
        headers[i++] = {name, value};
        return HeaderMap::Iterate::Continue;
      },
      nullptr);
  return {header_map->size(), headers};
}

} // namespace Utility
} // namespace Http
} // namespace Envoy
