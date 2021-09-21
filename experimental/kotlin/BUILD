load("@envoy_mobile//bazel:kotlin_test.bzl", "envoy_mobile_jni_kt_test")

envoy_mobile_jni_kt_test(
    name = "quic_stream_test",
    srcs = [
        "QuicStreamTest.kt",
    ],
    native_deps = [
        "//library/common/jni:libjava_jni_lib.so",
        "//library/common/jni:java_jni_lib.jnilib",
    ],
    deps = [
        "//library/kotlin/io/envoyproxy/envoymobile:envoy_interfaces_lib",
    ],
)
