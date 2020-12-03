#pragma once

#include <cstddef>
#include <optional>
#include <vector>

#include "envoy_error.h"
#include "executor.h"
#include "response_headers.h"
#include "response_trailers.h"


using OnHeadersCallback = std::function<void(ResponseHeaders headers, bool end_stream)>;
using OnDataCallback = std::function<void(std::vector<std::byte> data, bool end_stream)>;
using OnTrailersCallback = std::function<void(ResponseTrailers trailers)>;
using OnCancelCallback = std::function<void()>;
using OnErrorCallback = std::function<void(EnvoyError error)>;

struct StreamCallbacks {
  std::optional<OnHeadersCallback> on_headers;
  std::optional<OnDataCallback> on_data;
  std::optional<OnTrailersCallback> on_trailers;
  std::optional<OnCancelCallback> on_cancel;
  std::optional<OnErrorCallback> on_error;
};

class EnvoyHttpCallbacksAdapter {
public:
  EnvoyHttpCallbacksAdapter(Executor executor, StreamCallbacks callbacks);

private:
  // TODO(crockeo): figure out lifecycle management here
  Executor& executor_;
  StreamCallbacks stream_callbacks_;
};
