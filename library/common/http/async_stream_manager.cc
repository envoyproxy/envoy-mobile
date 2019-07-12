#include "library/common/http/async_stream_manager.h"

namespace Envoy {
namespace Http {

MobileAsyncStreamManager::MobileAsyncStreamManager(AsyncClient& async_client)
    : async_client_(async_client) {}

envoy_stream_t MobileAsyncStreamManager::createStream(envoy_observer observer) {
  // FIXME: need to deal with callback lifetime.
  MobileAsyncStreamCallbacks callbacks = MobileAsyncStreamCallbacks(observer);
  streams_.emplace(current_stream_id_, async_client_.start(callbacks, {}));
  return current_stream_id_++;
}

AsyncClient::Stream* MobileAsyncStreamManager::getStream(envoy_stream_t stream_id) const {
  const auto async_stream_it = streams_.find(stream_id);
  return (async_stream_it != streams_.end()) ? async_stream_it->second : nullptr;
}

} // namespace Http
} // namespace Envoy
