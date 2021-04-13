#include "stream_prototype.h"

#include "library/common/main_interface.h"

namespace Envoy {
namespace Platform {

StreamPrototype::StreamPrototype(EngineSharedPtr engine) : engine_(engine) {
  this->callbacks_ = std::make_unique<StreamCallbacks>();
}

StreamSharedPtr StreamPrototype::start() {
  auto envoy_stream = init_stream(this->engine_->engine_);
  auto stream = std::make_shared<Stream>(this->engine_, envoy_stream);

  auto callbacks = this->callbacks_.release();
  callbacks->parent = stream;
  start_stream(envoy_stream, callbacks->as_envoy_http_callbacks());

  return stream;
}

StreamPrototype& StreamPrototype::set_on_headers(OnHeadersCallback closure) {
  this->callbacks_->on_headers = closure;
  return *this;
}

StreamPrototype& StreamPrototype::set_on_data(OnDataCallback closure) {
  this->callbacks_->on_data = closure;
  return *this;
}

StreamPrototype& StreamPrototype::set_on_trailers(OnTrailersCallback closure) {
  this->callbacks_->on_trailers = closure;
  return *this;
}

StreamPrototype& StreamPrototype::set_on_error(OnErrorCallback closure) {
  this->callbacks_->on_error = closure;
  return *this;
}

StreamPrototype& StreamPrototype::set_on_complete(OnCompleteCallback closure) {
  this->callbacks_->on_complete = closure;
  return *this;
}

StreamPrototype& StreamPrototype::set_on_cancel(OnCancelCallback closure) {
  this->callbacks_->on_cancel = closure;
  return *this;
}

} // namespace Platform
} // namespace Envoy
