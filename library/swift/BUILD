licenses(["notice"])  # Apache 2

load("@build_bazel_rules_swift//swift:swift.bzl", "swift_library")
load("//bazel:swift_static_framework.bzl", "prebuilt_swift_static_framework")

prebuilt_swift_static_framework(
    name = "swift_framework",
    srcs = ["Envoy.swift"],
    deps = ["//library/objective-c:envoy_engine_objc_lib"]
)

swift_library(
  name = "envoy_swift_lib",
  srcs = [
    "Envoy.swift",
  ],
  module_name = "Envoy",
  visibility = ["//visibility:public"],
  deps = ["//library/objective-c:envoy_engine_objc_lib"]
)
