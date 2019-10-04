licenses(["notice"])  # Apache 2

load("@rules_proto_grpc//java:defs.bzl", "java_grpc_library")

java_grpc_library(
    name = "envoy_mobile_test_proto",
    deps = [":test_proto"],
)

proto_library(
    name = "test_proto",
    srcs = ["envoy_mobile_test.proto"],
)
