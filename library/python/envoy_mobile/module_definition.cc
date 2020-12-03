#include "pybind11/pybind11.h"
namespace py = pybind11;

#include "engine.h"
#include "engine_builder.h"
#include "engine_impl.h"
#include "envoy_error.h"
#include "executor.h"
#include "headers.h"
#include "headers_builder.h"
#include "log_level.h"
#include "request_headers.h"
#include "request_headers_builder.h"
#include "request_method.h"
#include "request_trailers.h"
#include "request_trailers_builder.h"
#include "response_headers.h"
#include "response_headers_builder.h"
#include "response_trailers.h"
#include "response_trailers_builder.h"
#include "retry_policy.h"
#include "stats_client.h"
#include "stats_client_impl.h"
#include "stream.h"
#include "stream_callbacks.h"
#include "stream_client.h"
#include "stream_client_impl.h"
#include "stream_prototype.h"
#include "trailers.h"
#include "upstream_http_protocol.h"


PYBIND11_MODULE(envoy_mobile, m) {
  m.doc() = "a thin wrapper around envoy-mobile to provide speedy networking for python";
}
