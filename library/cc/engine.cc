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
  if (!this->stream_client_) {
    this->stream_client_ = std::make_shared<StreamClient>(this->weak_from_this());
  }
  return this->stream_client_;
}

PulseClientSharedPtr Engine::pulseClient() {
  if (!this->pulse_client_) {
    this->pulse_client_ = std::make_shared<PulseClient>();
  }
  return this->pulse_client_;
}

void Engine::terminate() {
  if (this->terminated_) {
    throw std::runtime_error("attempting to double terminate Engine");
  }
  terminate_engine(this->engine_);
  this->terminated_ = true;
}

} // namespace Platform
} // namespace Envoy
