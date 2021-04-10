load("@envoy_mobile//bazel:kotlin_test.bzl", "envoy_mobile_jni_kt_test")

envoy_mobile_jni_kt_test(
    name = "engine_start_test",
    srcs = [
        "EnvoyEngineSimpleIntegrationTest.kt",
    ],
    native_dep = ["//library/common/jni:java_jni_lib.so"],
    native_lib_name = "envoy_jni",
    deps = [
        "//library/kotlin/io/envoyproxy/envoymobile:envoy_interfaces_lib",
    ],
)
