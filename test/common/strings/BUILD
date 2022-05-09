load("@envoy//bazel:envoy_build_system.bzl", "envoy_cc_test", "envoy_package")

licenses(["notice"])  # Apache 2

envoy_package()

envoy_cc_test(
    name = "string_conversions_test",
    srcs = ["string_conversions_test.cc"],
    repository = "@envoy",
    deps = [
        "//library/common/strings:string_conversions_lib",
    ],
)
