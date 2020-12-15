#pragma once

// NOLINT(namespace-envoy)

#include <memory>

#include "stream_prototype.h"

class StreamClient {
public:
  StreamClient(envoy_engine_t engine);

  StreamPrototypeSharedPtr new_stream_prototype();

private:
  envoy_engine_t engine_;
};

using StreamClientSharedPtr = std::shared_ptr<StreamClient>;
