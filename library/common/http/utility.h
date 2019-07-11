#pragma once

#include "envoy/buffer/buffer.h"
#include "envoy/http/header_map.h"

#include "library/common/types.h"

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
 * Transform envoy_data to Envoy::Buffer::Instance.
 * @param headers, the envoy_data to transform.
 * @return Envoy::Buffer::InstancePtr, the 1:1 transformation of the envoy_data param.
 */
Buffer::InstancePtr transformData(envoy_data data);

} // namespace Utility
} // namespace Http
} // namespace Envoy
