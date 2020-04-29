licenses(["notice"])  # Apache 2

load("@envoy//bazel:envoy_build_system.bzl", "envoy_cc_test", "envoy_package")

envoy_package()

envoy_cc_test(
    name = "engine_test",
    srcs = ["engine_test.cc"],
    repository = "@envoy",
    deps = [
        "//library/common:envoy_main_interface_lib",
        "//library/common/types:c_types_lib",
    ],
)