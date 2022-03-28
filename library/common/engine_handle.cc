#include "library/common/engine_handle.h"

namespace Envoy {

envoy_status_t EngineHandle::runOnEngineDispatcher(envoy_engine_t handle,
                                                   std::function<void(Envoy::Engine&)> func) {
  auto engine = reinterpret_cast<Envoy::Engine*>(handle);
  if (!engine || engine->isTerminated()) {
    return ENVOY_FAILURE;
  }
  return engine->dispatcher().post([engine, func]() { func(*engine); });
}

envoy_engine_t EngineHandle::initEngine(envoy_engine_callbacks callbacks, envoy_logger logger,
                                        envoy_event_tracker event_tracker) {
  auto engine = new Envoy::Engine(callbacks, logger, event_tracker);
  return reinterpret_cast<envoy_engine_t>(engine);
}

envoy_status_t EngineHandle::runEngine(envoy_engine_t handle, const char* config,
                                       const char* log_level) {
  auto engine = reinterpret_cast<Envoy::Engine*>(handle);
  if (!engine || engine->isTerminated()) {
    return ENVOY_FAILURE;
  }
  engine->run(config, log_level);
  return ENVOY_SUCCESS;
}

void EngineHandle::terminateEngine(envoy_engine_t handle) {
  auto engine = reinterpret_cast<Envoy::Engine*>(handle);
  engine->terminate();
  // TODO(jpsim): delete engine to avoid leaking it
}

void EngineHandle::release(envoy_engine_t handle) {
  auto engine = reinterpret_cast<Envoy::Engine*>(handle);
  delete engine;
}

} // namespace Envoy
