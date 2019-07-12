#include "library/common/http/async_stream_callbacks.h"

namespace Envoy {
namespace Http {

MobileAsyncStreamCallbacks::MobileAsyncStreamCallbacks(envoy_observer observer)
    : observer_(observer) {}

void MobileAsyncStreamCallbacks::onHeaders(HeaderMapPtr&&, bool) {
  // FIX: figure out why the overloading was busted. Same below.
  // observer_.h(Utility::transformHeaders(headers), end_stream);
}

void MobileAsyncStreamCallbacks::onData(Buffer::Instance&, bool) {
  // observer_.d(Utility::transformData(data), end_stream);
}

void MobileAsyncStreamCallbacks::onTrailers(HeaderMapPtr&&) {
  // observer_.t(Utility::transformHeaders(trailers));
}

void MobileAsyncStreamCallbacks::onReset() { observer_.e({ENVOY_STREAM_RESET, {0, nullptr}}); }

} // namespace Http
} // namespace Envoy
