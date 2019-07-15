licenses(["notice"])  # Apache 2

load("@envoy//bazel:envoy_build_system.bzl", "envoy_cc_binary", "envoy_cc_library", "envoy_package")

envoy_package()

envoy_cc_library(
    name = "envoy_main_interface_lib",
    srcs = ["main_interface.cc"],
    hdrs = ["main_interface.h"],
    repository = "@envoy",
    deps = [
        ":envoy_types_lib",
        "//library/common/http:async_stream_manager_lib",
        "//library/common/http:utility_lib",
        "@envoy//source/exe:envoy_main_common_lib",
    ],
)

envoy_cc_library(
    name = "envoy_buffer_utility_lib",
    hdrs = ["buffer_utility.h"],
    srcs = ["buffer_utility.cc"],
    repository = "@envoy",
    deps = [],
)

envoy_cc_library(
    name = "envoy_types_lib",
    hdrs = ["types.h"],
    repository = "@envoy",
)

cc_library(
    name = "envoy_jni_interface_lib",
    srcs = ["jni_interface.cc"],
    copts = ["-std=c++14"],
    linkopts = [
        "-lm",
        "-llog",
    ],
    deps = [":envoy_main_interface_lib"],
)
