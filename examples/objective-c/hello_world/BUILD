licenses(["notice"])  # Apache 2

load("@build_bazel_rules_apple//apple:ios.bzl", "ios_application", "ios_framework", "ios_static_framework")

objc_library(
    name = "appmain",
    srcs = glob([
        "*.h",
        "*.mm",
    ]),
    copts = ["-std=c++14"],
    data = ["config.yaml"],
    sdk_dylibs = [
        "resolv.9",
        "c++",
    ],
    deps = ["//library/objective-c:envoy_engine_objc_lib"],
)

ios_application(
    name = "app",
    bundle_id = "io.envoyproxy.envoymobile.helloworld",
    families = ["iphone"],
    infoplists = ["Info.plist"],
    minimum_os_version = "10.0",
    deps = ["appmain"],
)
