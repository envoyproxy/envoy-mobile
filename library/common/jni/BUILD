load("@rules_cc//cc:defs.bzl", "cc_binary")
load("@envoy//bazel:envoy_build_system.bzl", "envoy_cc_library", "envoy_package")

licenses(["notice"])  # Apache 2

envoy_package()

envoy_cc_library(
    name = "jni_utility_lib",
    srcs = [
        "jni_utility.cc",
        "jni_version.cc",
    ],
    hdrs = [
        "jni_utility.h",
        "jni_version.h",
    ],
    repository = "@envoy",
    deps = [
        "//:jni_headers",
        "//library/common/types:c_types_lib",
        "@envoy//source/common/common:assert_lib",
    ],
)

# Produces two binaries: one for Android, and one for tests.
# The test binary can't refer to any Android code.
cc_binary(
    name = "libenvoy_jni.so",
    srcs = [
        "android_jni_interface.cc",
        "jni_interface.cc",
    ],
    copts = [
        "-std=c++17",
    ] + select({
        ":unit_test": ["-DUNIT_TEST"],
        "//conditions:default": [],
    }),
    linkopts = [
    ] + select({
        ":unit_test": [],
        "//conditions:default": [
            "-Wl,-s",
            "-lm",
            "-llog",
        ],
    }),
    linkshared = True,
    deps = [
        ":jni_utility_lib",
        "//library/common:envoy_main_interface_lib",
        "//library/common/api:c_types",
    ] + select({
        ":unit_test": [":android_log_lib"],
        "//conditions:default": [],
    }),
)

# For test only
config_setting(
    name = "unit_test",
    values = {
        "define": "UNIT_TEST=True",
    },
    visibility = ["//visibility:private"],
)

# For test only
envoy_cc_library(
    name = "android_log_lib",
    srcs = [
        "android_log.cc",
    ],
    hdrs = [
        "android_log.h",
    ],
    repository = "@envoy",
    visibility = ["//visibility:private"],
)
