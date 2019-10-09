licenses(["notice"])  # Apache 2

load("@envoy//bazel:envoy_build_system.bzl", "envoy_cc_library", "envoy_package")
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
        "android_jni_utils.cc",
        "jni_interface.cc",
    ],
    hdrs = ["jni_utils.h"],
    copts = ["-std=c++14"],
    linkopts = [
        "-lm",
        "-llog",
    ],
    deps = [":envoy_main_interface_lib"],
)

cc_library(
    name = "envoy_jni_interface_lib_for_tests",  # TODO: Name better
    srcs = [
        "java_jni_utils.cc",
        "jni_interface.cc",
        "@local_jdk//:jni_header",
    ] + select({
        "@bazel_tools//src/conditions:darwin": ["@local_jdk//:jni_md_header-darwin"],
        "@bazel_tools//src/conditions:darwin_x86_64": ["@local_jdk//:jni_md_header-darwin"],
        "@bazel_tools//src/conditions:linux_x86_64": ["@local_jdk//:jni_md_header-linux"],
    }),
    hdrs = ["jni_utils.h"],
    copts = ["-std=c++14"],
    includes = [
        "../../external/local_jdk/include",
        "../../external/local_jdk/include/darwin",
        "../../external/local_jdk/include/linux",
    ],
    linkopts = [
        "-lm",
    ],
    deps = [":envoy_main_interface_lib"],
)
