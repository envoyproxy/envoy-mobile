licenses(["notice"])  # Apache 2

load("@envoy//bazel:envoy_build_system.bzl", "envoy_cc_library", "envoy_package")

envoy_package()

envoy_cc_library(
    name = "envoy_main_interface_lib",
    srcs = [
        "certificates.inc",
        "config_template.cc",
        "main_interface.cc",
    ],
    hdrs = ["main_interface.h"],
    repository = "@envoy",
    deps = [
        "//library/common/buffer:utility_lib",
        "//library/common/http:dispatcher_lib",
        "//library/common/http:header_utility_lib",
        "//library/common/types:c_types_lib",
        "@envoy//include/envoy/server:lifecycle_notifier_interface",
        "@envoy//source/exe:envoy_main_common_lib",
    ],
)

cc_library(
    name = "envoy_jni_interface_lib",
    srcs = [
        "jni_interface.cc",
    ],
    copts = ["-std=c++14"],
    linkopts = [
        "-lm",
        "-llog",
    ],
    deps = [":envoy_main_interface_lib"],
)


cc_library(
    name = "envoy_java_jni_interface_lib",
    srcs = [
        "jni_interface.cc",
        "@local_jdk//:jni_header",
#        "@local_jdk//:jni_md_header-linux",
    ],
    copts = ["-std=c++14"],
    linkopts = [
        "-lm",
#        "-llog",
    ],
    deps = [":envoy_main_interface_lib"],
    includes = [
        "../../external/local_jdk/include",
        "../../external/local_jdk/include/darwin"
    ],
)

cc_binary(
    name = "envoy_jni.so",
    deps = [ "envoy_java_jni_interface_lib" ],
    visibility = ["//visibility:public"],
    linkshared = 1,
)