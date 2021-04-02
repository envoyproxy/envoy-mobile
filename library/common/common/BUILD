load("@envoy//bazel:envoy_build_system.bzl", "envoy_cc_library", "envoy_package")

licenses(["notice"])  # Apache 2

envoy_package()

envoy_cc_library(
    name = "lambda_logger_delegate_lib",
    hdrs = ["lambda_logger_delegate.h"],
    srcs = ["lambda_logger_delegate.cc"],
    repository = "@envoy",
    deps = [
        "@envoy//source/common/common:lock_guard_lib",
        "@envoy//source/common/common:macros",
        "@envoy//source/common/common:minimal_logger_lib",
        "@envoy//source/common/common:thread_lib",
    ],
)
