licenses(["notice"])  # Apache 2

load("//bazel:kotlin_lib.bzl", "envoy_mobile_kt_library")

envoy_mobile_kt_library(
    name = "envoy_mobile_grpc_java_test_server_lib",
    srcs = [
        "EnvoyMobileEchoingTestServer.kt",
    ],
    visibility = ["//visibility:public"],
    deps = [
        "//library/proto/test:envoy_mobile_test_proto",
    ],
)