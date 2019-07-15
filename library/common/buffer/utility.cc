#include "library/common/buffer/utility.h"

#include "envoy/buffer/buffer.h"

namespace Envoy {
namespace Buffer {
namespace Utility {

/**
 * Transform envoy_data to Envoy::Buffer::Instance.
 * @param headers, the envoy_data to transform.
 * @return Envoy::Buffer::InstancePtr, the 1:1 transformation of the envoy_data param.
 */
Buffer::InstancePtr transformData(envoy_data) {
  return nullptr;
}

envoy_data transformData(Buffer::Instance&) { return {0, nullptr}; }

} // namespace Utility
} // namespace Buffer
} // namespace Envoy
