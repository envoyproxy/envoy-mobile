#pragma once

#include "envoy/buffer/buffer.h"
#include "envoy/http/header_map.h"

#include "library/common/include/c_types.h"

namespace Envoy {
namespace Http {
namespace Utility {

/**
 * Transform envoy_headers to Envoy::Http::HeaderMap.
 * @param headers, the envoy_headers to transform.
 * @return Envoy::Http::HeaderMapPtr, the HeaderMap 1:1 transformation of the headers param.
 */
HeaderMapPtr transformHeaders(envoy_headers headers);


/**
 */
envoy_headers transformHeaders(HeaderMapPtr&&);

} // namespace Utility
} // namespace Http
} // namespace Envoy
