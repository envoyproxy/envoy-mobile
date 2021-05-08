#pragma once

#include <functional>

#include "engine.h"

namespace Envoy {
namespace Platform {

struct EngineCallbacks {
  std::function<void()> on_engine_running;
  // unused:
  // std::function<void()> on_exit;

  envoy_engine_callbacks asEnvoyEngineCallbacks();

  EngineSharedPtr parent;
};

using EngineCallbacksPtr = std::unique_ptr<EngineCallbacks>;

} // namespace Platform
} // namespace Envoy
