#pragma once

#include <memory>

#include "engine.h"
#include "stream_prototype.h"

namespace Envoy {
namespace Platform {

class Engine;
using EngineWeakPtr = std::weak_ptr<Engine>;

class StreamPrototype;
using StreamPrototypeSharedPtr = std::shared_ptr<StreamPrototype>;

class StreamClient {
public:
  StreamClient(EngineWeakPtr engine);

  StreamPrototypeSharedPtr new_stream_prototype();

private:
  EngineWeakPtr engine_;
};

using StreamClientSharedPtr = std::shared_ptr<StreamClient>;

} // namespace Platform
} // namespace Envoy
