#include "library/common/http/async_stream.h"

namespace Envoy {
namespace Http {

MobileAsyncStream::MobileAsyncStream(envoy_observer) {}

void MobileAsyncStream::sendHeaders(Envoy::Http::HeaderMap&, bool) {}
void MobileAsyncStream::sendData(Envoy::Buffer::Instance&, bool) {}
void MobileAsyncStream::sendMetadata(Envoy::Http::HeaderMap&, bool) {}
void MobileAsyncStream::sendTrailers(Envoy::Http::HeaderMap&) {}
void MobileAsyncStream::reset() {}
void MobileAsyncStream::locally_close() {}

} // namespace Http
} // namespace Envoy
