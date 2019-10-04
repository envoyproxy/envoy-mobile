licenses(["notice"])  # Apache 2

load("//bazel:kotlin_lib.bzl", "envoy_mobile_kt_library")
load("@rules_proto_grpc//java:defs.bzl", "java_grpc_library")

envoy_mobile_kt_library(
    name = "envoy_mobile_grpc_java_test_server_lib",
    srcs = [
        "EnvoyMobileEchoingTestServer.kt",
    ],
    visibility = ["//visibility:public"],
    deps = [
        ":envoy_mobile_test_proto",
    ],
)

java_grpc_library(
    name = "envoy_mobile_test_proto",
    deps = [":test_proto"],
)

proto_library(
    name = "test_proto",
    srcs = ["envoy_mobile_test.proto"],
)
