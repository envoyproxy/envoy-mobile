licenses(["notice"])  # Apache 2

load("//bazel:swift_static_framework.bzl", "swift_static_framework")
load("@build_bazel_rules_apple//apple/internal/resource_rules:apple_resource_bundle.bzl", "apple_resource_bundle")

apple_resource_bundle(
    name = "ios_resources",
    resources = [
        "config.yaml",
    ],
)

swift_static_framework(
    name = "ios_framework",
    srcs = [
        "Client.swift",
        "Configuration.swift",
        "Envoy.swift",
        "Error.swift",
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
    module_name = "Envoy",
    objc_includes = [
        "//library/objective-c:EnvoyEngine.h",
    ],
#    resources = [":ios_resources"],
    resources = ["config.yaml"],
    visibility = ["//visibility:public"],
    deps = ["//library/objective-c:envoy_engine_objc_lib"],
)
