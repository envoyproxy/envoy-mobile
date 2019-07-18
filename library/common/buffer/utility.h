#pragma once

#include "envoy/buffer/buffer.h"

#include "library/common/include/c_types.h"

namespace Envoy {
namespace Buffer {
namespace Utility {

/**
 * Transform envoy_data to Envoy::Buffer::Instance.
 * @param headers, the envoy_data to transform.
 * @return Envoy::Buffer::InstancePtr, the 1:1 transformation of the envoy_data param.
 */
Buffer::InstancePtr transformData(envoy_data data);

envoy_data transformData(Buffer::Instance&);

} // namespace Utility
} // namespace Buffer
} // namespace Envoy
