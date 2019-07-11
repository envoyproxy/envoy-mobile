#pragma once

#include "library/common/types.h"

#include "envoy/http/header_map.h"
#include "envoy/buffer/buffer.h"

namespace EnvoyMobile {
namespace Http {

class AsyncStream {
public:
  // TODO: do we want to have the raw observer struct?
  AsyncStream(envoy_observer);

  void sendHeaders(Envoy::Http::HeaderMap& headers, bool end_stream);
  void sendData(Envoy::Buffer::Instance& data, bool end_stream);
  void sendMetadata(Envoy::Http::HeaderMap& trailers, bool end_stream);
  void sendTrailers(Envoy::Http::HeaderMap& trailers);
  void close();
  void evict();
};

} // namespace Http
} // namespace EnvoyMobile