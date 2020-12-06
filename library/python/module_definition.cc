#include "pybind11/pybind11.h"
namespace py = pybind11;

#include "library/cc/engine.h"
#include "library/cc/engine_builder.h"
#include "library/cc/engine_impl.h"
#include "library/cc/envoy_error.h"
#include "library/cc/executor.h"
#include "library/cc/log_level.h"
#include "library/cc/request_headers.h"
#include "library/cc/request_headers_builder.h"
#include "library/cc/request_method.h"
#include "library/cc/request_trailers.h"
#include "library/cc/request_trailers_builder.h"
#include "library/cc/response_headers.h"
#include "library/cc/response_headers_builder.h"
#include "library/cc/response_trailers.h"
#include "library/cc/response_trailers_builder.h"
#include "library/cc/retry_policy.h"
#include "library/cc/stats_client.h"
#include "library/cc/stats_client_impl.h"
#include "library/cc/stream.h"
#include "library/cc/stream_callbacks.h"
#include "library/cc/stream_client.h"
#include "library/cc/stream_client_impl.h"
#include "library/cc/stream_prototype.h"
#include "library/cc/upstream_http_protocol.h"

// TODO(crockeo): define Executor trampoline class

PYBIND11_MODULE(envoy_mobile, m) {
  m.doc() = "a thin wrapper around envoy-mobile to provide speedy networking for python";

  py::class_<Engine, EngineImpl>(m, "Engine");
  py::class_<EngineBuilder>(m, "EngineBuilder");
  py::class_<EnvoyError>(m, "EnvoyError");
  // TODO(crockeo): do enums
  // py::class_<LogLevel>(m, "LogLevel");
  py::class_<RequestHeaders>(m, "RequestHeaders");
  py::class_<RequestHeadersBuilder>(m, "RequestHeadersBuilder");
  py::class_<RequestMethod>(m, "RequestMethod");
  py::class_<RequestTrailers>(m, "RequestTrailers");
  py::class_<RequestTrailersBuilder>(m, "RequestTrailersBuilder");
  py::class_<ResponseHeaders>(m, "ResponseHeaders");
  py::class_<ResponseHeadersBuilder>(m, "ResponseHeadersBuilder");
  py::class_<ResponseTrailers>(m, "ResponseTrailers");
  py::class_<ResponseTrailersBuilder>(m, "ResponseTrailersBuilder");
  // TODO(crockeo): do enums
  // py::class_<RetryRule>(m, "RetryRule");
  py::class_<RetryPolicy>(m, "RetryPolicy");
  py::class_<StatsClient, StatsClientImpl>(m, "StatsClient");
  py::class_<Stream>(m, "Stream");
  py::class_<StreamCallbacks>(m, "StreamCallbacks");
  py::class_<StreamClient, StreamClientImpl>(m, "StreamClient");
  py::class_<StreamPrototype>(m, "StreamPrototype");
  // TODO(crockeo): do enums
  // py::class_<UpstreamHttpProtocol>(m, "UpstreamHttpProtocol");

}
