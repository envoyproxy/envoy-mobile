#include "stream_callbacks.h"

// NOLINT(namespace-envoy)

#include "bridge_utility.h"

EnvoyHttpCallbacksAdapter::EnvoyHttpCallbacksAdapter(ExecutorSharedPtr executor,
                                                     StreamCallbacksSharedPtr callbacks)
    : executor_(executor), stream_callbacks_(callbacks) {}

envoy_http_callbacks EnvoyHttpCallbacksAdapter::as_envoy_http_callbacks() {
  envoy_http_callbacks callbacks{
      .on_headers = &EnvoyHttpCallbacksAdapter::dispatch_on_headers,
      .on_data = &EnvoyHttpCallbacksAdapter::dispatch_on_data,
      // on_metadata is not used
      .on_trailers = &EnvoyHttpCallbacksAdapter::dispatch_on_trailers,
      .on_error = &EnvoyHttpCallbacksAdapter::dispatch_on_error,
      .on_complete = &EnvoyHttpCallbacksAdapter::dispatch_on_complete,
      .on_cancel = &EnvoyHttpCallbacksAdapter::dispatch_on_cancel,
      .context = this,
  };
  return callbacks;
}

void* EnvoyHttpCallbacksAdapter::dispatch_on_headers(envoy_headers headers, bool end_stream,
                                                     void* context) {
  auto self = static_cast<EnvoyHttpCallbacksAdapter*>(context);
  if (self->stream_callbacks_->on_headers.has_value()) {
    auto raw_headers = envoy_headers_as_raw_headers(headers);
    auto response_headers = std::make_shared<ResponseHeaders>(raw_headers);
    self->executor_->execute(
        [=]() { self->stream_callbacks_->on_headers.value()(response_headers, end_stream); });
  }
  return context;
}

void* EnvoyHttpCallbacksAdapter::dispatch_on_data(envoy_data data, bool end_stream, void* context) {
  auto self = static_cast<EnvoyHttpCallbacksAdapter*>(context);
  if (self->stream_callbacks_->on_error.has_value()) {
    auto buffer = envoy_data_as_buffer(data);
    self->executor_->execute(
        [=]() { self->stream_callbacks_->on_data.value()(buffer, end_stream); });
  }
  return context;
}

void* EnvoyHttpCallbacksAdapter::dispatch_on_trailers(envoy_headers metadata, void* context) {
  auto self = static_cast<EnvoyHttpCallbacksAdapter*>(context);
  if (self->stream_callbacks_->on_trailers.has_value()) {
    auto raw_headers = envoy_headers_as_raw_headers(metadata);
    auto response_trailers = std::make_shared<ResponseTrailers>(raw_headers);
    self->executor_->execute(
        [=]() { self->stream_callbacks_->on_trailers.value()(response_trailers); });
  }
  return context;
}

void* EnvoyHttpCallbacksAdapter::dispatch_on_error(envoy_error raw_error, void* context) {
  auto self = static_cast<EnvoyHttpCallbacksAdapter*>(context);
  if (self->stream_callbacks_->on_error.has_value()) {
    EnvoyErrorSharedPtr error = std::make_shared<EnvoyError>();
    error->error_code = raw_error.error_code;
    error->message = envoy_data_as_string(raw_error.message);
    error->attempt_count = absl::optional<int>(raw_error.attempt_count);

    self->executor_->execute([=]() { self->stream_callbacks_->on_error.value()(error); });
  }
  return context;
}

void* EnvoyHttpCallbacksAdapter::dispatch_on_complete(void* context) {
  auto self = static_cast<EnvoyHttpCallbacksAdapter*>(context);
  if (self->stream_callbacks_->on_complete.has_value()) {
    self->executor_->execute(self->stream_callbacks_->on_complete.value());
  }
  return context;
}

void* EnvoyHttpCallbacksAdapter::dispatch_on_cancel(void* context) {
  auto self = static_cast<EnvoyHttpCallbacksAdapter*>(context);
  if (self->stream_callbacks_->on_cancel.has_value()) {
    self->executor_->execute(self->stream_callbacks_->on_cancel.value());
  }
  return context;
}
