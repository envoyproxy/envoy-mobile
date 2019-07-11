#pragma once

#include <atomic>
#include <unordered_map>

#include "library/common/types.h"
#include "library/common/http/async_stream.h"

namespace EnvoyMobile {
namespace Http {

class AsyncStreamManager {
public:
  AsyncStreamManager();

  envoy_stream_t createStream(envoy_observer);
  std::shared_ptr<AsyncStream> getStream(envoy_stream_t stream_id) const;

private:
  std::unordered_map<envoy_stream_t, AsyncStream> stream_;
  std::atomic<envoy_stream_t> current_stream_id_;
};

} // namespace Http
} // namespace EnvoyMobile