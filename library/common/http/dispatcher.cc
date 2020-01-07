#include "library/common/http/dispatcher.h"

#include "common/buffer/buffer_impl.h"
#include "common/common/lock_guard.h"
#include "common/http/headers.h"
#include "common/http/utility.h"

#include "library/common/buffer/bridge_fragment.h"
#include "library/common/buffer/utility.h"
#include "library/common/http/header_utility.h"
#include "library/common/network/synthetic_address_impl.h"

namespace Envoy {
namespace Http {

/**
 * IMPORTANT: stream closure semantics in envoy mobile depends on the fact that the HCM fires a
 * stream reset when the remote side of the stream is closed but the local side remains open.
 * In other words the HCM (like the rest of Envoy) dissallows locally half-open streams.
 * If this changes in Envoy, this file will need to change as well.
 * For implementation details @see Dispatcher::DirectStreamCallbacks::closeRemote.
 */

Dispatcher::DirectStreamCallbacks::DirectStreamCallbacks(DirectStream& direct_stream,
                                                         envoy_http_callbacks bridge_callbacks,
                                                         Dispatcher& http_dispatcher)
    : direct_stream_(direct_stream), bridge_callbacks_(bridge_callbacks),
      http_dispatcher_(http_dispatcher) {}

void Dispatcher::DirectStreamCallbacks::encodeHeaders(const HeaderMap& headers, bool end_stream) {
  ENVOY_LOG(debug, "[S{}] response headers for stream (end_stream={}):\n{}",
            direct_stream_.stream_handle_, end_stream, headers);
  // TODO: ***HACK*** currently Envoy sends local replies in cases where an error ought to be
  // surfaced via the error path. There are ways we can clean up Envoy's local reply path to
  // make this possible, but nothing expedient. For the immediate term this is our only real
  // option. See https://github.com/lyft/envoy-mobile/issues/460

  // The presence of EnvoyUpstreamServiceTime implies these headers are not due to a local reply.
  if (headers.get(Headers::get().EnvoyUpstreamServiceTime) != nullptr) {
    envoy_headers bridge_headers = Utility::toBridgeHeaders(headers);
    bridge_callbacks_.on_headers(bridge_headers, end_stream, bridge_callbacks_.context);
    closeRemote(end_stream);
    return;
  }

  uint64_t response_status = Http::Utility::getResponseStatus(headers);
  switch (response_status) {
  case 200: {
    // We still treat successful local responses as actual success.
    envoy_headers bridge_headers = Utility::toBridgeHeaders(headers);
    bridge_callbacks_.on_headers(bridge_headers, end_stream, bridge_callbacks_.context);
    closeRemote(end_stream);
    return;
  }
  case 503:
    error_code_ = ENVOY_CONNECTION_FAILURE;
    break;
  default:
    error_code_ = ENVOY_UNDEFINED_ERROR;
  }
  ENVOY_LOG(debug, "[S{}] intercepted local response", direct_stream_.stream_handle_);
  if (end_stream) {
    // The local stream may or may not have completed.
    // If the local is not closed envoy will fire the reset for us.
    // @see Dispatcher::DirectStreamCallbacks::closeRemote.
    // Otherwise fire the reset from here.
    if (direct_stream_.local_closed_) {
      onReset();
    }
  }
}

void Dispatcher::DirectStreamCallbacks::encodeData(Buffer::Instance& data, bool end_stream) {
  ENVOY_LOG(debug, "[S{}] response data for stream (length={} end_stream={})",
            direct_stream_.stream_handle_, data.length(), end_stream);
  if (!error_code_.has_value()) {
    bridge_callbacks_.on_data(Buffer::Utility::toBridgeData(data), end_stream,
                              bridge_callbacks_.context);
    closeRemote(end_stream);
  } else {
    // FIXME: should this be an ASSERT? Yes, if we are in the local response path, then Envoy
    // finishes the sequence with an encodeData. What if it later does trailers?
    ASSERT(end_stream);
    error_message_ = Buffer::Utility::toBridgeData(data);
    // The local stream may or may not have completed.
    // If the local is not closed envoy will fire the reset for us.
    // @see Dispatcher::DirectStreamCallbacks::closeRemote.
    // Otherwise fire the reset from here.
    if (direct_stream_.local_closed_) {
      onReset();
    }
  }
}

void Dispatcher::DirectStreamCallbacks::encodeTrailers(const HeaderMap& trailers) {
  ENVOY_LOG(debug, "[S{}] response trailers for stream:\n{}", direct_stream_.stream_handle_,
            trailers);
  bridge_callbacks_.on_trailers(Utility::toBridgeHeaders(trailers), bridge_callbacks_.context);
  closeRemote(true);
}

void Dispatcher::DirectStreamCallbacks::closeRemote(bool end_stream) {
  if (end_stream) {
    // Envoy itself does not currently allow half-open streams where the local half is open
    // but the remote half is closed. Therefore, we fire the on_complete callback
    // to the platform layer whenever remote closes.
    // To understand DirectStream cleanup @see Dispatcher::DirectStream::closeRemote().
    ENVOY_LOG(debug, "[S{}] complete stream", direct_stream_.stream_handle_);
    bridge_callbacks_.on_complete(bridge_callbacks_.context);
    direct_stream_.closeRemote(true);
  }
}

Stream& Dispatcher::DirectStreamCallbacks::getStream() { return direct_stream_; }

void Dispatcher::DirectStreamCallbacks::onReset() {
  ENVOY_LOG(debug, "[S{}] remote reset stream", direct_stream_.stream_handle_);
  envoy_error_code_t code = error_code_.value_or(ENVOY_STREAM_RESET);
  envoy_data message = error_message_.value_or(envoy_nodata);
  // Note: in the case that we received a complete remote response but envoy is resetting the stream
  // due to an incomplete local request this on_error call will happen after an on_complete.
  // FIXME: we still have to think these scenarios a bit more carefully.
  // FIXME: After some discussion of this we decided to
  // move atomic platform state, contracts, and cancellation down here.
  // Will do that ASAP after getting the HCM out of envoy.
  bridge_callbacks_.on_error({code, message}, bridge_callbacks_.context);
  // FIXME: this can't be right because then we could be potentially calling cleanup twice on a
  // stream.
  http_dispatcher_.cleanup(direct_stream_.stream_handle_);
}

Dispatcher::DirectStream::DirectStream(envoy_stream_t stream_handle, Dispatcher& http_dispatcher)
    : stream_handle_(stream_handle), parent_(http_dispatcher) {}

void Dispatcher::DirectStream::resetStream(StreamResetReason) { callbacks_->onReset(); }

void Dispatcher::DirectStream::closeLocal(bool end_stream) {
  // TODO: potentially guard against double local closure.
  local_closed_ = end_stream;

  // No cleanup happens here because cleanup always happens on remote closure.
  // see @Dispatcher::DirectStreamCallbacks::closeRemote.
  // Remote closure is guaranteed to happen either through the golden path, or through an Envoy
  // driven reset.
}
void Dispatcher::DirectStream::closeRemote(bool end_stream) {
  remote_closed_ = end_stream;
  // Even though we have potentially fired an on_complete callback to the bridge_callbacks,
  // We will only cleanup if **both** the local and the remote have closed because otherwise we
  // can be expecting a reset from envoy.
  // See the comment for @see Dispatcher::DirectStreamCallbacks::closeRemote().
  if ((complete())) {
    parent_.cleanup(stream_handle_);
  }
}

bool Dispatcher::DirectStream::complete() { return local_closed_ && remote_closed_; }

void Dispatcher::ready(Event::Dispatcher& event_dispatcher, ApiListener* api_listener) {
  Thread::LockGuard lock(dispatch_lock_);

  // Drain the init_queue_ into the event_dispatcher_.
  for (const Event::PostCb& cb : init_queue_) {
    event_dispatcher.post(cb);
  }

  // Ordering somewhat matters here if concurrency guarantees are loosened (e.g. if
  // we rely on atomics instead of locks).
  event_dispatcher_ = &event_dispatcher;
  api_listener_ = api_listener;
}

void Dispatcher::post(Event::PostCb callback) {
  Thread::LockGuard lock(dispatch_lock_);

  // If the event_dispatcher_ is set, then post the functor directly to it.
  if (event_dispatcher_ != nullptr) {
    event_dispatcher_->post(callback);
    return;
  }

  // Otherwise, push the functor to the init_queue_ which will be drained once the
  // event_dispatcher_ is ready.
  init_queue_.push_back(callback);
}

Dispatcher::Dispatcher(std::atomic<envoy_network_t>& preferred_network)
    : preferred_network_(preferred_network),
      address_(std::make_shared<Network::Address::SyntheticAddressImpl>()) {}

envoy_status_t Dispatcher::startStream(envoy_stream_t new_stream_handle,
                                       envoy_http_callbacks bridge_callbacks,
                                       envoy_stream_options) {
  post([this, new_stream_handle, bridge_callbacks]() -> void {
    DirectStreamPtr direct_stream{new DirectStream(new_stream_handle, *this)};
    direct_stream->callbacks_ =
        std::make_unique<DirectStreamCallbacks>(*direct_stream, bridge_callbacks, *this);

    // FIXME:
    // 1. Preferred network.
    // 2. Stream options -- buffering.
    preferred_network_.load();

    // Only the initial setting of the api_listener_ is guarded.
    direct_stream->stream_decoder_ =
        &TS_UNCHECKED_READ(api_listener_)->newStream(*direct_stream->callbacks_);
    ENVOY_LOG(error, "got stream decoder");

    streams_.emplace(new_stream_handle, std::move(direct_stream));
    ENVOY_LOG(debug, "[S{}] start stream", new_stream_handle);
  });

  return ENVOY_SUCCESS;
}

envoy_status_t Dispatcher::sendHeaders(envoy_stream_t stream, envoy_headers headers,
                                       bool end_stream) {
  post([this, stream, headers, end_stream]() -> void {
    DirectStream* direct_stream = getStream(stream);
    // If direct_stream is not found, it means the stream has already closed or been reset
    // and the appropriate callback has been issued to the caller. There's nothing to do here
    // except silently swallow this.
    // TODO: handle potential race condition with cancellation or failure get a stream in the
    // first place. Additionally it is possible to get a nullptr due to bogus envoy_stream_t
    // from the caller.
    // https://github.com/lyft/envoy-mobile/issues/301
    if (direct_stream != nullptr) {
      HeaderMapPtr internal_headers = Utility::toInternalHeaders(headers);
      ENVOY_LOG(debug, "[S{}] request headers for stream (end_stream={}):\n{}", stream, end_stream,
                *internal_headers);
      direct_stream->stream_decoder_->decodeHeaders(std::move(internal_headers), end_stream);
      direct_stream->closeLocal(end_stream);
    }
  });

  return ENVOY_SUCCESS;
}

envoy_status_t Dispatcher::sendData(envoy_stream_t stream, envoy_data data, bool end_stream) {
  post([this, stream, data, end_stream]() -> void {
    DirectStream* direct_stream = getStream(stream);
    // If direct_stream is not found, it means the stream has already closed or been reset
    // and the appropriate callback has been issued to the caller. There's nothing to do here
    // except silently swallow this.
    // TODO: handle potential race condition with cancellation or failure get a stream in the
    // first place. Additionally it is possible to get a nullptr due to bogus envoy_stream_t
    // from the caller.
    // https://github.com/lyft/envoy-mobile/issues/301
    if (direct_stream != nullptr) {
      // The buffer is moved internally, in a synchronous fashion, so we don't need the lifetime
      // of the InstancePtr to outlive this function call.
      Buffer::InstancePtr buf = Buffer::Utility::toInternalData(data);

      ENVOY_LOG(debug, "[S{}] request data for stream (length={} end_stream={})\n", stream,
                data.length, end_stream);
      direct_stream->stream_decoder_->decodeData(*buf, end_stream);
      direct_stream->closeLocal(end_stream);
    }
  });

  return ENVOY_SUCCESS;
}

// TODO: implement.
envoy_status_t Dispatcher::sendMetadata(envoy_stream_t, envoy_headers) { return ENVOY_FAILURE; }

envoy_status_t Dispatcher::sendTrailers(envoy_stream_t stream, envoy_headers trailers) {
  post([this, stream, trailers]() -> void {
    DirectStream* direct_stream = getStream(stream);
    // If direct_stream is not found, it means the stream has already closed or been reset
    // and the appropriate callback has been issued to the caller. There's nothing to do here
    // except silently swallow this.
    // TODO: handle potential race condition with cancellation or failure get a stream in the
    // first place. Additionally it is possible to get a nullptr due to bogus envoy_stream_t
    // from the caller.
    // https://github.com/lyft/envoy-mobile/issues/301
    if (direct_stream != nullptr) {
      HeaderMapPtr internal_trailers = Utility::toInternalHeaders(trailers);
      ENVOY_LOG(debug, "[S{}] request trailers for stream:\n{}", stream, *internal_trailers);
      direct_stream->stream_decoder_->decodeTrailers(std::move(internal_trailers));
      direct_stream->closeLocal(true);
    }
  });

  return ENVOY_SUCCESS;
}

envoy_status_t Dispatcher::resetStream(envoy_stream_t stream) {
  post([this, stream]() -> void {
    DirectStream* direct_stream = getStream(stream);
    if (direct_stream) {
      // FIXME discuss the enum case to use.
      // FIXME Cleanup at this layer because of a local cancellation.
      direct_stream->runResetCallbacks(StreamResetReason::RemoteReset);
    }
  });
  return ENVOY_SUCCESS;
}

Dispatcher::DirectStream* Dispatcher::getStream(envoy_stream_t stream) {
  // The dispatch_lock_ does not need to guard the event_dispatcher_ pointer here because this
  // function should only be called from the context of Envoy's event dispatcher.
  ASSERT(TS_UNCHECKED_READ(event_dispatcher_)->isThreadSafe(),
         "stream interaction must be performed on the event_dispatcher_'s thread.");
  auto direct_stream_pair_it = streams_.find(stream);
  return (direct_stream_pair_it != streams_.end()) ? direct_stream_pair_it->second.get() : nullptr;
}

void Dispatcher::cleanup(envoy_stream_t stream_handle) {
  DirectStream* direct_stream = getStream(stream_handle);
  RELEASE_ASSERT(direct_stream,
                 "cleanup is a private method that is only called with stream ids that exist");

  auto it = streams_.find(stream_handle);
  TS_UNCHECKED_READ(event_dispatcher_)->deferredDelete(std::move(it->second));
  size_t erased = streams_.erase(stream_handle);
  ASSERT(erased == 1, "cleanup should always remove one entry from the streams map");
  ENVOY_LOG(debug, "[S{}] remove stream", stream_handle);
}

} // namespace Http
} // namespace Envoy
