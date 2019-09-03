#pragma once

#include "envoy/buffer/buffer.h"

#include "library/common/types/c_types.h"

namespace Envoy {
namespace Buffer {
namespace Utility {

/**
 * Transform envoy_data to Envoy::Buffer::Instance.
 * @param headers, the envoy_data to transform.
 * @return Envoy::Buffer::InstancePtr, the 1:1 transformation of the envoy_data param.
 */
Buffer::InstancePtr toInternalData(envoy_data data);

/**
 * Transform from Buffer::Instance to envoy_data.
 * Note: this function allocates data on the heap, which can fail.
 * In case of failure the returned envoy_data will have non-zero length but nullptr bytes.
 * It is up to the caller to verify this failure scenario.
 * @param data, the Buffer::Instance to transform.
 * @return envoy_data, the 1:1 transformation of the Buffer::Instance param.
 */
envoy_data toBridgeData(Buffer::Instance&);

} // namespace Utility
} // namespace Buffer
} // namespace Envoy
