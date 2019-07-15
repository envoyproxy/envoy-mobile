#pragma once

#include <atomic>
#include <unordered_map>

#include "envoy/http/async_client.h"

#include "library/common/http/async_stream_callbacks.h"
#include "library/common/include/c_types.h"

namespace Envoy {
namespace Http {

class MobileAsyncStreamManager {
public:
  MobileAsyncStreamManager(AsyncClient&);

  envoy_stream_t createStream(envoy_observer);
  AsyncClient::Stream* getStream(envoy_stream_t stream_id) const;

private:
  std::unordered_map<envoy_stream_t, AsyncClient::Stream*> streams_;
  std::atomic<envoy_stream_t> current_stream_id_;
  AsyncClient& async_client_;
};

} // namespace Http
} // namespace Envoy
