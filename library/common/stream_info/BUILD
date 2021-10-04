load("@envoy//bazel:envoy_build_system.bzl", "envoy_cc_library", "envoy_package")

licenses(["notice"])  # Apache 2

envoy_package()

envoy_cc_library(
    name = "aux_stream_info_lib",
    srcs = ["aux_stream_info.cc"],
    hdrs = ["aux_stream_info.h"],
    repository = "@envoy",
    deps = [
        "//library/common/types:c_types_lib",
        "@envoy//envoy/stream_info:stream_info_interface",
        "@envoy//source/common/common:assert_lib",
    ],
)
