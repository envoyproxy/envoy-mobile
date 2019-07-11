#include "library/common/http/async_stream.h"

namespace EnvoyMobile {
namespace Http {

AsyncStream::AsyncStream(envoy_observer) {}

void AsyncStream::sendHeaders(Envoy::Http::HeaderMap&, bool) {}
void AsyncStream::sendData(Envoy::Buffer::Instance&, bool) {}
void AsyncStream::sendMetadata(Envoy::Http::HeaderMap&, bool) {}
void AsyncStream::sendTrailers(Envoy::Http::HeaderMap&) {}
void AsyncStream::close() {}
void AsyncStream::evict() {}

} // namespace Http
} // namespace EnvoyMobile