#include "library/common/buffer/utility.h"

#include "envoy/buffer/buffer.h"

namespace Envoy {
namespace Buffer {
namespace Utility {

Buffer::InstancePtr transformData(envoy_data) { return nullptr; }

envoy_data transformData(Buffer::Instance&) { return {0, nullptr}; }

} // namespace Utility
} // namespace Buffer
} // namespace Envoy
