#pragma once

// NOLINT(namespace-envoy)

#include <cstddef>
#include <functional>
#include <memory>

#include "envoy_error.h"
#include "executor.h"
#include "library/common/types/c_types.h"
#include "response_headers.h"
#include "response_trailers.h"
#include "stream.h"
#include "stream_callbacks.h"

class StreamPrototype {
public:
  StreamPrototype(envoy_engine_t engine);

  StreamSharedPtr start(ExecutorSharedPtr executor);

  StreamPrototype& set_on_headers(OnHeadersCallback closure);
  StreamPrototype& set_on_data(OnDataCallback closure);
  StreamPrototype& set_on_trailers(OnTrailersCallback closure);
  StreamPrototype& set_on_error(OnErrorCallback closure);
  StreamPrototype& set_on_complete(OnCompleteCallback closure);
  StreamPrototype& set_on_cancel(OnCancelCallback closure);

private:
  envoy_engine_t engine_;
  StreamCallbacksSharedPtr callbacks_;
};

using StreamPrototypeSharedPtr = std::shared_ptr<StreamPrototype>;
