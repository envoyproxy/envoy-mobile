load("@envoy//bazel:envoy_build_system.bzl", "envoy_cc_library")
load("@envoy_api//bazel:api_build_system.bzl", "api_proto_package")

licenses(["notice"])  # Apache 2

package(default_visibility = ["//visibility:public"])

api_proto_package(
    has_services = True,
    deps = [
        "@envoy_api//envoy/config/core/v3:pkg",
        "@prometheus_metrics_model//:client_model",
    ],
)

envoy_cc_library(
    name = "mobile_grpc_streamer",
    srcs = ["mobile_grpc_streamer.cc"],
    hdrs = ["mobile_grpc_streamer.h"],
    repository = "@envoy",
    deps = [
        ":pkg_cc_proto",
        "@envoy//source/common/common:minimal_logger_lib",
        "@envoy//source/extensions/stat_sinks/metrics_service:metrics_service_grpc_lib",
    ],
)

envoy_cc_library(
    name = "config",
    srcs = ["config.cc"],
    hdrs = ["config.h"],
    repository = "@envoy",
    deps = [
        ":mobile_grpc_streamer",
        "@envoy//source/common/grpc:async_client_lib",
        "@envoy//source/extensions/stat_sinks/metrics_service:metrics_service_grpc_lib",
        "@envoy//source/server:configuration_lib",
    ],
)
