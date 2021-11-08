#include "library/common/engine_handle.h"

// NOLINT(namespace-envoy)

envoy_status_t EngineHandle::runOnEngineDispatcher(envoy_engine_t,
                                                   std::function<void(Envoy::Engine&)> func) {
  if (auto e = engine()) {
    return e->dispatcher().post([func]() {
      if (auto e = engine()) {
        func(*e);
      }
    });
  }
  return ENVOY_FAILURE;
} // namespace Envoy

Envoy::EngineSharedPtr EngineHandle::strong_engine_;
Envoy::EngineWeakPtr EngineHandle::engine_;
