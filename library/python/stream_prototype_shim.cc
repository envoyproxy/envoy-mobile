#include "stream_prototype_shim.h"

#include "bytes_view.h"

namespace Envoy {
namespace Python {
namespace StreamPrototype {

Platform::StreamPrototype& set_on_data_shim(Platform::StreamPrototype& self,
                                            OnPyBytesDataCallback on_data) {
  return self.set_on_data([on_data](envoy_data data, bool end_stream) {
    py::bytes bytes = envoy_data_as_py_bytes(data);
    on_data(bytes, end_stream);
  });
}

} // namespace StreamPrototype
} // namespace Python
} // namespace Envoy
