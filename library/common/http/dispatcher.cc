#include "library/common/http/dispatcher.h"

#include "library/common/buffer/utility.h"
#include "library/common/http/header_utility.h"

namespace Envoy {
namespace Http {

DirectStreamCallbacks::DirectStreamCallbacks(envoy_stream_t stream, envoy_observer observer,
                                             Dispatcher& http_dispatcher)
    : stream_(stream), observer_(observer), http_dispatcher_(http_dispatcher) {}

void DirectStreamCallbacks::onHeaders(HeaderMapPtr&& headers, bool end_stream) {
  ENVOY_LOG_MISC(info, "__________________CALLBACK TIME __________________");
  if (end_stream) {
    http_dispatcher_.removeStream(stream_);
  }
  ENVOY_LOG_MISC(info, "__________________Platform CALLBACK TIME __________________");
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

Dispatcher::Dispatcher(Event::Dispatcher& event_dispatcher,
                       Upstream::ClusterManager& cluster_manager)
    : event_dispatcher_(event_dispatcher), cluster_manager_(cluster_manager) {}

envoy_stream_t Dispatcher::startStream(envoy_observer observer) {
  envoy_stream_t new_stream_id = current_stream_id_++; // FIXME: This needs to be equivalent to
                                                       // getAnIncrement(); confirm or update.

  event_dispatcher_.post([this, observer, new_stream_id]() -> void {
    ENVOY_LOG(info, "Inside posted startStream");
    DirectStreamCallbacksPtr callbacks =
        std::make_unique<DirectStreamCallbacks>(new_stream_id, observer, *this);

    DirectStreamPtr direct_stream = std::make_unique<DirectStream>(std::move(callbacks));

    ENVOY_LOG(info, "Emplace direct stream");
    AsyncClient& async_client = cluster_manager_.httpAsyncClientForCluster("lyft_api");

    direct_stream->underlying_stream_ = async_client.start(*direct_stream->callbacks_, {});

    streams_.emplace(new_stream_id, std::move(direct_stream));

    ENVOY_LOG(info, "Started Stream");
  });

  ENVOY_LOG(info, "return createStream");
  return new_stream_id;
}

envoy_status_t Dispatcher::sendHeaders(envoy_stream_t stream_id, envoy_headers headers,
                                       bool end_stream) {
  event_dispatcher_.post([this, stream_id, headers, end_stream]() -> void {
    DirectStream* direct_stream = getStream(stream_id);
    if (direct_stream != nullptr) {
      ENVOY_LOG(info, "dispatch headers");
      direct_stream->headers_ = Utility::transformHeaders(headers);
      ENVOY_LOG(info, "transformed headers");
      direct_stream->underlying_stream_->sendHeaders(*direct_stream->headers_, end_stream);
      ENVOY_LOG(info, "sent headers to underlying stream");
    }
  });

  return ENVOY_SUCCESS;
}
envoy_status_t Dispatcher::sendData(envoy_stream_t, envoy_headers, bool) { return ENVOY_FAILURE; }
envoy_status_t Dispatcher::sendMetadata(envoy_stream_t, envoy_headers, bool) {
  return ENVOY_FAILURE;
}
envoy_status_t Dispatcher::sendTrailers(envoy_stream_t, envoy_headers) { return ENVOY_FAILURE; }
envoy_status_t Dispatcher::locallyCloseStream(envoy_stream_t) { return ENVOY_FAILURE; }
envoy_status_t Dispatcher::resetStream() { return ENVOY_FAILURE; }

Dispatcher::DirectStream* Dispatcher::getStream(envoy_stream_t stream_id) {
  // FIXME: assert thread
  ASSERT(event_dispatcher_.isThreadSafe(),
         "stream interaction must be performed on the event_dispatcher_'s thread.");
  // FIXME: fix names
  auto direct_stream_pair_it = streams_.find(stream_id);
  return (direct_stream_pair_it != streams_.end()) ? direct_stream_pair_it->second.get() : nullptr;
}

envoy_status_t Dispatcher::removeStream(envoy_stream_t) { return ENVOY_FAILURE; }

Dispatcher::DirectStream::DirectStream(DirectStreamCallbacksPtr callbacks)
    : callbacks_(std::move(callbacks)) {}

} // namespace Http
} // namespace Envoy
