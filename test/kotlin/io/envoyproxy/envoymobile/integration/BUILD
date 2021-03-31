load("@envoy_mobile//bazel:kotlin_test.bzl", "envoy_mobile_jni_kt_test")

licenses(["notice"])  # Apache 2

envoy_mobile_jni_kt_test(
    name = "jni_test",
    srcs = [
        "FooTest.kt",
    ],
    lib = "//library/common/jni:envoy_java_jni_lib",
    deps = [
        "//library/kotlin/io/envoyproxy/envoymobile:envoy_interfaces_lib",
    ],
)