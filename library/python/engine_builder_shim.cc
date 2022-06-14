#include "engine_builder_shim.h"

namespace py = pybind11;

namespace Envoy {
namespace Python {
namespace EngineBuilder {

Platform::EngineBuilder& setOnEngineRunningShim(Platform::EngineBuilder& self,
                                                std::function<void()> closure) {
  // TODO(jpsim): Expose engine handle in the Python API
  return self.setOnEngineRunning([closure](envoy_engine_t) {
    py::gil_scoped_acquire acquire;
    closure();
  });
}

} // namespace EngineBuilder
} // namespace Python
} // namespace Envoy
