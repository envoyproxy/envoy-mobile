licenses(["notice"])  # Apache 2

load("@build_bazel_rules_swift//swift:swift.bzl", "swift_library")
load("//bazel:swift_static_framework.bzl", "swift_static_framework")

swift_static_framework(
    name = "ios_framework",
    module_name = "Envoy",
    srcs = ["Envoy.swift"],
    objc_includes = ["//library/objective-c:EnvoyEngine.h"],
    visibility = ["//visibility:public"],
    deps = ["//library/objective-c:envoy_engine_objc_lib"],
)

swift_library(
    name = "envoy_swift_lib",
    srcs = [
        "Envoy.swift",
    ],
    module_name = "Envoy",
    visibility = ["//visibility:public"],
    deps = ["//library/objective-c:envoy_engine_objc_lib"],
)
