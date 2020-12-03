load("@pybind11_bazel//:build_defs.bzl", "pybind_extension")

cc_library(
    name = "envoy_mobile_lib",
    hdrs = [
        "engine.h",
        "engine_builder.h",
        "engine_impl.h",
        "envoy_error.h",
        "executor.h",
        "headers.h",
        "headers_builder.h",
        "log_level.h",
        "request_headers.h",
        "request_headers_builder.h",
        "request_method.h",
        "request_trailers.h",
        "request_trailers_builder.h",
        "response_headers.h",
        "response_headers_builder.h",
        "response_trailers.h",
        "response_trailers_builder.h",
        "retry_policy.h",
        "stats_client.h",
        "stats_client_impl.h",
        "stream.h",
        "stream_callbacks.h",
        "stream_client.h",
        "stream_client_impl.h",
        "stream_prototype.h",
        "trailers.h",
        "upstream_http_protocol.h",
    ],
    visibility = ["//visibility:public"],
    deps = [
        "//library/common:envoy_main_interface_lib",
    ],
)

pybind_extension(
    name = "envoy_mobile",
    srcs = [
        "module_definition.cc",
    ],
    visibility = ["//visibility:public"],
    deps = [
        ":envoy_mobile_lib",
    ],
)
