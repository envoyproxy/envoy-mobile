#include "library/common/http/async_stream_callbacks.h"

namespace Envoy {
namespace Http {

MobileAsyncStreamCallbacks::MobileAsyncStreamCallbacks(envoy_observer observer)
    : observer_(observer) {}

void MobileAsyncStreamCallbacks::onHeaders(HeaderMapPtr&& headers, bool end_stream) {
  observer_.h(Utility::transformHeaders(std::move(headers)), end_stream);
}

void MobileAsyncStreamCallbacks::onData(Buffer::Instance& data, bool end_stream) {
  // observer_.d(Envoy::Buffer::Utility::transformData(data), end_stream);
}

void MobileAsyncStreamCallbacks::onTrailers(HeaderMapPtr&& trailers) {
  observer_.t(Utility::transformHeaders(std::move(trailers));
}

void MobileAsyncStreamCallbacks::onReset() { observer_.e({ENVOY_STREAM_RESET, {0, nullptr}}); }

} // namespace Http
} // namespace Envoy
