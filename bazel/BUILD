licenses(["notice"])  # Apache 2

load("@io_bazel_rules_kotlin//kotlin:kotlin.bzl", "kt_jvm_library")
load("//bazel:envoy_build_system.bzl", "envoy_package")

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

load(
    "@build_bazel_rules_apple//apple/testing/default_runner:ios_test_runner.bzl",
    "ios_test_runner",
)

# Fix for specifying device/OS until this is released:
# https://github.com/bazelbuild/rules_apple/pull/549
ios_test_runner(
    name = "envoy_ios_test_runner",
    device_type = "iPhone X",
    os_version = "12.1",
    visibility = ["//visibility:public"],
)
