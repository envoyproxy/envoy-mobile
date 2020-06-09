licenses(["notice"])  # Apache 2

load("@envoy//bazel:envoy_build_system.bzl", "envoy_cc_library", "envoy_package")

envoy_package()

envoy_cc_library(
    name = "platform_filter_lib",
    srcs = ["platform_filter.cc"],
    hdrs = [
        "c_types.h",
        "platform_filter.h",
    ],
    repository = "@envoy",
    deps = [
        "//library/common/buffer:utility_lib",
        "//library/common/http:header_utility_lib",
        "//library/common/types:c_types_lib",
        "@envoy//include/envoy/http:filter_interface",
        "@envoy//source/extensions/filters/http/common:pass_through_filter_lib",
    ],
)
