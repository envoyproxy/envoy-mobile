#pragma once

#include <cstddef>
#include <functional>

#include "engine.h"
#include "envoy_error.h"
#include "executor.h"
#include "response_headers.h"
#include "response_trailers.h"
#include "stream.h"
#include "stream_callbacks.h"

class Engine;

class StreamPrototype {
public:
  StreamPrototype(Engine engine);

  Stream start(Executor executor);

  StreamPrototype& set_on_response_headers(
      std::function<void(const ResponseHeaders& headers, bool end_stream)> closure);
  StreamPrototype& set_on_response_data(
      std::function<void(const std::vector<std::byte>& bytes, bool end_stream)> closure);
  StreamPrototype&
  set_on_response_trailers(std::function<void(const ResponseTrailers& trailers)> closure);
  StreamPrototype& set_on_error(std::function<void(const EnvoyError& error)> closure);
  StreamPrototype& set_on_cancel(std::function<void()> closure);

private:
  StreamCallbacks callbacks_;
  EnvoyHttpCallbacksAdapter adapter_;
};
