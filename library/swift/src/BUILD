licenses(["notice"])  # Apache 2

load("//bazel:swift_static_framework.bzl", "swift_static_framework")
load("@build_bazel_rules_swift//swift:swift.bzl", "swift_library")

swift_static_framework(
    name = "ios_framework",
    srcs = ["Empty.swift"],
    module_name = "Envoy",
    objc_includes = [
        "//library/objective-c:EnvoyEngine.h",
    ],
    visibility = ["//visibility:public"],
    deps = [
        ":ios_interfaces_lib",
        "//library/swift/src/grpc:ios_grpc_lib",
    ],
)

swift_library(
    name = "ios_interfaces_lib",
    srcs = [
        "Client.swift",
        "Envoy.swift",
        "EnvoyBuilder.swift",
        "EnvoyError.swift",
        "EnvoyStreamEmitter.swift",
        "LogLevel.swift",
        "Request.swift",
        "RequestBuilder.swift",
        "RequestMapper.swift",
        "RequestMethod.swift",
        "ResponseHandler.swift",
        "RetryPolicy.swift",
        "RetryPolicyMapper.swift",
        "StreamEmitter.swift",
    ],
    copts = [
        "-import-objc-header",
        "$(location //library/objective-c:EnvoyEngine.h)",
    ],
    module_name = "EnvoyInterfaces",
    swiftc_inputs = ["//library/objective-c:EnvoyEngine.h"],
    visibility = ["//visibility:public"],
    deps = [
        "//library/objective-c:envoy_engine_objc_lib",
    ],
)
