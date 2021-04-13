load("@rules_cc//cc:defs.bzl", "cc_binary", "cc_library")
load("@envoy//bazel:envoy_build_system.bzl", "envoy_package")

licenses(["notice"])  # Apache 2

envoy_package()

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
    ],
    linkopts = [
        "-Wl,-s",
    ],
    linkshared = True,
    deps = [
        "//library/common:envoy_main_interface_lib",
        "//library/common/api:c_types",
    ] + select({
        ":unit_test": [":java_jni_support"],
        "//conditions:default": [":ndk_jni_support"],
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

# Utility implementation for Java JNI
cc_library(
    name = "java_jni_support",
    srcs = [
        "java_jni_support.cc",
        "jni_utility.cc",
        "jni_version.cc",
        "@local_jdk//:jni_header",
    ],
    hdrs = [
        "jni_support.h",
        "jni_utility.h",
        "jni_version.h",
    ],
    copts = ["-std=c++14"],
    linkopts = [
        "-lm",
    ],
    deps = [
        "//bazel:jni",
        "//library/common/types:c_types_lib",
        "@envoy//source/common/common:assert_lib",
    ],
)

# Utility implementation for NDK JNI
cc_library(
    name = "ndk_jni_support",
    srcs = [
        "jni_utility.cc",
        "jni_version.cc",
        "ndk_jni_support.cc",
    ],
    hdrs = [
        "jni_support.h",
        "jni_utility.h",
        "jni_version.h",
    ],
    copts = ["-std=c++14"],
    linkopts = [
        "-lm",
        "-llog",
    ],
    deps = [
        "//library/common:envoy_main_interface_lib",
        "//library/common/types:c_types_lib",
        "@envoy//source/common/common:assert_lib",
    ],
)
