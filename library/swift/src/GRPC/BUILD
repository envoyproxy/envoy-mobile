licenses(["notice"])  # Apache 2

load("//bazel:swift_static_framework.bzl", "swift_static_framework")

swift_static_framework(
    name = "ios_grpc_framework",
    srcs = [
        "GRPCClient.swift",
        "GRPCHandler.swift",
        "GRPCRequestBuilder.swift",
        "GRPCStreamEmitter.swift",
    ],
    module_name = "EnvoyGRPC",
    visibility = ["//visibility:public"],
    deps = [
        "//library/swift/src:ios_framework",
    ],
)
