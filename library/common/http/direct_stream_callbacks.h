#pragma once

#include "envoy/buffer/buffer.h"
#include "envoy/http/async_client.h"
#include "envoy/http/header_map.h"

#include "library/common/include/c_types.h"

namespace Envoy {
namespace Http {

class DirectStreamCallbacks : public AsyncClient::StreamCallbacks {
public:
  DirectStreamCallbacks(envoy_stream_t stream, envoy_observer observer, Dispatcher& http_dispatcher);
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

} // namespace Http
} // namespace Envoy
