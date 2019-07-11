licenses(["notice"])  # Apache 2

load("@envoy//bazel:envoy_build_system.bzl", "envoy_cc_library", "envoy_package")

envoy_package()

envoy_cc_library(
    name = "async_stream_lib",
    srcs = ["async_stream.cc"],
    hdrs = ["async_stream.h"],
    repository = "@envoy",
    deps = [
        "//library/common:envoy_types_lib",
        "@envoy//include/envoy/buffer:buffer_interface",
        "@envoy//include/envoy/http:async_client_interface",
        "@envoy//include/envoy/http:header_map_interface",
    ],
)

envoy_cc_library(
    name = "async_stream_manager_lib",
    srcs = ["async_stream_manager.cc"],
    hdrs = ["async_stream_manager.h"],
    repository = "@envoy",
    deps = [
        ":async_stream_lib",
        "//library/common:envoy_types_lib",
    ],
)

envoy_cc_library(
    name = "utility_lib",
    srcs = ["utility.cc"],
    hdrs = ["utility.h"],
    repository = "@envoy",
    deps = [
        "//library/common:envoy_types_lib",
        "@envoy//include/envoy/buffer:buffer_interface",
        "@envoy//include/envoy/http:header_map_interface",
    ],
)
