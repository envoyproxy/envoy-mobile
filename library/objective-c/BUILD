licenses(["notice"])  # Apache 2

load("@build_bazel_rules_apple//apple:ios.bzl", "ios_application")

exports_files(["EnvoyEngine.h"])

filegroup(
    name = "envoy_engine_hdrs",
    srcs = ["EnvoyEngine.h"],
    visibility = ["//visibility:public"],
)

objc_library(
    name = "envoy_engine_objc_lib",
    srcs = [
        "EnvoyEngine.mm",
    ],
    hdrs = [
        "EnvoyEngine.h",
    ],
    copts = ["-std=c++14"],
    visibility = ["//visibility:public"],
    deps = ["//library/common:envoy_main_interface_lib"],
)

objc_library(
    name = "appmain",
    srcs = [
        "AppDelegate.h",
        "AppDelegate.mm",
        "main.mm",
    ],
    copts = ["-std=c++14"],
    data = ["config.yaml"],
    sdk_dylibs = [
        "resolv.9",
        "c++",
    ],
    deps = ["envoy_engine_objc_lib"],
)

ios_application(
    name = "app",
    bundle_id = "com.foo.bar",
    families = ["iphone"],
    infoplists = ["Info.plist"],
    minimum_os_version = "11.0",
    deps = ["appmain"],
)
