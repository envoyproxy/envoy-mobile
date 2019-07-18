#include "library/common/http/async_stream_manager.h"

namespace Envoy {
namespace Http {

Dispatcher::Dispatcher(Event::Dispatcher& event_dispatcher)
    : event_dispatcher_(event_dispatcher) {}

envoy_stream_t Dispatcher::startStream(envoy_observer observer) {
  envoy_stream_t new_stream_id = current_stream_id_++; // FIXME: This needs to be equivalent to getAnIncrement(); confirm or update.
  server_.dispatcher().post([this, observer, new_stream_id]() -> void {
    ENVOY_LOG(info, "Inside createStream");
    std::unique_ptr<DirectStreamCallbacks> callbacks =
      std::make_unique<DirectStreamCallbacks>(new_stream_id, observer, *this);
    
    ENVOY_LOG(info, "Emplace pair");
    streams_.emplace(new_stream_id,
                     std::make_pair(std::move(callbacks), async_client_.start(*callbacks, {})));
  
    ENVOY_LOG(info, "return createStream");
  });

  return new_stream_id;
}

AsyncClient::Stream* Dispatcher::getStream(envoy_stream_t stream_id) const {
  // FIXME: assert thread
  // FIXME: fix names
  const auto async_stream_pair_it = streams_.find(stream_id);
  return (async_stream_pair_it != streams_.end()) ? async_stream_pair_it->second.second : nullptr;
}

} // namespace Http
} // namespace Envoy
