#include "library/common/engine_handle.h"

#include <atomic>

static std::atomic<envoy_engine_t> default_handle_{0};
static std::once_flag default_handle_once_flag;

namespace Envoy {

envoy_engine_t EngineHandle::defaultHandle() { return default_handle_.load(); }

envoy_status_t EngineHandle::runOnEngineDispatcher(envoy_engine_t handle,
                                                   std::function<void(Envoy::Engine&)> func) {
  if (auto engine = reinterpret_cast<Envoy::Engine*>(handle)) {
    return engine->dispatcher().post([engine, func]() { func(*engine); });
  }
  return ENVOY_FAILURE;
}

envoy_engine_t EngineHandle::initEngine(envoy_engine_callbacks callbacks, envoy_logger logger,
                                        envoy_event_tracker event_tracker) {
  auto engine = new Envoy::Engine(callbacks, logger, event_tracker);
  envoy_engine_t handle = reinterpret_cast<envoy_engine_t>(engine);
  std::call_once(default_handle_once_flag, [handle]() { default_handle_.exchange(handle); });
  return handle;
}

envoy_status_t EngineHandle::runEngine(envoy_engine_t handle, const char* config,
                                       const char* log_level, const char* admin_address_path) {
  if (auto engine = reinterpret_cast<Envoy::Engine*>(handle)) {
    engine->run(config, log_level, admin_address_path);
    return ENVOY_SUCCESS;
  }
  return ENVOY_FAILURE;
}

void EngineHandle::terminateEngine(envoy_engine_t handle, bool release) {
  auto engine = reinterpret_cast<Envoy::Engine*>(handle);
  engine->terminate();
  if (release) {
    // TODO(jpsim): Always delete engine to avoid leaking it
    delete engine;
  }
}

} // namespace Envoy
