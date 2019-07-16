licenses(["notice"])  # Apache 2

load("@envoy//bazel:envoy_build_system.bzl", "envoy_cc_library", "envoy_package")

envoy_package()

envoy_cc_library(
    name = "async_stream_callbacks_lib",
    srcs = ["async_stream_callbacks.cc"],
    hdrs = ["async_stream_callbacks.h"],
    repository = "@envoy",
    deps = [
        "//library/common/buffer:utility_lib",
        "//library/common/http:header_utility_lib",
        "//library/common/include:c_types_interface",
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
        ":async_stream_callbacks_lib",
        "//library/common/include:c_types_interface",
        "@envoy//include/envoy/http:async_client_interface",
    ],
)

envoy_cc_library(
    name = "header_utility_lib",
    srcs = ["header_utility.cc"],
    hdrs = ["header_utility.h"],
    repository = "@envoy",
    deps = [
        "//library/common/include:c_types_interface",
        "@envoy//include/envoy/buffer:buffer_interface",
        "@envoy//include/envoy/http:header_map_interface",
        "@envoy//source/common/http:header_map_lib",
    ],
)
