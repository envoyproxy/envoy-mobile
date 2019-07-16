#include "library/common/http/async_stream_callbacks.h"

#include "library/common/buffer/utility.h"
#include "library/common/http/header_utility.h"

namespace Envoy {
namespace Http {

MobileAsyncStreamCallbacks::MobileAsyncStreamCallbacks(envoy_stream_t stream, envoy_observer observer)
    : stream_(stream), observer_(observer) {}

void MobileAsyncStreamCallbacks::onHeaders(HeaderMapPtr&& headers, bool end_stream) {
  observer_.h(stream_, Utility::transformHeaders(std::move(headers)), end_stream);
}

void MobileAsyncStreamCallbacks::onData(Buffer::Instance& data, bool end_stream) {
  observer_.d(stream_, Envoy::Buffer::Utility::transformData(data), end_stream);
}

void MobileAsyncStreamCallbacks::onTrailers(HeaderMapPtr&& trailers) {
  observer_.t(stream_, Utility::transformHeaders(std::move(trailers)));
}

void MobileAsyncStreamCallbacks::onReset() {
  observer_.e(stream_, {ENVOY_STREAM_RESET, {0, nullptr}});
}

} // namespace Http
} // namespace Envoy
