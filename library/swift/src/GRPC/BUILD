licenses(["notice"])  # Apache 2

load("@build_bazel_rules_swift//swift:swift.bzl", "swift_library")

swift_library(
    name = "ios_grpc_lib",
    srcs = [
        "GRPCClient.swift",
        "GRPCRequestBuilder.swift",
        "GRPCResponseHandler.swift",
        "GRPCStreamEmitter.swift",
    ],
    module_name = "EnvoyGRPC",
    visibility = ["//visibility:public"],
    deps = [
        "//library/swift/src:ios_interfaces_lib",
    ],
)
