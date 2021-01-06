#include "stream_prototype_shim.h"

#include "bytes_view.h"

namespace Envoy {
namespace Python {
namespace StreamPrototype {

Platform::StreamPrototype& set_on_headers_shim(Platform::StreamPrototype self,
                                               Platform::OnHeadersCallback closure) {
  return self.set_on_headers(
      [closure](Platform::ResponseHeadersSharedPtr headers, bool end_stream) {
        py::gil_scoped_acquire acquire;
        closure(std::move(headers), end_stream);
      });
}

Platform::StreamPrototype& set_on_data_shim(Platform::StreamPrototype& self,
                                            OnPyBytesDataCallback closure) {
  return self.set_on_data([closure](envoy_data data, bool end_stream) {
    py::gil_scoped_acquire acquire;
    py::bytes bytes = envoy_data_as_py_bytes(data);
    closure(bytes, end_stream);
  });
}

Platform::StreamPrototype& set_on_trailers_shim(Platform::StreamPrototype self,
                                                Platform::OnTrailersCallback closure) {
  return self.set_on_trailers([closure](Platform::ResponseTrailersSharedPtr trailers) {
    py::gil_scoped_acquire acquire;
    closure(std::move(trailers));
  });
}

Platform::StreamPrototype& set_on_error_shim(Platform::StreamPrototype self,
                                             Platform::OnErrorCallback closure) {
  return self.set_on_error([closure](Platform::EnvoyErrorSharedPtr error) {
    py::gil_scoped_acquire acquire;
    closure(std::move(error));
  });
}

Platform::StreamPrototype& set_on_complete_shim(Platform::StreamPrototype self,
                                                Platform::OnCompleteCallback closure) {
  return self.set_on_complete([closure]() {
    py::gil_scoped_acquire acquire;
    closure();
  });
}

Platform::StreamPrototype& set_on_cancel_shim(Platform::StreamPrototype self,
                                              Platform::OnCancelCallback closure) {
  return self.set_on_cancel([closure]() {
    py::gil_scoped_acquire acquire;
    closure();
  });
}

} // namespace StreamPrototype
} // namespace Python
} // namespace Envoy
