#include "library/common/http/async_stream_manager.h"

namespace Envoy {
namespace Http {

envoy_stream_t MobileAsyncStreamManager::createStream(envoy_observer observer) {
  stream_.emplace(current_stream_id_, MobileAsyncStream(observer));
  return current_stream_id_++;
}

std::shared_ptr<MobileAsyncStream> MobileAsyncStreamManager::getStream(envoy_stream_t stream_id) const {
  const auto async_stream_it = stream_.find(stream_id);
  return (async_stream_it != stream_.end()) ? std::make_shared<MobileAsyncStream>(async_stream_it->second)
                                            : nullptr;
}

} // namespace Http
} // namespace EnvoyMobile
