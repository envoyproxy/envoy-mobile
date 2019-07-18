#include "library/common/http/dispatcher.h"

#include "library/common/buffer/utility.h"
#include "library/common/http/header_utility.h"

namespace Envoy {
namespace Http {

DirectStreamCallbacks::DirectStreamCallbacks(envoy_stream_t stream, envoy_observer observer,
                                             Dispatcher& http_dispatcher)
    : stream_(stream), observer_(observer), http_dispatcher_(http_dispatcher) {}

void DirectStreamCallbacks::onHeaders(HeaderMapPtr&& headers, bool end_stream) {
  if (end_stream) {
    http_dispatcher_.removeStream(stream_);
  }
  observer_.h(stream_, Utility::transformHeaders(std::move(headers)), end_stream);
}

void DirectStreamCallbacks::onData(Buffer::Instance& data, bool end_stream) {
  if (end_stream) {
    http_dispatcher_.removeStream(stream_);
  }
  observer_.d(stream_, Envoy::Buffer::Utility::transformData(data), end_stream);
}

void DirectStreamCallbacks::onTrailers(HeaderMapPtr&& trailers) {
  http_dispatcher_.removeStream(stream_);
  observer_.t(stream_, Utility::transformHeaders(std::move(trailers)));
}

void DirectStreamCallbacks::onReset() {
  http_dispatcher_.removeStream(stream_);
  observer_.e(stream_, {ENVOY_STREAM_RESET, {0, nullptr}});
}

Dispatcher::Dispatcher(Event::Dispatcher& event_dispatcher) : event_dispatcher_(event_dispatcher) {}

envoy_stream_t Dispatcher::startStream(envoy_observer observer) {
  envoy_stream_t new_stream_id = current_stream_id_++; // FIXME: This needs to be equivalent to
                                                       // getAnIncrement(); confirm or update.
  event_dispatcher_.post([this, observer, new_stream_id]() -> void {
    ENVOY_LOG(info, "Inside createStream");
    std::unique_ptr<DirectStreamCallbacks> callbacks =
        std::make_unique<DirectStreamCallbacks>(new_stream_id, observer, *this);

    ENVOY_LOG(info, "Emplace pair");
    // FIXME get async client. Get the stream
    streams_.emplace(new_stream_id, std::make_pair(std::move(callbacks), nullptr));

    ENVOY_LOG(info, "return createStream");
  });

  return new_stream_id;
}

envoy_status_t Dispatcher::sendHeaders(envoy_stream_t, envoy_headers, bool) {
  return ENVOY_FAILURE;
}
envoy_status_t Dispatcher::sendData(envoy_stream_t, envoy_headers, bool) { return ENVOY_FAILURE; }
envoy_status_t Dispatcher::sendMetadata(envoy_stream_t, envoy_headers, bool) {
  return ENVOY_FAILURE;
}
envoy_status_t Dispatcher::sendTrailers(envoy_stream_t, envoy_headers) { return ENVOY_FAILURE; }
envoy_status_t Dispatcher::locallyCloseStream(envoy_stream_t) { return ENVOY_FAILURE; }
envoy_status_t Dispatcher::resetStream() { return ENVOY_FAILURE; }

AsyncClient::Stream* Dispatcher::getStream(envoy_stream_t stream_id) const {
  // FIXME: assert thread
  // FIXME: fix names
  const auto async_stream_pair_it = streams_.find(stream_id);
  return (async_stream_pair_it != streams_.end()) ? async_stream_pair_it->second.second : nullptr;
}

envoy_status_t Dispatcher::removeStream(envoy_stream_t) { return ENVOY_FAILURE; }

} // namespace Http
} // namespace Envoy
