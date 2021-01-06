#pragma once

#include <functional>

#include "library/cc/stream_prototype.h"
#include "library/common/types/c_types.h"
#include "pybind11/pybind11.h"

namespace py = pybind11;

namespace Envoy {
namespace Python {
namespace StreamPrototype {

using OnPyBytesDataCallback = std::function<void(py::bytes bytes, bool end_stream)>;

Platform::StreamPrototype& set_on_data_shim(Platform::StreamPrototype& self,
                                            OnPyBytesDataCallback on_data);

} // namespace StreamPrototype
} // namespace Python
} // namespace Envoy
