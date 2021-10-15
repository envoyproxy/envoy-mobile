load("@envoy//bazel:envoy_build_system.bzl", "envoy_package")
load("@envoy_mobile//bazel:kotlin_test.bzl", "envoy_mobile_android_test")

licenses(["notice"])  # Apache 2

envoy_package()

android_library(
    name = "testing",
    srcs = [
        "QuicTestServer.java",
    ],
    data = glob(["data/*"]),
    visibility = ["//test:__subpackages__"],
    deps = [
        "//library/java/io/envoyproxy/envoymobile/engine:envoy_base_engine_lib",
        "//library/java/io/envoyproxy/envoymobile/engine:envoy_engine_lib",
        "//library/kotlin/io/envoyproxy/envoymobile:envoy_interfaces_lib",
        "//library/kotlin/io/envoyproxy/envoymobile:envoy_lib",
        "//test/java/org/chromium/net/testing",
        "@maven//:junit_junit",
    ],
)

envoy_mobile_android_test(
    name = "quic_test_server_test",
    srcs = [
        "QuicTestServerTest.java",
    ],
    library_path = "library/common/jni:test/common/jni",
    native_deps = [
        "//library/common/jni:libndk_envoy_jni.so",
        "//library/common/jni:libndk_envoy_jni.jnilib",
        "//test/common/jni:libquic_test_server_jni.so",
        "//test/common/jni:libquic_test_server_jni.jnilib",
    ],
    deps = [
        ":testing",
        "//library/java/io/envoyproxy/envoymobile/engine:envoy_base_engine_lib",
        "//library/java/io/envoyproxy/envoymobile/engine:envoy_engine_lib",
        "//library/kotlin/io/envoyproxy/envoymobile:envoy_interfaces_lib",
        "//library/kotlin/io/envoyproxy/envoymobile:envoy_lib",
        "//test/java/org/chromium/net/testing",
    ],
)
