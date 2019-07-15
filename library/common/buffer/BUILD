licenses(["notice"])  # Apache 2

load("@envoy//bazel:envoy_build_system.bzl", "envoy_cc_library", "envoy_package")

envoy_package()

envoy_cc_library(
    name = "utility_lib",
    hdrs = ["utility.h"],
    srcs = ["utility.cc"],
    repository = "@envoy",
    deps = [
      "//library/common/include:c_types_interface",
      "@envoy//include/envoy/buffer:buffer_interface",
    ],
)

