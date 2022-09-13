load("@envoy_mobile//bazel:kotlin_lib.bzl", "envoy_mobile_kt_library")
load("@envoy_mobile//bazel:kotlin_test.bzl", "envoy_mobile_android_test")

envoy_mobile_kt_library(
    name = "proxy_lib",
    srcs = ["Proxy.kt"],
    visibility = ["//visibility:public"],
    deps = [
        # "//library/java/io/envoyproxy/envoymobile/engine:envoy_base_engine_lib",
        "//library/kotlin/io/envoyproxy/envoymobile:envoy_interfaces_lib",
        "//library/kotlin/io/envoyproxy/envoymobile:envoy_lib",
        # "@maven//:org_assertj_assertj_core",
    ],
)

envoy_mobile_android_test(
    name = "perform_http_request_using_proxy",
    srcs = [
        "PerformHTTPRequestUsingProxy.kt",
    ],
    native_deps = [
        "//library/common/jni:libndk_envoy_jni.so",
        "//library/common/jni:libndk_envoy_jni.jnilib",
    ],
    deps = [
        "//library/kotlin/io/envoyproxy/envoymobile:envoy_interfaces_lib",
        "//library/kotlin/io/envoyproxy/envoymobile:envoy_lib",
        "//test/kotlin/integration/proxy:proxy_lib",
    ],
)

envoy_mobile_android_test(
    name = "perform_https_request_using_proxy",
    srcs = [
        "PerformHTTPSRequestUsingProxy.kt",
    ],
    native_deps = [
        "//library/common/jni:libndk_envoy_jni.so",
        "//library/common/jni:libndk_envoy_jni.jnilib",
    ],
    deps = [
        "//library/kotlin/io/envoyproxy/envoymobile:envoy_interfaces_lib",
        "//library/kotlin/io/envoyproxy/envoymobile:envoy_lib",
        "//test/kotlin/integration/proxy:proxy_lib",
    ],
)