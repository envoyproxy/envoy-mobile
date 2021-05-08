#include "stream_client.h"

namespace Envoy {
namespace Platform {

StreamClient::StreamClient(EngineWeakPtr engine) : engine_(engine) {}

StreamPrototypeSharedPtr StreamClient::newStreamPrototype() {
  if (auto engine = this->engine_.lock()) {
    return std::make_shared<StreamPrototype>(engine);
  } else {
    throw std::runtime_error("attempted to use Engine weakptr after free");
  }
}

} // namespace Platform
} // namespace Envoy
