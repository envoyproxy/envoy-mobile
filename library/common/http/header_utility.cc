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
  
}

} // namespace Utility
} // namespace Http
} // namespace Envoy
