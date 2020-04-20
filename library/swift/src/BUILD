licenses(["notice"])  # Apache 2

load("@build_bazel_rules_apple//apple:ios.bzl", "ios_static_framework")
load("@build_bazel_rules_swift//swift:swift.bzl", "swift_library")
load("//bazel:swift_header_collector.bzl", "swift_header_collector")

swift_library(
    name = "lib",
    srcs = [
        "Data+Extension.swift",
        "EnvoyClient.swift",
        "EnvoyClientBuilder.swift",
        "EnvoyError.swift",
        "EnvoyStreamEmitter.swift",
        "GRPCClient.swift",
        "GRPCRequestBuilder.swift",
        "GRPCResponseHandler.swift",
        "GRPCStreamEmitter.swift",
        "HTTPClient.swift",
        "LogLevel.swift",
        "Request.swift",
        "RequestBuilder.swift",
        "RequestMapper.swift",
        "RequestMethod.swift",
        "ResponseHandler.swift",
        "RetryPolicy.swift",
        "RetryPolicyMapper.swift",
        "StreamEmitter.swift",
        "UpstreamHttpProtocol.swift",
    ],
    module_name = "Envoy",
    private_deps = ["//library/objective-c:envoy_engine_objc_lib"],
)

swift_header_collector(
    name = "collector",
    library = "lib",
)

ios_static_framework(
    name = "ios_framework",
    hdrs = ["collector"],
    bundle_name = "Envoy",
    minimum_os_version = "10.0",
    visibility = ["//visibility:public"],
    deps = ["lib"],
)
