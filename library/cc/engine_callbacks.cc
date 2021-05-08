#include "engine_callbacks.h"

namespace Envoy {
namespace Platform {

namespace {

void c_on_engine_running(void* context) {
  EngineCallbacks* engine_callbacks = static_cast<EngineCallbacks*>(context);
  engine_callbacks->on_engine_running();
}

void c_on_exit(void* context) {
  EngineCallbacks* engine_callbacks = static_cast<EngineCallbacks*>(context);
  delete engine_callbacks;
}

} // namespace

envoy_engine_callbacks EngineCallbacks::asEnvoyEngineCallbacks() {
  return envoy_engine_callbacks{
      .on_engine_running = &c_on_engine_running,
      .on_exit = &c_on_exit,
      .context = this,
  };
}

} // namespace Platform
} // namespace Envoy
