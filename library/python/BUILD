load("@pybind11_bazel//:build_defs.bzl", "pybind_extension", "pybind_library")

licenses(["notice"])  # Apache 2

pybind_library(
    name = "envoy_engine_lib",
    srcs = [
        "envoy_engine/bytes_view.cc",
        "envoy_engine/engine_builder_shim.cc",
        "envoy_engine/stream_prototype_shim.cc",
        "envoy_engine/stream_shim.cc",
    ],
    hdrs = [
        "envoy_engine/bytes_view.h",
        "envoy_engine/engine_builder_shim.h",
        "envoy_engine/stream_prototype_shim.h",
        "envoy_engine/stream_shim.h",
    ],
    visibility = ["//visibility:public"],
    deps = [
        "//library/cc:envoy_engine_cc_lib",
    ],
)

pybind_extension(
    name = "envoy_engine",
    srcs = [
        "envoy_engine/module_definition.cc",
    ],
    visibility = ["//visibility:public"],
    deps = [
        ":envoy_engine_lib",
        "//library/cc:envoy_engine_cc_lib",
        # for log levels enum
        "@envoy//source/common/common:base_logger_lib",
    ],
)
