licenses(["notice"])  # Apache 2

load("//bazel:kotlin_lib.bzl", "envoy_mobile_kt_library")

envoy_mobile_kt_library(
    name = "envoy_mobile_grpc_java_test_server_lib",
    srcs = [
        "EnvoyMobileEchoingTestServer.kt",
        "EnvoyMobileTestGrpc.java",
        "EnvoyMobileTestOuterClass.java",
    ],
    visibility = ["//visibility:public"],
    deps = [
        "@maven//:javax_annotation_javax_annotation_api",
        "@maven//:io_grpc_grpc_stub",
        "@maven//:io_grpc_grpc_core",
        "@maven//:io_grpc_grpc_protobuf",
        "@maven//:io_grpc_grpc_netty_shaded",
        "@maven//:com_google_protobuf_protobuf_java",
        "@maven//:com_google_guava_guava",
    ],
)