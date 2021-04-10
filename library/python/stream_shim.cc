#include "stream_shim.h"

#include "bytes_view.h"

namespace Envoy {
namespace Python {
namespace Stream {

Platform::Stream& send_data_shim(Platform::Stream& self, py::bytes data) {
  envoy_data raw_data = pyBytesAsEnvoyData(data);
  return self.sendData(raw_data);
}

void close_shim(Platform::Stream& self, py::bytes data) {
  envoy_data raw_data = pyBytesAsEnvoyData(data);
  self.close(raw_data);
}

} // namespace Stream
} // namespace Python
} // namespace Envoy
