#include "library/common/buffer/utility.h"

#include "envoy/buffer/buffer.h"

namespace Envoy {
namespace Buffer {
namespace Utility {

// TODO: implement this https://github.com/lyft/envoy-mobile/issues/284.
Buffer::InstancePtr toNativeData(envoy_data) { return nullptr; }

// TODO: implement this https://github.com/lyft/envoy-mobile/issues/284.
envoy_data toBrdigeData(Buffer::Instance&) { return envoy_nodata; }

} // namespace Utility
} // namespace Buffer
} // namespace Envoy
