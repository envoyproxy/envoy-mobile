#include "engine_builder_shim.h"

namespace py = pybind11;

namespace Envoy {
namespace Python {
namespace EngineBuilder {

Platform::EngineBuilder& setOnEngineRunningShim(Platform::EngineBuilder& self,
                                                std::function<void(envoy_engine_t)> closure) {
  return self.setOnEngineRunning([closure](envoy_engine_t engine) {
    py::gil_scoped_acquire acquire;
    closure(engine);
  });
}

} // namespace EngineBuilder
} // namespace Python
} // namespace Envoy
