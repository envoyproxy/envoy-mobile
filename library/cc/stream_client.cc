#include "stream_client.h"

// NOLINT(namespace-envoy)

StreamClient::StreamClient(envoy_engine_t engine) : engine_(engine) {}

StreamPrototypeSharedPtr StreamClient::new_stream_prototype() {
  return std::make_shared<StreamPrototype>(this->engine_);
}
