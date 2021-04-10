load("@envoy_mobile//bazel:kotlin_test.bzl", "envoy_mobile_android_test", "envoy_mobile_jni_kt_test")

#envoy_mobile_jni_kt_test(
#    name = "engine_start_test",
#    srcs = [
#        "EnvoyEngineSimpleIntegrationTest.kt",
#    ],
#    native_deps = [
#        "//library/common/jni:envoy_jni_jnilib",
#        "//library/common/jni:envoy_jni_so",
#    ],
#    native_lib_name = "envoy_jni",
#    deps = [
#        "//library/kotlin/io/envoyproxy/envoymobile:envoy_interfaces_lib",
#    ],
#)

envoy_mobile_android_test(
    name = "android_engine_start_test",
    srcs = [
        "AndroidEnvoyEngineSimpleIntegrationTest.kt"
    ],
    native_lib_name = "envoy_jni",
    native_deps = [
#        "//library/common/jni:envoy_jni_jnilib",
#        "//library/common/jni:envoy_jni_so",
    ],
    deps = [
        "//library/kotlin/io/envoyproxy/envoymobile:envoy_interfaces_lib",
    ],
)