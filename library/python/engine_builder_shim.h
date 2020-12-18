#pragma once

#include <functional>

#include "library/cc/engine_builder.h"
#include "pybind11/pybind11.h"

namespace Envoy {
namespace Python {

Platform::EngineBuilder& set_on_engine_running_shim(Platform::EngineBuilder& self,
                                                    std::function<void()> closure);

} // namespace Python
} // namespace Envoy
