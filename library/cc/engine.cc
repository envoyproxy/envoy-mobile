#include "engine.h"

#include "library/common/main_interface.h"
#include "library/common/types/c_types.h"

namespace Envoy {
namespace Platform {

Engine::Engine(envoy_engine_t engine) : engine_(engine), terminated_(false) {}

// we lazily construct the stream and pulse clients
// because they either require or will require a weak ptr
// which can't be provided from inside of the constructor
// because of how std::enable_shared_from_this works
StreamClientSharedPtr Engine::streamClient() {
  return std::make_shared<StreamClient>(this->shared_from_this());
}

PulseClientSharedPtr Engine::pulseClient() { return std::make_shared<PulseClient>(); }

void Engine::terminate() {
  if (this->terminated_) {
    throw std::runtime_error("attempting to double terminate Engine");
  }
  terminate_engine(this->engine_);
  this->terminated_ = true;
}

} // namespace Platform
} // namespace Envoy
