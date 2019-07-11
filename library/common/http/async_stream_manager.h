#pragma once

#include <atomic>
#include <unordered_map>

#include "library/common/http/async_stream.h"
#include "library/common/types.h"

namespace Envoy {
namespace Http {

class MobileAsyncStreamManager {
public:
  MobileAsyncStreamManager();

  envoy_stream_t createStream(envoy_observer);
  std::shared_ptr<MobileAsyncStream> getStream(envoy_stream_t stream_id) const;

private:
  std::unordered_map<envoy_stream_t, MobileAsyncStream> stream_;
  std::atomic<envoy_stream_t> current_stream_id_;
};

} // namespace Http
} // namespace EnvoyMobile
