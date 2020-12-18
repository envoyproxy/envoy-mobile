#pragma once

#include <vector>

#include "library/common/types/c_types.h"
#include "request_headers.h"
#include "request_trailers.h"
#include "stream_callbacks.h"

namespace Envoy {
namespace Platform {

class Stream {
public:
  Stream(envoy_stream_t handle, EnvoyHttpCallbacksAdapterSharedPtr adapter);

  Stream& send_headers(RequestHeadersSharedPtr headers, bool end_stream);
  Stream& send_data(const std::vector<uint8_t>& data);
  void close(RequestTrailersSharedPtr trailers);
  void close(const std::vector<uint8_t>& data);
  void cancel();

private:
  envoy_stream_t handle_;
  EnvoyHttpCallbacksAdapterSharedPtr adapter_;
};

using StreamSharedPtr = std::shared_ptr<Stream>;

} // namespace Platform
} // namespace Envoy
