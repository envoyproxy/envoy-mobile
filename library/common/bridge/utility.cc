#include "library/common/bridge/utility.h"

#include <string>

#include "library/common/data/utility.h"

namespace Envoy {
namespace Bridge {
namespace Utility {

envoy_error_code_t errorCodeFromLocalStatus(Http::Code status) {
  switch (status) {
  case Http::Code::RequestTimeout:
    return ENVOY_REQUEST_TIMEOUT;
  case Http::Code::PayloadTooLarge:
    return ENVOY_BUFFER_LIMIT_EXCEEDED;
  case Http::Code::ServiceUnavailable:
    return ENVOY_CONNECTION_FAILURE;
  default:
    return ENVOY_UNDEFINED_ERROR;
  }
}

} // namespace Utility
} // namespace Bridge
} // namespace Envoy
