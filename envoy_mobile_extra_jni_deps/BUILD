load("@envoy//bazel:envoy_build_system.bzl", "envoy_cc_library", "envoy_package")

licenses(["notice"])  # Apache 2

envoy_package()

envoy_cc_library(
    name = "extra_jni_dep",
    repository = "@envoy",
    visibility = ["//visibility:public"],
)
