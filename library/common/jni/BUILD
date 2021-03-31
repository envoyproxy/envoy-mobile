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
        "//library/common/types:c_types_lib",
        "@envoy//source/common/common:assert_lib",
    ],
)

cc_binary(
    name = "libenvoy_jni.so",
    srcs = [
        "android_jni_interface.cc",
        "jni_interface.cc",
    ],
    copts = ["-std=c++17"],
    linkopts = [
        "-lm",
        "-llog",
        "-Wl,-s",
    ],
    linkshared = True,
    deps = [
        ":jni_utility_lib",
        "//library/common:envoy_main_interface_lib",
        "//library/common/api:c_types",
    ],
)

cc_library(
    name = "envoy_android_jni_lib",
    srcs = [
        "jni_interface.cc",
    ],
    hdrs = ["jni_support.h"],
    copts = ["-std=c++14"],
    linkopts = [
        "-lm",
        "-llog",
    ],
    deps = [":envoy_main_interface_lib"],
)

cc_library(
    name = "envoy_java_jni_lib",
    srcs = [
        "jni_interface.cc",
        "jni_utility.cc",
        "jni_version.cc",
        "@local_jdk//:jni_header",
    ]
#    + select({
#        "//bazel:darwin": ["@local_jdk//:jni_md_header-darwin"],
#        "//bazel:darwin_x86_64": ["@local_jdk//:jni_md_header-darwin"],
#        "//bazel:linux_x86_64": ["@local_jdk//:jni_md_header-linux"],
#    })
    ,
    hdrs = [
        "jni_support.h",
        "jni_utility.h",
        "jni_version.h",
    ],
    copts = ["-std=c++14"],
    includes = [
        "../../../external/local_jdk/include",
        "../../../external/local_jdk/include/darwin",
        "../../../external/local_jdk/include/linux",
    ],
    linkopts = [
        "-lm",
    ],
    deps = [
        "//library/common:envoy_main_interface_lib",
        "//library/common/types:c_types_lib",
        "@envoy//source/common/common:assert_lib",
    ],
)
