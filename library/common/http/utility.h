#pragma once

#include "library/common/types.h"

#include "envoy/buffer/buffer.h"
#include "envoy/http/header_map.h"

namespace EnvoyMobile {
namespace Http {
namespace Utility {

/**
 * Transform envoy_headers to Envoy::Http::HeaderMap.
 * @param headers, the envoy_headers to transform.
 * @return Envoy::Http::HeaderMapPtr, the HeaderMap 1:1 transformation of the headers param.
 */
Envoy::Http::HeaderMapPtr transformHeaders(envoy_headers headers);

/**
 * Transform envoy_data to Envoy::Buffer::Instance.
 * @param headers, the envoy_data to transform.
 * @return Envoy::Buffer::InstancePtr, the 1:1 transformation of the envoy_data param.
 */
Envoy::Buffer::InstancePtr transformData(envoy_data headers);

} // namespace Utility
} // namespace Http
} // namespace EnvoyMobile