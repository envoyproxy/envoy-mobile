#include "library/common/http/async_stream_callbacks.h"

#include "library/common/buffer/utility.h"
#include "library/common/http/header_utility.h"

namespace Envoy {
namespace Http {

DirectStreamCallbacks::DirectStreamCallbacks(envoy_stream_t stream, envoy_observer observer, Dispatcher& http_dispatcher)
    : stream_(stream), observer_(observer), http_dispatcher_(http_dispatcher) {}

void DirectStreamCallbacks::onHeaders(HeaderMapPtr&& headers, bool end_stream) {
  if (end_stream) {
    http_dispatcher_.removeStream(stream_);
  }
  observer_.h(stream_, Utility::transformHeaders(std::move(headers)), end_stream);
}

void DirectStreamCallbacks::onData(Buffer::Instance& data, bool end_stream) {
  if (end_stream) {
    http_dispatcher_.removeStream(stream_);
  }
  observer_.d(stream_, Envoy::Buffer::Utility::transformData(data), end_stream);
}

void DirectStreamCallbacks::onTrailers(HeaderMapPtr&& trailers) {
  http_dispatcher_.removeStream(stream_);
  observer_.t(stream_, Utility::transformHeaders(std::move(trailers)));
}

void DirectStreamCallbacks::onReset() {
  http_dispatcher_.removeStream(stream_);
  observer_.e(stream_, {ENVOY_STREAM_RESET, {0, nullptr}});
}

} // namespace Http
} // namespace Envoy
