#pragma once

#include <atomic>
#include <unordered_map>

#include "envoy/buffer/buffer.h"
#include "envoy/event/dispatcher.h"
#include "envoy/http/async_client.h"
#include "envoy/http/header_map.h"
#include "envoy/upstream/cluster_manager.h"

#include "common/common/logger.h"

#include "library/common/include/c_types.h"

namespace Envoy {
namespace Http {

class Dispatcher : public Logger::Loggable<Logger::Id::http> {
public:
  Dispatcher(Event::Dispatcher& event_dispatcher, Upstream::ClusterManager& cluster_manager);

  envoy_stream_t startStream(envoy_observer);
  envoy_status_t sendHeaders(envoy_stream_t stream, envoy_headers headers, bool end_stream);
  envoy_status_t sendData(envoy_stream_t stream, envoy_headers headers, bool end_stream);
  envoy_status_t sendMetadata(envoy_stream_t stream, envoy_headers headers, bool end_stream);
  envoy_status_t sendTrailers(envoy_stream_t stream, envoy_headers headers);
  envoy_status_t locallyCloseStream(envoy_stream_t stream);
  envoy_status_t resetStream();
  envoy_status_t removeStream(envoy_stream_t stream_id);

private:
  class DirectStreamCallbacks : public AsyncClient::StreamCallbacks,
                                public Logger::Loggable<Logger::Id::http> {
  public:
    DirectStreamCallbacks(envoy_stream_t stream, envoy_observer observer,
                          Dispatcher& http_dispatcher);

    // AsyncClient::StreamCallbacks
    void onHeaders(HeaderMapPtr&& headers, bool end_stream);
    void onData(Buffer::Instance& data, bool end_stream);
    void onTrailers(HeaderMapPtr&& trailers);
    void onReset();

  private:
    envoy_stream_t stream_;
    envoy_observer observer_;
    Dispatcher& http_dispatcher_;
  };

  using DirectStreamCallbacksPtr = std::unique_ptr<DirectStreamCallbacks>;

  class DirectStream {
  public:
    // FIXME follow up on concerns about the lifetime of the stream pointer.
    DirectStream(AsyncClient::Stream* underlying_stream, DirectStreamCallbacksPtr callbacks);

    AsyncClient::Stream* underlying_stream_;
    DirectStreamCallbacksPtr callbacks_;

    HeaderMapPtr headers_;
    // TODO: because the client may send infinite metadata frames we need some ongoing way to
    // free metadata ahead of object destruction.
    // An implementation option would be to have drainable header maps, or done callbacks.
    std::vector<HeaderMapPtr> metadata_;
    HeaderMapPtr trailers_;
  };

  using DirectStreamPtr = std::unique_ptr<DirectStream>;

  // Everything in the below interface must only be accessed from the event_dispatcher's thread.
  // This allows us to generally avoid synchronization.
  DirectStream* getStream(envoy_stream_t stream_id);

  std::unordered_map<envoy_stream_t, DirectStreamPtr> streams_;
  envoy_stream_t current_stream_id_ = 0;
  // The event_dispatcher is the only member state that may be accessed from a thread other than
  // the event_dispatcher's own thread.
  Event::Dispatcher& event_dispatcher_;
  Upstream::ClusterManager& cluster_manager_;
};

} // namespace Http
} // namespace Envoy
