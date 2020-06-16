load("@envoy//bazel:envoy_build_system.bzl", "envoy_cc_library", "envoy_package")

licenses(["notice"])  # Apache 2

envoy_package()

envoy_cc_library(
    name = "bootstrap_config",
    srcs = [
        "certificates.inc",
        "config_template.cc",
    ],
    repository = "@envoy",
    visibility = ["//visibility:public"],
)
