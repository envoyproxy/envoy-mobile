#include "pybind11/pybind11.h"
namespace py = pybind11;

#include "library/cc/engine.h"
#include "library/cc/engine_builder.h"
#include "library/cc/engine_impl.h"
#include "library/cc/envoy_error.h"
#include "library/cc/executor.h"
#include "library/cc/headers.h"
#include "library/cc/headers_builder.h"
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
#include "library/cc/trailers.h"
#include "library/cc/upstream_http_protocol.h"


PYBIND11_MODULE(envoy_mobile, m) {
  m.doc() = "a thin wrapper around envoy-mobile to provide speedy networking for python";
}
