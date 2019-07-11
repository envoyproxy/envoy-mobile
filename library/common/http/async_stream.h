#pragma once

#include "library/common/types.h"

#include "envoy/buffer/buffer.h"
#include "envoy/http/async_client.h"
#include "envoy/http/header_map.h"

namespace Envoy {
namespace Http {

class MobileAsyncStream : public AsyncClient::Stream,
                          Logger::Loggable<Logger::Id::http> {
public:
  // TODO: do we want to have the raw observer struct?
  MobileAsyncStream(envoy_observer);

  // AsyncClient::Stream
  void sendHeaders(Envoy::Http::HeaderMap& headers, bool end_stream);
  void sendData(Envoy::Buffer::Instance& data, bool end_stream);
  void sendMetadata(Envoy::Http::HeaderMap& trailers, bool end_stream);
  void sendTrailers(Envoy::Http::HeaderMap& trailers);
  void reset();

  void locally_close();
};

} // namespace Http
} // namespace EnvoyMobile
