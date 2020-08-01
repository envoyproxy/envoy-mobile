load("@rules_cc//cc:defs.bzl", "cc_binary")
load("@envoy//bazel:envoy_build_system.bzl", "envoy_package")
load("//bazel:local_jar_execution.bzl", "local_jar_cc_binary")

licenses(["notice"])  # Apache 2

envoy_package()

cc_binary(
    name = "libenvoy_jni.so",
    srcs = [
        "jni_utility.h",
        "jni_interface.cc",
        "android_utility.cc",
    ],
    copts = ["-std=c++17"],
    linkopts = [
        "-lm",
        "-llog",
        "-Wl,-s",
    ],
    linkshared = True,
    deps = ["//library/common:envoy_main_interface_lib"],
)

local_jar_cc_binary(
    name = "java_libenvoy_jni.so",
    srcs = [
        "jni_utility.h",
        "jni_interface.cc",
        "java_utility.cc"
    ],
        includes = [
        "../../../external/local_jdk/include",
        "../../../external/local_jdk/include/darwin",
        "../../../external/local_jdk/include/linux",
    ],
    deps = ["//library/common:envoy_main_interface_lib"],
)
