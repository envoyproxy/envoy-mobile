#include "library/common/http/async_stream_manager.h"

namespace Envoy {
namespace Http {

MobileAsyncStreamManager::MobileAsyncStreamManager(AsyncClient& async_client)
    : async_client_(async_client) {}

envoy_stream_t MobileAsyncStreamManager::createStream(envoy_observer observer) {
  std::unique_ptr<MobileAsyncStreamCallbacks> callbacks =
      std::make_unique<MobileAsyncStreamCallbacks>(current_stream_id_, observer);
  streams_.emplace(current_stream_id_,
                   std::make_pair(callbacks, async_client_.start(*callbacks, {})));
  return current_stream_id_++;
}

AsyncClient::Stream* MobileAsyncStreamManager::getStream(envoy_stream_t stream_id) const {
  const auto async_stream_pair_it = streams_.find(stream_id);
  return (async_stream_pair_it != streams_.end()) ? async_stream_pair_it->second.second : nullptr;
}

} // namespace Http
} // namespace Envoy
