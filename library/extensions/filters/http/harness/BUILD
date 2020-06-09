
package(default_visibility = ["//visibility:public"])

load(
    "@envoy//bazel:envoy_build_system.bzl",
    "envoy_cc_binary",
    "envoy_cc_library",
    "envoy_cc_test",
)

load("@envoy_api//bazel:api_build_system.bzl", "api_proto_package")

api_proto_package()

envoy_cc_library(
    name = "harness_filter_lib",
    srcs = ["filter.cc"],
    hdrs = ["filter.h"],
    repository = "@envoy",
    deps = [
        ":pkg_cc_proto",
                "@envoy//include/envoy/http:filter_interface",
                        "@envoy//source/extensions/filters/http/common:pass_through_filter_lib",
    ],
)

envoy_cc_library(
    name = "config",
    srcs = ["config.cc"],
    hdrs = ["config.h"],
    repository = "@envoy",
    deps = [
                ":pkg_cc_proto",
        ":harness_filter_lib",
                "@envoy//source/extensions/filters/http/common:factory_base_lib",
    ],
)
