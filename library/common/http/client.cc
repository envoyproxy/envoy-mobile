#include "library/common/http/client.h"

#include "source/common/buffer/buffer_impl.h"
#include "source/common/common/dump_state_utils.h"
#include "source/common/common/scope_tracker.h"
#include "source/common/http/codes.h"
#include "source/common/http/header_map_impl.h"
#include "source/common/http/headers.h"
#include "source/common/http/utility.h"

#include "library/common/buffer/bridge_fragment.h"
#include "library/common/data/utility.h"
#include "library/common/http/header_utility.h"
#include "library/common/http/headers.h"

namespace Envoy {
namespace Http {

/**
 * IMPORTANT: stream closure semantics in envoy mobile depends on the fact that the HCM fires a
 * stream reset when the remote side of the stream is closed but the local side remains open.
 * In other words the HCM (like the rest of Envoy) dissallows locally half-open streams.
 * If this changes in Envoy, this file will need to change as well.
 * For implementation details @see Client::DirectStreamCallbacks::closeRemote.
 */

Client::DirectStreamCallbacks::DirectStreamCallbacks(DirectStream& direct_stream,
                                                     envoy_http_callbacks bridge_callbacks,
                                                     Client& http_client)
    : direct_stream_(direct_stream), bridge_callbacks_(bridge_callbacks), http_client_(http_client),
      explicit_flow_control_(http_client_.explicit_flow_control_) {}

void Client::DirectStreamCallbacks::encodeHeaders(const ResponseHeaderMap& headers,
                                                  bool end_stream) {
  ScopeTrackerScopeState scope(&direct_stream_, http_client_.scopeTracker());
  ENVOY_LOG(debug, "[S{}] response headers for stream (end_stream={}):\n{}",
            direct_stream_.stream_handle_, end_stream, headers);

  ASSERT(http_client_.getStream(direct_stream_.stream_handle_,
                                GetStreamFilters::ALLOW_FOR_ALL_STREAMS));
  if (end_stream) {
    closeStream();
  }

  uint64_t response_status = Utility::getResponseStatus(headers);

  // Presence of internal error header indicates an error that should be surfaced as an
  // error callback (rather than an HTTP response).
  const auto error_code_header = headers.get(InternalHeaders::get().ErrorCode);
  if (!error_code_header.empty()) {
    envoy_error_code_t error_code;
    bool check = absl::SimpleAtoi(error_code_header[0]->value().getStringView(), &error_code);
    RELEASE_ASSERT(
        check, fmt::format("[S{}] parse error reading error code", direct_stream_.stream_handle_));
    error_code_ = error_code;

    const auto error_message_header = headers.get(InternalHeaders::get().ErrorMessage);
    if (!error_message_header.empty()) {
      error_message_ =
          Data::Utility::copyToBridgeData(error_message_header[0]->value().getStringView());
    }

    uint32_t attempt_count = 1;
    if (headers.EnvoyAttemptCount()) {
      RELEASE_ASSERT(
          absl::SimpleAtoi(headers.EnvoyAttemptCount()->value().getStringView(), &attempt_count),
          "parse error reading attempt count");
    }
    error_attempt_count_ = attempt_count;

    if (end_stream) {
      onError();
    }
    return;
  }

  // Track success for later bookkeeping (stream could still be reset).
  success_ = CodeUtility::is2xx(response_status);

  ENVOY_LOG(debug, "[S{}] dispatching to platform response headers for stream (end_stream={}):\n{}",
            direct_stream_.stream_handle_, end_stream, headers);
  bridge_callbacks_.on_headers(Utility::toBridgeHeaders(headers), end_stream,
                               bridge_callbacks_.context);
  response_headers_forwarded_ = true;
  if (end_stream) {
    onComplete();
  }
}

uint32_t calculateBytesToSend(const Buffer::Instance& data, uint32_t max_bytes) {
  if (max_bytes == 0) {
    return data.length();
  }
  return std::min<uint32_t>(max_bytes, data.length());
}

void Client::DirectStreamCallbacks::encodeData(Buffer::Instance& data, bool end_stream) {
  ScopeTrackerScopeState scope(&direct_stream_, http_client_.scopeTracker());
  ENVOY_LOG(debug, "[S{}] response data for stream (length={} end_stream={})",
            direct_stream_.stream_handle_, data.length(), end_stream);

  ASSERT(http_client_.getStream(direct_stream_.stream_handle_,
                                GetStreamFilters::ALLOW_FOR_ALL_STREAMS));
  if (end_stream) {
    closeStream();
  }

  if (error_code_) {
    onError();
    return;
  }

  // Send data if in default flow control mode, or if resumeData has been called in explicit
  // flow control mode.
  if (bytes_to_send_ > 0 || !explicit_flow_control_) {
    ASSERT(!hasBufferedData());
    sendDataToBridge(data, end_stream);
  }

  // If not all the bytes have been sent up, buffer any remaining data in response_data.
  if (data.length() != 0) {
    ASSERT(explicit_flow_control_);
    if (!response_data_) {
      response_data_ = std::make_unique<Buffer::WatermarkBuffer>(
          [this]() -> void { this->onBufferedDataDrained(); },
          [this]() -> void { this->onHasBufferedData(); }, []() -> void {});
      // Default to 1M per stream. This is fairly arbitrary and will result in
      // Envoy buffering up to 1M + flow-control-window for HTTP/2 and HTTP/3,
      // and having local data of 1M + kernel-buffer-limit for HTTP/1.1
      response_data_->setWatermarks(1000000);
    }
    ENVOY_LOG(
        debug, "[S{}] buffering {} bytes due to explicit flow control. {} total bytes buffered.",
        direct_stream_.stream_handle_, data.length(), data.length() + response_data_->length());
    response_data_->move(data);
  }
}

void Client::DirectStreamCallbacks::sendDataToBridge(Buffer::Instance& data, bool end_stream) {
  ASSERT(!explicit_flow_control_ || bytes_to_send_ > 0);

  // Cap by bytes_to_send_ if and only if applying explicit flow control.
  uint32_t bytes_to_send = calculateBytesToSend(data, bytes_to_send_);
  // Only send end stream if all data is being sent.
  bool send_end_stream = end_stream && (bytes_to_send == data.length());

  ENVOY_LOG(debug,
            "[S{}] dispatching to platform response data for stream (length={} end_stream={})",
            direct_stream_.stream_handle_, bytes_to_send, send_end_stream);

  bridge_callbacks_.on_data(Data::Utility::toBridgeData(data, bytes_to_send), end_stream,
                            bridge_callbacks_.context);
  if (send_end_stream) {
    onComplete();
  }
  // Make sure that when using explicit flow control this won't send more data until the next call
  // to resumeData.
  bytes_to_send_ = 0;
}

void Client::DirectStreamCallbacks::encodeTrailers(const ResponseTrailerMap& trailers) {
  ScopeTrackerScopeState scope(&direct_stream_, http_client_.scopeTracker());
  ENVOY_LOG(debug, "[S{}] response trailers for stream:\n{}", direct_stream_.stream_handle_,
            trailers);

  ASSERT(http_client_.getStream(direct_stream_.stream_handle_,
                                GetStreamFilters::ALLOW_FOR_ALL_STREAMS));
  closeStream(); // Trailers always indicate the end of the stream.

  // For explicit flow control, don't send data unless prompted.
  if (explicit_flow_control_ && bytes_to_send_ == 0) {
    response_trailers_ = ResponseTrailerMapImpl::create();
    HeaderMapImpl::copyFrom(*response_trailers_, trailers);
    return;
  }

  sendTrailersToBridge(trailers);
}

void Client::DirectStreamCallbacks::sendTrailersToBridge(const ResponseTrailerMap& trailers) {
  ENVOY_LOG(debug, "[S{}] dispatching to platform response trailers for stream:\n{}",
            direct_stream_.stream_handle_, trailers);

  bridge_callbacks_.on_trailers(Utility::toBridgeHeaders(trailers), bridge_callbacks_.context);
  onComplete();
}

void Client::DirectStreamCallbacks::resumeData(int32_t bytes_to_send) {
  ASSERT(explicit_flow_control_);
  ASSERT(bytes_to_send > 0);

  bytes_to_send_ = bytes_to_send;

  ENVOY_LOG(debug, "[S{}] received resume data call for {} bytes", direct_stream_.stream_handle_,
            bytes_to_send_);

  // If there is buffered data, send up to bytes_to_send bytes.
  // Make sure to send end stream with data only if
  // 1) it has been received from the peer and
  // 2) there are no trailers
  if (hasBufferedData() ||
      (remote_end_stream_received_ && !remote_end_stream_forwarded_ && !response_trailers_)) {
    sendDataToBridge(*response_data_, remote_end_stream_received_ && !response_trailers_.get());
    bytes_to_send_ = 0;
  }

  // If all buffered data has been sent, send and free up trailers.
  if (!hasBufferedData() && response_trailers_.get() && bytes_to_send_ > 0) {
    sendTrailersToBridge(*response_trailers_);
    response_trailers_.reset();
    bytes_to_send_ = 0;
  }

  if (error_code_.has_value() && bytes_to_send_ > 0) {
    onError();
  }
}

void Client::DirectStreamCallbacks::closeStream() {
  remote_end_stream_received_ = true;

  auto& client = direct_stream_.parent_;
  auto stream = client.getStream(direct_stream_.stream_handle_, ALLOW_ONLY_FOR_OPEN_STREAMS);
  ASSERT(stream != nullptr);
  if (stream) {
    client.closed_streams_.emplace(direct_stream_.stream_handle_, std::move(stream));
    size_t erased = client.streams_.erase(direct_stream_.stream_handle_);
    ASSERT(erased == 1, "closeStream should always remove one entry from the streams map");
  }
}

void Client::DirectStreamCallbacks::onComplete() {
  http_client_.removeStream(direct_stream_.stream_handle_);
  remote_end_stream_forwarded_ = true;
  ENVOY_LOG(debug, "[S{}] complete stream (success={})", direct_stream_.stream_handle_, success_);
  if (success_) {
    http_client_.stats().stream_success_.inc();
  } else {
    http_client_.stats().stream_failure_.inc();
  }
  bridge_callbacks_.on_complete(bridge_callbacks_.context);
}

void Client::DirectStreamCallbacks::onError() {
  ScopeTrackerScopeState scope(&direct_stream_, http_client_.scopeTracker());
  ENVOY_LOG(debug, "[S{}] remote reset stream", direct_stream_.stream_handle_);

  // When using explicit flow control, if any response data has been sent (e.g. headers), response
  // errors must be deferred until after resumeData has been called.
  if (explicit_flow_control_ && response_headers_forwarded_ && bytes_to_send_ == 0) {
    return;
  }

  http_client_.removeStream(direct_stream_.stream_handle_);
  // The stream should no longer be preset in the map, because onError() was either called from a
  // terminal callback that mapped to an error or it was called in response to a resetStream().
  ASSERT(!http_client_.getStream(direct_stream_.stream_handle_,
                                 GetStreamFilters::ALLOW_FOR_ALL_STREAMS));
  envoy_error_code_t code = error_code_.value_or(ENVOY_STREAM_RESET);
  envoy_data message = error_message_.value_or(envoy_nodata);
  int32_t attempt_count = error_attempt_count_.value_or(-1);

  ENVOY_LOG(debug, "[S{}] dispatching to platform remote reset stream",
            direct_stream_.stream_handle_);
  http_client_.stats().stream_failure_.inc();

  error_code_ = {};
  error_message_ = {};
  error_attempt_count_ = {};

  bridge_callbacks_.on_error({code, message, attempt_count}, bridge_callbacks_.context);
}

void Client::DirectStreamCallbacks::onCancel() {
  ScopeTrackerScopeState scope(&direct_stream_, http_client_.scopeTracker());
  ENVOY_LOG(debug, "[S{}] dispatching to platform cancel stream", direct_stream_.stream_handle_);
  http_client_.stats().stream_cancel_.inc();
  bridge_callbacks_.on_cancel(bridge_callbacks_.context);
}

Client::DirectStream::DirectStream(envoy_stream_t stream_handle, Client& http_client)
    : stream_handle_(stream_handle), parent_(http_client) {}

Client::DirectStream::~DirectStream() { ENVOY_LOG(debug, "[S{}] destroy stream", stream_handle_); }

void Client::DirectStream::resetStream(StreamResetReason reason) {
  // This seems in line with other codec implementations, and so the assumption is that this is in
  // line with upstream expectations.
  // TODO(goaway): explore an upstream fix to get the HCM to clean up ActiveStream itself.
  runResetCallbacks(reason);
  if (!parent_.getStream(stream_handle_, GetStreamFilters::ALLOW_FOR_ALL_STREAMS)) {
    // We don't assert here, because Envoy will issue a stream reset if a stream closes remotely
    // while still open locally. In this case the stream will already have been removed from
    // our stream maps due to the remote closure.
    return;
  }
  callbacks_->onError();
}

void Client::DirectStream::dumpState(std::ostream&, int indent_level) const {
  // TODO(junr03): output to ostream arg - https://github.com/envoyproxy/envoy-mobile/issues/1497.
  std::stringstream ss;
  const char* spaces = spacesForLevel(indent_level);

  ss << spaces << "DirectStream" << DUMP_MEMBER(stream_handle_) << std::endl;
  ENVOY_LOG(error, "\n{}", ss.str());
}

void Client::startStream(envoy_stream_t new_stream_handle, envoy_http_callbacks bridge_callbacks) {
  ASSERT(dispatcher_.isThreadSafe());
  Client::DirectStreamSharedPtr direct_stream{new DirectStream(new_stream_handle, *this)};
  direct_stream->callbacks_ =
      std::make_unique<DirectStreamCallbacks>(*direct_stream, bridge_callbacks, *this);

  // Note: streams created by Envoy Mobile are tagged as is_internally_created. This means that
  // the Http::ConnectionManager _will not_ sanitize headers when creating a stream.
  direct_stream->request_decoder_ =
      &api_listener_.newStream(*direct_stream->callbacks_, true /* is_internally_created */);

  streams_.emplace(new_stream_handle, std::move(direct_stream));
  ENVOY_LOG(debug, "[S{}] start stream", new_stream_handle);
}

void Client::sendHeaders(envoy_stream_t stream, envoy_headers headers, bool end_stream) {
  ASSERT(dispatcher_.isThreadSafe());
  Client::DirectStreamSharedPtr direct_stream =
      getStream(stream, GetStreamFilters::ALLOW_ONLY_FOR_OPEN_STREAMS);
  // If direct_stream is not found, it means the stream has already closed or been reset
  // and the appropriate callback has been issued to the caller. There's nothing to do here
  // except silently swallow this.
  // TODO: handle potential race condition with cancellation or failure get a stream in the
  // first place. Additionally it is possible to get a nullptr due to bogus envoy_stream_t
  // from the caller.
  // https://github.com/lyft/envoy-mobile/issues/301
  if (direct_stream) {
    ScopeTrackerScopeState scope(direct_stream.get(), scopeTracker());
    RequestHeaderMapPtr internal_headers = Utility::toRequestHeaders(headers);
    // The second argument here specifies whether to use an 'alternate' cluster (see discussion
    // below in cluster definitions). Random selection avoids determinism resulting from consistent
    // patterns in, e.g., mobile application flows.
    setDestinationCluster(*internal_headers, random_.random() % 2);
    // Set the x-forwarded-proto header to https because Envoy Mobile only has clusters with TLS
    // enabled. This is done here because the ApiListener's synthetic connection would make the
    // Http::ConnectionManager set the scheme to http otherwise. In the future we might want to
    // configure the connection instead of setting the header here.
    // https://github.com/envoyproxy/envoy/issues/10291
    //
    // Setting this header is also currently important because Envoy Mobile starts stream with the
    // ApiListener setting the is_internally_created bool to true. This means the
    // Http::ConnectionManager *will not* mutate Envoy Mobile's request headers. One of the
    // mutations done is adding the x-forwarded-proto header if not present. Unfortunately, the
    // router relies on the present of this header to determine if it should provided a route for
    // a request here:
    // https://github.com/envoyproxy/envoy/blob/c9e3b9d2c453c7fe56a0e3615f0c742ac0d5e768/source/common/router/config_impl.cc#L1091-L1096
    internal_headers->setReferenceForwardedProto(Headers::get().SchemeValues.Https);
    ENVOY_LOG(debug, "[S{}] request headers for stream (end_stream={}):\n{}", stream, end_stream,
              *internal_headers);
    direct_stream->request_decoder_->decodeHeaders(std::move(internal_headers), end_stream);
  }
}

void Client::sendData(envoy_stream_t stream, envoy_data data, bool end_stream) {
  ASSERT(dispatcher_.isThreadSafe());
  Client::DirectStreamSharedPtr direct_stream =
      getStream(stream, GetStreamFilters::ALLOW_ONLY_FOR_OPEN_STREAMS);
  // If direct_stream is not found, it means the stream has already closed or been reset
  // and the appropriate callback has been issued to the caller. There's nothing to do here
  // except silently swallow this.
  // TODO: handle potential race condition with cancellation or failure get a stream in the
  // first place. Additionally it is possible to get a nullptr due to bogus envoy_stream_t
  // from the caller.
  // https://github.com/lyft/envoy-mobile/issues/301
  if (direct_stream) {
    ScopeTrackerScopeState scope(direct_stream.get(), scopeTracker());
    // The buffer is moved internally, in a synchronous fashion, so we don't need the lifetime
    // of the InstancePtr to outlive this function call.
    Buffer::InstancePtr buf = Data::Utility::toInternalData(data);

    ENVOY_LOG(debug, "[S{}] request data for stream (length={} end_stream={})\n", stream,
              data.length, end_stream);
    direct_stream->request_decoder_->decodeData(*buf, end_stream);
  }
}

void Client::sendMetadata(envoy_stream_t, envoy_headers) { NOT_IMPLEMENTED_GCOVR_EXCL_LINE; }

void Client::sendTrailers(envoy_stream_t stream, envoy_headers trailers) {
  ASSERT(dispatcher_.isThreadSafe());
  Client::DirectStreamSharedPtr direct_stream =
      getStream(stream, GetStreamFilters::ALLOW_ONLY_FOR_OPEN_STREAMS);
  // If direct_stream is not found, it means the stream has already closed or been reset
  // and the appropriate callback has been issued to the caller. There's nothing to do here
  // except silently swallow this.
  // TODO: handle potential race condition with cancellation or failure get a stream in the
  // first place. Additionally it is possible to get a nullptr due to bogus envoy_stream_t
  // from the caller.
  // https://github.com/lyft/envoy-mobile/issues/301
  if (direct_stream) {
    ScopeTrackerScopeState scope(direct_stream.get(), scopeTracker());
    RequestTrailerMapPtr internal_trailers = Utility::toRequestTrailers(trailers);
    ENVOY_LOG(debug, "[S{}] request trailers for stream:\n{}", stream, *internal_trailers);
    direct_stream->request_decoder_->decodeTrailers(std::move(internal_trailers));
  }
}

void Client::cancelStream(envoy_stream_t stream) {
  ASSERT(dispatcher_.isThreadSafe());
  // This is the one place where downstream->upstream communication is allowed
  // for closed streams: if the client cancels the stream it should be canceled
  // whether it was closed or not.
  Client::DirectStreamSharedPtr direct_stream =
      getStream(stream, GetStreamFilters::ALLOW_FOR_ALL_STREAMS);
  if (direct_stream) {
    ScopeTrackerScopeState scope(direct_stream.get(), scopeTracker());
    removeStream(direct_stream->stream_handle_);

    direct_stream->callbacks_->onCancel();

    // Since https://github.com/envoyproxy/envoy/pull/13052, the connection manager expects that
    // response code details are set on all possible paths for streams.
    direct_stream->setResponseDetails(getCancelDetails());

    // The runResetCallbacks call synchronously causes Envoy to defer delete the HCM's
    // ActiveStream. We have some concern that this could potentially race a terminal callback
    // scheduled on the same iteration of the event loop. If we see violations in the callback
    // assertions checking stream presence, this is a likely potential culprit. However, it's
    // plausible that upstream guards will protect us here, given that Envoy allows streams to be
    // reset from a wide variety of contexts without apparent issue.
    direct_stream->runResetCallbacks(StreamResetReason::RemoteReset);
  }
}

const HttpClientStats& Client::stats() const { return stats_; }

Client::DirectStreamSharedPtr Client::getStream(envoy_stream_t stream,
                                                GetStreamFilters get_stream_filters) {
  auto direct_stream_pair_it = streams_.find(stream);
  if (direct_stream_pair_it != streams_.end()) {
    return direct_stream_pair_it->second;
  }
  if (direct_stream_pair_it == streams_.end() && get_stream_filters == ALLOW_FOR_ALL_STREAMS) {
    direct_stream_pair_it = closed_streams_.find(stream);
    if (direct_stream_pair_it != closed_streams_.end()) {
      return direct_stream_pair_it->second;
    }
  }
  return nullptr;
}

void Client::removeStream(envoy_stream_t stream_handle) {
  RELEASE_ASSERT(
      dispatcher_.isThreadSafe(),
      fmt::format("[S{}] stream removeStream must be performed on the dispatcher_'s thread.",
                  stream_handle));
  Client::DirectStreamSharedPtr direct_stream =
      getStream(stream_handle, GetStreamFilters::ALLOW_FOR_ALL_STREAMS);
  RELEASE_ASSERT(
      direct_stream,
      fmt::format(
          "[S{}] removeStream is a private method that is only called with stream ids that exist",
          stream_handle));

  // The DirectStream should live through synchronous code that already has a reference to it.
  // Hence why it is scheduled for deferred deletion. If this was all that was needed then it
  // would be sufficient to return a shared_ptr in getStream. However, deferred deletion is still
  // required because in Client::resetStream the DirectStream needs to live for as long and
  // the HCM's ActiveStream lives. Hence its deletion needs to live beyond the synchronous code in
  // Client::resetStream.
  auto direct_stream_wrapper = std::make_unique<DirectStreamWrapper>(std::move(direct_stream));
  dispatcher_.deferredDelete(std::move(direct_stream_wrapper));
  // However, the entry in the map should not exist after removeStream.
  // Hence why it is synchronously erased from the streams map.
  size_t erased = streams_.erase(stream_handle);
  if (erased != 1) {
    erased = closed_streams_.erase(stream_handle);
  }
  ASSERT(erased == 1, "removeStream should always remove one entry from the streams map");
  ENVOY_LOG(debug, "[S{}] erased stream from streams container", stream_handle);
}

namespace {

const LowerCaseString ClusterHeader{"x-envoy-mobile-cluster"};
const LowerCaseString H2UpstreamHeader{"x-envoy-mobile-upstream-protocol"};

// Alternate clusters included here are a stopgap to make it less likely for a given connection
// class to suffer "catastrophic" failure of all outbound requests due to a network blip, by
// distributing requests across a minimum of two potential connections per connection class.
// Long-term we will be working to generally provide more responsive connection handling within
// Envoy itself.

const char* BaseClusters[][3] = {{
                                     "base",
                                     "base_wlan",
                                     "base_wwan",
                                 },
                                 {
                                     "base_alt",
                                     "base_wlan_alt",
                                     "base_wwan_alt",
                                 }};

const char* H2Clusters[][3] = {{
                                   "base_h2",
                                   "base_wlan_h2",
                                   "base_wwan_h2",
                               },
                               {
                                   "base_h2_alt",
                                   "base_wlan_h2_alt",
                                   "base_wwan_h2_alt",
                               }};

const char* ClearTextClusters[][3] = {{
                                          "base_clear",
                                          "base_wlan_clear",
                                          "base_wwan_clear",
                                      },
                                      {
                                          "base_clear_alt",
                                          "base_wlan_clear_alt",
                                          "base_wwan_clear_alt",
                                      }};

} // namespace

void Client::setDestinationCluster(Http::RequestHeaderMap& headers, bool alternate) {
  // Determine upstream cluster:
  // - Use TLS by default.
  // - Use http/2 if requested explicitly via x-envoy-mobile-upstream-protocol.
  // - Force http/1.1 if request scheme is http (cleartext).
  const char* cluster{};
  auto h2_header = headers.get(H2UpstreamHeader);
  auto network = preferred_network_.load();
  ASSERT(network >= 0 && network < 3, "preferred_network_ must be valid index into cluster array");

  if (headers.getSchemeValue() == Headers::get().SchemeValues.Http) {
    cluster = ClearTextClusters[alternate][network];
  } else if (!h2_header.empty()) {
    ASSERT(h2_header.size() == 1);
    const auto value = h2_header[0]->value().getStringView();
    if (value == "http2") {
      cluster = H2Clusters[alternate][network];
    } else {
      RELEASE_ASSERT(value == "http1", fmt::format("using unsupported protocol version {}", value));
      cluster = BaseClusters[alternate][network];
    }
  } else {
    cluster = BaseClusters[alternate][network];
  }

  if (!h2_header.empty()) {
    headers.remove(H2UpstreamHeader);
  }

  headers.addCopy(ClusterHeader, std::string{cluster});
}

} // namespace Http
} // namespace Envoy
