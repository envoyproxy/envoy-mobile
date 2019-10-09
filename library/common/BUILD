licenses(["notice"])  # Apache 2

load("@envoy//bazel:envoy_build_system.bzl", "envoy_cc_library", "envoy_package")
load("@envoy//bazel:envoy_build_system.bzl", "envoy_cc_library", "envoy_package")

envoy_package()

envoy_cc_library(
    name = "envoy_main_interface_lib",
    srcs = [
        "certificates.inc",
        "config_template.cc",
        "engine.cc",
        "engine.h",
        "main_interface.cc",
    ],
    hdrs = ["main_interface.h"],
    repository = "@envoy",
    deps = [
        ":envoy_mobile_main_common_lib",
        "//library/common/buffer:utility_lib",
        "//library/common/extensions:registry",
        "//library/common/http:dispatcher_lib",
        "//library/common/http:header_utility_lib",
        "//library/common/types:c_types_lib",
        "@envoy//include/envoy/server:lifecycle_notifier_interface",
    ],
)

envoy_cc_library(
    name = "envoy_mobile_main_common_lib",
    srcs = ["envoy_mobile_main_common.cc"],
    hdrs = ["envoy_mobile_main_common.h"],
    repository = "@envoy",
    deps = [
        "@envoy//source/common/runtime:runtime_lib",
        "@envoy//source/exe:envoy_common_lib",
        "@envoy//source/exe:envoy_main_common_lib",
    ],
)
#
#cc_binary(
#    name = "libenvoy_jni.so",
cc_library(
    name = "android_envoy_jni_interface_lib",
    srcs = [
        "android_support.cc",
        "jni_interface.cc",
    ],
    hdrs = ["jni_support.h"],
    copts = ["-std=c++14"],
    linkopts = [
        "-lm",
        "-llog",
    ],
#    linkshared = True,
    deps = [":envoy_main_interface_lib"],
)

cc_library(
    name = "java_envoy_jni_interface_lib",
    srcs = [
        "java_support.cc",
        "jni_interface.cc",
        "@local_jdk//:jni_header",
    ] + select({
        "//bazel:darwin": ["@local_jdk//:jni_md_header-darwin"],
        "//bazel:darwin_x86_64": ["@local_jdk//:jni_md_header-darwin"],
        "//bazel:linux_x86_64": ["@local_jdk//:jni_md_header-linux"],
    }),
    hdrs = ["jni_support.h"],
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
