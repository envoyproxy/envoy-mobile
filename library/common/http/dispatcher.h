#pragma once

#include <atomic>
#include <unordered_map>

#include "envoy/http/async_client.h"

#include "common/common/logger.h"

#include "library/common/http/async_stream_callbacks.h"
#include "library/common/include/c_types.h"

namespace Envoy {
namespace Http {

using DirectStream = std::pair<std::unique_ptr<MobileAsyncStreamCallbacks>, AsyncClient::Stream*>;

class Dispatcher : public Logger::Loggable<Logger::Id::http> {
public:
  Dispatcher(Event::Dispatcher&);

  envoy_stream_t startStream(envoy_observer);
  envoy_status_t sendHeaders(envoy_stream_t stream, envoy_headers headers, bool end_stream);
  envoy_status_t sendData(envoy_stream_t stream, envoy_headers headers, bool end_stream);
  envoy_status_t sendMetadata(envoy_stream_t stream, envoy_headers headers, bool end_stream);
  envoy_status_t sendTrailers(envoy_stream_t stream, envoy_headers headers);
  envoy_status_t locallyCloseStream(envoy_stream_t stream);
  envoy_status_t resetStream();
  
private:
// Everything in the below interface must only be accessed from the event_dispatcher's thread. This
// allows us to generally avoid synchronization.
  AsyncClient::Stream* getStream(envoy_stream_t stream_id) const;
  void removeStream(envoy_stream_t stream_id) const;

  std::unordered_map<envoy_stream_t, DirectStream> streams_;
  envoy_stream_t current_stream_id_ = 0;

// The event_dispatcher is the only member state that may be accessed from a thread other than
// the event_dispatcher's own thread.
  Event::Dispatcher& event_dispatcher_;
};

} // namespace Http
} // namespace Envoy
