#include "library/common/http/dispatcher.h"

#include "library/common/buffer/utility.h"
#include "library/common/http/header_utility.h"

namespace Envoy {
namespace Http {

Dispatcher::DirectStreamCallbacks::DirectStreamCallbacks(envoy_stream_t stream,
                                                         envoy_observer observer,
                                                         Dispatcher& http_dispatcher)
    : stream_(stream), observer_(observer), http_dispatcher_(http_dispatcher) {}

void Dispatcher::DirectStreamCallbacks::onHeaders(HeaderMapPtr&& headers, bool end_stream) {
  ENVOY_LOG(debug, "[S{}] response headers for stream (end_stream={}):\n{}", stream_, end_stream,
            *headers);
  observer_.on_headers_f(Utility::transformHeaders(*headers), end_stream, observer_.context);
  http_dispatcher_.closeRemote(stream_, end_stream);
}

void Dispatcher::DirectStreamCallbacks::onData(Buffer::Instance& data, bool end_stream) {
  ENVOY_LOG(debug, "[S{}] response data for stream (length={} end_stream={})", stream_,
            data.length(), end_stream);
  observer_.on_data_f(Envoy::Buffer::Utility::transformData(data), end_stream, observer_.context);
  http_dispatcher_.closeRemote(stream_, end_stream);
}

void Dispatcher::DirectStreamCallbacks::onTrailers(HeaderMapPtr&& trailers) {
  ENVOY_LOG(debug, "[S{}] response trailers for stream:\n{}", stream_, *trailers);
  observer_.on_trailers_f(Utility::transformHeaders(*trailers), observer_.context);
  http_dispatcher_.closeRemote(stream_, true);
}

void Dispatcher::DirectStreamCallbacks::onReset() {
  ENVOY_LOG(debug, "[S{}] remote reset stream", stream_);
  observer_.on_error_f({ENVOY_STREAM_RESET, {0, nullptr}}, observer_.context);
  http_dispatcher_.closeRemote(stream_, true);
}

Dispatcher::DirectStream::DirectStream(envoy_stream_t stream_id,
                                       AsyncClient::Stream& underlying_stream,
                                       DirectStreamCallbacksPtr&& callbacks)
    : stream_id_(stream_id), underlying_stream_(underlying_stream),
      callbacks_(std::move(callbacks)) {}

Dispatcher::Dispatcher(Event::Dispatcher& event_dispatcher,
                       Upstream::ClusterManager& cluster_manager)
    : current_stream_id_(0), event_dispatcher_(event_dispatcher),
      cluster_manager_(cluster_manager) {}

envoy_stream_t Dispatcher::startStream(envoy_observer observer) {
  envoy_stream_t new_stream_id = current_stream_id_++;

  event_dispatcher_.post([this, observer, new_stream_id]() -> void {
    DirectStreamCallbacksPtr callbacks =
        std::make_unique<DirectStreamCallbacks>(new_stream_id, observer, *this);
    AsyncClient& async_client = cluster_manager_.httpAsyncClientForCluster("egress_cluster");
    AsyncClient::Stream* underlying_stream = async_client.start(*callbacks, {});

    if (!underlying_stream) {
      // TODO: this callback might fire before the startStream function returns.
      // Take this into account when thinking about stream cancellation.
      callbacks->onReset();
    } else {
      DirectStreamPtr direct_stream =
          std::make_unique<DirectStream>(new_stream_id, *underlying_stream, std::move(callbacks));
      streams_.emplace(new_stream_id, std::move(direct_stream));
      ENVOY_LOG(debug, "[S{}] start stream", new_stream_id);
    }
  });

  return new_stream_id;
}

envoy_status_t Dispatcher::sendHeaders(envoy_stream_t stream_id, envoy_headers headers,
                                       bool end_stream) {
  event_dispatcher_.post([this, stream_id, headers, end_stream]() -> void {
    DirectStream* direct_stream = getStream(stream_id);
    // If direct_stream is not found, it means the stream has already closed or been reset
    // and the appropriate callback has been issued to the caller. There's nothing to do here
    // except silently swallow this.
    // TODO: handle potential race condition with cancellation or failure get a stream in the
    // first place. Additionally it is possible to get a nullptr due to bogus stream_id
    // from the caller.
    // https://github.com/lyft/envoy-mobile/issues/301
    if (direct_stream != nullptr) {
      direct_stream->headers_ = Utility::transformHeaders(headers);
      ENVOY_LOG(debug, "[S{}] request headers for stream (end_stream={}):\n{}", stream_id,
                end_stream, *direct_stream->headers_);
      direct_stream->underlying_stream_.sendHeaders(*direct_stream->headers_, end_stream);
      closeLocal(stream_id, end_stream);
    }
  });

  return ENVOY_SUCCESS;
}

// TODO: implement.
envoy_status_t Dispatcher::sendData(envoy_stream_t, envoy_headers, bool) { return ENVOY_FAILURE; }
envoy_status_t Dispatcher::sendMetadata(envoy_stream_t, envoy_headers, bool) {
  return ENVOY_FAILURE;
}
envoy_status_t Dispatcher::sendTrailers(envoy_stream_t, envoy_headers) { return ENVOY_FAILURE; }

envoy_status_t Dispatcher::locallyCloseStream(envoy_stream_t stream_id) {
  event_dispatcher_.post([this, stream_id]() -> void {
    closeLocal(stream_id, true);
    ENVOY_LOG(debug, "[S{}] locally close stream", stream_id);
  });
  return ENVOY_SUCCESS;
}

envoy_status_t Dispatcher::resetStream(envoy_stream_t stream_id) {
  event_dispatcher_.post([this, stream_id]() -> void {
    closeLocal(stream_id, true);
    closeRemote(stream_id, true);
    ENVOY_LOG(debug, "[S{}] local reset stream", stream_id);
  });
  return ENVOY_SUCCESS;
}

Dispatcher::DirectStream* Dispatcher::getStream(envoy_stream_t stream_id) {
  ASSERT(event_dispatcher_.isThreadSafe(),
         "stream interaction must be performed on the event_dispatcher_'s thread.");
  auto direct_stream_pair_it = streams_.find(stream_id);
  return (direct_stream_pair_it != streams_.end()) ? direct_stream_pair_it->second.get() : nullptr;
}

void Dispatcher::closeLocal(envoy_stream_t stream_id, bool end_stream) {
  DirectStream* stream = getStream(stream_id);
  // FIXME: why would there be no stream?
  if (stream) {
    stream->local_closed_ = stream->local_closed_ || end_stream;
    ENVOY_LOG(debug, "[S{}] local close for stream (local_closed={} remote_closed={})", stream_id,
              stream->local_closed_, stream->remote_closed_);
    cleanup(*stream);
  }
}

void Dispatcher::closeRemote(envoy_stream_t stream_id, bool end_stream) {
  DirectStream* stream = getStream(stream_id);
  // FIXME: why would there be no stream?
  if (stream) {
    stream->remote_closed_ = stream->remote_closed_ || end_stream;
    ENVOY_LOG(debug, "[S{}] remote close for stream (local_closed={} remote_closed={})", stream_id,
              stream->local_closed_, stream->remote_closed_);
    cleanup(*stream);
  }
}

void Dispatcher::cleanup(DirectStream& stream) {
  ASSERT(event_dispatcher_.isThreadSafe(),
         "stream interaction must be performed on the event_dispatcher_'s thread.");
  ENVOY_LOG(debug, "[S{}] cleanup for stream (local_closed={} remote_closed={})", stream.stream_id_,
            stream.local_closed_, stream.remote_closed_);
  if (stream.local_closed_ && stream.remote_closed_) {
    removeStream(stream.stream_id_);
  }
}

void Dispatcher::removeStream(envoy_stream_t stream_id) {
  ASSERT(event_dispatcher_.isThreadSafe(),
         "stream interaction must be performed on the event_dispatcher_'s thread.");
  // FIXME do we need to worry about calling this with a stream id that is not present?
  size_t erased = streams_.erase(stream_id);
  if (erased > 0) {
    ENVOY_LOG(debug, "[S{}] remove stream", stream_id);
  }
}

} // namespace Http
} // namespace Envoy
