#include "stream.h"

#include "bridge_utility.h"
#include "library/common/main_interface.h"
#include "library/common/types/c_types.h"

namespace Envoy {
namespace Platform {

Stream::Stream(envoy_engine_t engine_handle, envoy_stream_t handle)
    : engine_handle_(engine_handle), handle_(handle) {}

Stream& Stream::sendHeaders(RequestHeadersSharedPtr headers, bool end_stream) {
  envoy_headers raw_headers = rawHeaderMapAsEnvoyHeaders(headers->allHeaders());
  ::send_headers(this->engine_handle_, this->handle_, raw_headers, end_stream);
  return *this;
}

Stream& Stream::sendData(envoy_data data, bool end_stream) {
  ::send_data(this->engine_handle_, this->handle_, data, end_stream);
  return *this;
}

void Stream::close(RequestTrailersSharedPtr trailers) {
  envoy_headers raw_headers = rawHeaderMapAsEnvoyHeaders(trailers->allHeaders());
  ::send_trailers(this->engine_handle_, this->handle_, raw_headers);
}

void Stream::close(envoy_data data) {
  ::send_data(this->engine_handle_, this->handle_, data, true);
}

void Stream::cancel() { reset_stream(this->engine_handle_, this->handle_); }

} // namespace Platform
} // namespace Envoy
