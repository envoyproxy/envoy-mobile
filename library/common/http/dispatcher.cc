#include "library/common/http/dispatcher.h"

#include "common/buffer/buffer_impl.h"
#include "common/common/lock_guard.h"
#include "common/http/headers.h"
#include "common/http/utility.h"

#include "library/common/buffer/bridge_fragment.h"
#include "library/common/buffer/utility.h"
#include "library/common/http/header_utility.h"

namespace Envoy {
namespace Http {

Dispatcher::DirectStreamCallbacks::DirectStreamCallbacks(envoy_stream_t stream,
                                                         envoy_http_callbacks bridge_callbacks,
                                                         Dispatcher& http_dispatcher)
    : stream_handle_(stream), bridge_callbacks_(bridge_callbacks),
      http_dispatcher_(http_dispatcher) {}

void Dispatcher::DirectStreamCallbacks::encodeHeaders(const HeaderMap& headers, bool end_stream) {
  ENVOY_LOG(debug, "[S{}] response headers for stream (end_stream={}):\n{}", stream_handle_,
            end_stream, headers);
  // FIXME FIXME
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

  // FIXME
  // We assume that local replies represent error conditions, having audited occurrences in
  // Envoy today. This is not a good long-term solution.
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
  ENVOY_LOG(debug, "[S{}] intercepted local response", stream_handle_);
  if (end_stream) {
    // The local stream may or may not have completed. We don't want to be tracking/synchronized
    // on that state, so we just reset everything now to ensure teardown.
    auto stream = http_dispatcher_.getStream(stream_handle_);
    ASSERT(stream);
    // FIXME
    // stream->underlying_stream_.reset();
  }
}

void Dispatcher::DirectStreamCallbacks::encodeData(Buffer::Instance& data, bool end_stream) {
  ENVOY_LOG(debug, "[S{}] response data for stream (length={} end_stream={})", stream_handle_,
            data.length(), end_stream);
  if (!error_code_.has_value()) {
    bridge_callbacks_.on_data(Buffer::Utility::toBridgeData(data), end_stream,
                              bridge_callbacks_.context);
    closeRemote(end_stream);
  } else {
    ASSERT(end_stream);
    error_message_ = Buffer::Utility::toBridgeData(data);
    // The local stream may or may not have completed. We don't want to be tracking/synchronized on
    // that state, so we just reset everything now to ensure teardown.
    auto stream = http_dispatcher_.getStream(stream_handle_);
    ASSERT(stream);
    // FIXME
    // stream->underlying_stream_.reset();
  }
}

void Dispatcher::DirectStreamCallbacks::encodeTrailers(const HeaderMap& trailers) {
  ENVOY_LOG(debug, "[S{}] response trailers for stream:\n{}", stream_handle_, trailers);
  bridge_callbacks_.on_trailers(Utility::toBridgeHeaders(trailers), bridge_callbacks_.context);
  closeRemote(true);
}

// FIXME the other option is to make the DirectStreamCallbacks aware of the DirectStream.
void Dispatcher::DirectStreamCallbacks::closeRemote(bool end_stream) {
  if (end_stream) {
    auto stream = http_dispatcher_.getStream(stream_handle_);
    ASSERT(stream);
    stream->closeRemote(true);
  }
}

Stream& Dispatcher::DirectStreamCallbacks::getStream() {
  auto stream = http_dispatcher_.getStream(stream_handle_);
  // The stream owns the callbacks.
  // Hence if this code is executing the stream must still exist.
  ASSERT(stream);
  return *stream;
}

// FIXME
// void Dispatcher::DirectStreamCallbacks::onComplete() {
//   ENVOY_LOG(debug, "[S{}] complete stream", stream_handle_);
//   bridge_callbacks_.on_complete(bridge_callbacks_.context);
//   // Very important: onComplete and onReset both clean up stream state in the http dispatcher
//   // because the underlying async client implementation **guarantees** that only onComplete
//   **or**
//   // onReset will be fired for a stream. This means it is safe to clean up the stream when either
//   of
//   // the terminal callbacks fire without keeping additional state in this layer.
//   http_dispatcher_.cleanup(stream_handle_);
// }

void Dispatcher::DirectStreamCallbacks::onReset() {
  ENVOY_LOG(debug, "[S{}] remote reset stream", stream_handle_);
  envoy_error_code_t code = error_code_.value_or(ENVOY_STREAM_RESET);
  envoy_data message = error_message_.value_or(envoy_nodata);
  bridge_callbacks_.on_error({code, message}, bridge_callbacks_.context);
  // Very important: onComplete and onReset both clean up stream state in the http dispatcher
  // because the underlying async client implementation **guarantees** that only onComplete
  // **or**
  // onReset will be fired for a stream. This means it is safe to clean up the stream when either
  // of
  // the terminal callbacks fire without keeping additional state in this layer.
  http_dispatcher_.cleanup(stream_handle_);
}

Dispatcher::DirectStream::DirectStream(envoy_stream_t stream_handle, StreamDecoder& stream_decoder,
                                       DirectStreamCallbacksPtr&& callbacks,
                                       Dispatcher& http_dispatcher)
    : stream_handle_(stream_handle), stream_decoder_(stream_decoder),
      callbacks_(std::move(callbacks)), parent_(http_dispatcher) {}

// FIXME, the stream reset reason will always be local. Confirm this.
// The router will fire a chain of events that ends up surfacing here.
void Dispatcher::DirectStream::resetStream(StreamResetReason) { callbacks_->onReset(); }

void Dispatcher::DirectStream::closeLocal(bool end_stream) {
  local_end_stream_ = end_stream;
  if (complete()) {
    parent_.cleanup(stream_handle_);
  }
}
void Dispatcher::DirectStream::closeRemote(bool end_stream) {
  remote_end_stream_ = end_stream;
  if ((complete())) {
    parent_.cleanup(stream_handle_);
  }
}

bool Dispatcher::DirectStream::complete() { return local_end_stream_ && remote_end_stream_; }

void Dispatcher::ready(Event::Dispatcher& event_dispatcher,
                       ServerConnectionCallbacks& conn_manager) {
  Thread::LockGuard lock(dispatch_lock_);

  // Drain the init_queue_ into the event_dispatcher_.
  for (const Event::PostCb& cb : init_queue_) {
    event_dispatcher.post(cb);
  }

  // Ordering somewhat matters here if concurrency guarantees are loosened (e.g. if
  // we rely on atomics instead of locks).
  event_dispatcher_ = &event_dispatcher;
  conn_manager_ = &conn_manager;
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
    : preferred_network_(preferred_network) {}

envoy_status_t Dispatcher::startStream(envoy_stream_t new_stream_handle,
                                       envoy_http_callbacks bridge_callbacks,
                                       envoy_stream_options) {
  post([this, new_stream_handle, bridge_callbacks]() -> void {
    DirectStreamCallbacksPtr callbacks =
        std::make_unique<DirectStreamCallbacks>(new_stream_handle, bridge_callbacks, *this);

    // TODO:
    // 1. Preferred network.
    // 2. Stream options -- buffering.
    preferred_network_.load();

    // Only the initial setting of the conn_manager_ is guarded.
    StreamDecoder& stream_decoder = TS_UNCHECKED_READ(conn_manager_)->newStream(*callbacks);

    DirectStreamPtr direct_stream = std::make_unique<DirectStream>(
        new_stream_handle, stream_decoder, std::move(callbacks), *this);
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
      direct_stream->stream_decoder_.decodeHeaders(std::move(internal_headers), end_stream);
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
      // The buffer is moved internally, in a synchronous fashion, so we don't need the lifetime of
      // the InstancePtr to outlive this function call.
      Buffer::InstancePtr buf = Buffer::Utility::toInternalData(data);

      ENVOY_LOG(debug, "[S{}] request data for stream (length={} end_stream={})\n", stream,
                data.length, end_stream);
      direct_stream->stream_decoder_.decodeData(*buf, end_stream);
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
      direct_stream->stream_decoder_.decodeTrailers(std::move(internal_trailers));
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

// FIXME consider calling cleanup with a stream ref rather than doing a lookup again!
void Dispatcher::cleanup(envoy_stream_t stream_handle) {
  DirectStream* direct_stream = getStream(stream_handle);
  RELEASE_ASSERT(direct_stream,
                 "cleanup is a private method that is only called with stream ids that exist");

  size_t erased = streams_.erase(stream_handle);
  ASSERT(erased == 1, "cleanup should always remove one entry from the streams map");
  ENVOY_LOG(debug, "[S{}] remove stream", stream_handle);
}

} // namespace Http
} // namespace Envoy
