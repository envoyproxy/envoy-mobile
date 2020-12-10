#pragma once

#include <cstddef>
#include <vector>

#include "library/common/types/c_types.h"
#include "request_headers.h"
#include "request_trailers.h"

class Stream {
public:
  Stream(envoy_stream_t handle);

  Stream& send_headers(RequestHeadersSharedPtr headers, bool end_stream);
  Stream& send_data(const std::vector<std::byte>& data);
  void close(RequestTrailersSharedPtr trailers);
  void close(const std::vector<std::byte>& data);
  void cancel();

private:
  envoy_stream_t handle_;
};

using StreamSharedPtr = std::shared_ptr<Stream>;
