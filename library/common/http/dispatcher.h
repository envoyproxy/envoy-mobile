#pragma once

#include <atomic>
#include <unordered_map>

#include "envoy/buffer/buffer.h"
#include "envoy/http/async_client.h"
#include "envoy/http/header_map.h"

#include "common/common/logger.h"

#include "library/common/include/c_types.h"

namespace Envoy {
namespace Http {

// FIXME think about the lifetime of the AsyncStream pointers we are storing in the streams_ map.
// Also about the lifetime of the http_dispatcher.

class Dispatcher;

class DirectStreamCallbacks : public AsyncClient::StreamCallbacks {
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

using DirectStream = std::pair<DirectStreamCallbacksPtr, AsyncClient::Stream*>;

class Dispatcher : public Logger::Loggable<Logger::Id::http> {
public:
  Dispatcher(Event::Dispatcher& event_dispatcher);

  envoy_stream_t startStream(envoy_observer);
  envoy_status_t sendHeaders(envoy_stream_t stream, envoy_headers headers, bool end_stream);
  envoy_status_t sendData(envoy_stream_t stream, envoy_headers headers, bool end_stream);
  envoy_status_t sendMetadata(envoy_stream_t stream, envoy_headers headers, bool end_stream);
  envoy_status_t sendTrailers(envoy_stream_t stream, envoy_headers headers);
  envoy_status_t locallyCloseStream(envoy_stream_t stream);
  envoy_status_t resetStream();

  // FIXME determine scope of this function
  envoy_status_t removeStream(envoy_stream_t stream_id);

private:
  // Everything in the below interface must only be accessed from the event_dispatcher's thread.
  // This allows us to generally avoid synchronization.
  AsyncClient::Stream* getStream(envoy_stream_t stream_id) const;

  std::unordered_map<envoy_stream_t, DirectStream> streams_;
  envoy_stream_t current_stream_id_ = 0;

  // The event_dispatcher is the only member state that may be accessed from a thread other than
  // the event_dispatcher's own thread.
  Event::Dispatcher& event_dispatcher_;
};

} // namespace Http
} // namespace Envoy
