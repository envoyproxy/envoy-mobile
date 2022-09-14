load(
    "@envoy//bazel:envoy_build_system.bzl",
    "envoy_cc_extension",
    "envoy_cc_library",
    "envoy_extension_package",
    "envoy_proto_library",
)

licenses(["notice"])  # Apache 2

envoy_extension_package()

envoy_proto_library(
    name = "sink_config",
    srcs = [
        "config.proto",
    ],
    deps = [
        "@envoy_api//envoy/config/core/v3:pkg",
    ],
)

cc_library(
    name = "dynamic_sink_interface",
    hdrs = ["dynamic_sink.h"],
)

envoy_cc_library(
    name = "sink_lib",
    srcs = ["sink.cc"],
    hdrs = [
        "sink.h",
    ],
    repository = "@envoy",
    deps = [
        ":dynamic_sink_interface",
        ":sink_config_cc_proto",
        "@envoy//envoy/server:factory_context_interface",
        "@envoy//envoy/stats:stats_interface",
        "@envoy//source/common/common:random_generator_lib",
        "@envoy_api//envoy/config/core/v3:pkg_cc_proto",
        "@envoy_api//envoy/service/metrics/v3:pkg_cc_proto",
        "@envoy_mobile//library/common/types:c_types_lib",
    ],
)

envoy_cc_extension(
    name = "config",
    srcs = ["config.cc"],
    hdrs = ["config.h"],
    repository = "@envoy",
    deps = [
        ":sink_lib",
        "@envoy//envoy/registry",
        "@envoy//source/common/common:assert_lib",
        "@envoy//source/common/config:utility_lib",
        "@envoy//source/extensions/filters/http/common:factory_base_lib",
        "@envoy//source/server:configuration_lib",
    ],
)
