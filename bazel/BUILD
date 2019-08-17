licenses(["notice"])  # Apache 2

load(
    "@build_bazel_rules_apple//apple/testing/default_runner:ios_test_runner.bzl",
    "ios_test_runner",
)

ios_test_runner(
    name = "envoy_ios_test_runner",
    device_type = "iPhone Xs",
    os_version = "12.4",
)

load("@envoy//bazel:envoy_build_system.bzl", "envoy_package")
load("@io_bazel_rules_kotlin//kotlin:kotlin.bzl", "kt_jvm_library")

envoy_package()

kt_jvm_library(
    name = "envoy_mobile_test_suite",
    srcs = [
        "EnvoyMobileTestSuite.kt",
    ],
    visibility = ["//visibility:public"],
    deps = [
        "@maven//:junit_junit",
    ],
)
