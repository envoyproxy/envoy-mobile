load("@envoy//bazel:envoy_build_system.bzl", "envoy_package")
load("@envoy_mobile//bazel:kotlin_test.bzl", "envoy_mobile_android_test")

licenses(["notice"])  # Apache 2

envoy_package()

EXTERNAL_DEPS = [
    "@maven//:androidx_annotation_annotation",
    "@maven//:androidx_test_core",
    "@maven//:androidx_test_ext_junit",
    "@maven//:androidx_test_runner",
    "@maven//:androidx_test_monitor",
    "@maven//:androidx_test_rules",
    "@maven//:org_assertj_assertj_core",
    "@maven//:org_mockito_mockito_core",
    "@maven//:org_mockito_mockito_inline",
    "@maven//:org_robolectric_robolectric",
    "@robolectric//bazel:android-all",
]

envoy_mobile_android_test(
    name = "cronvoy_test",
    srcs = [
        "CronvoyEngineTest.java",
    ],
    native_deps = [
        "//library/common/jni:libndk_envoy_jni.so",
        "//library/common/jni:libndk_envoy_jni.jnilib",
    ],
    deps = [
        "//library/java/io/envoyproxy/envoymobile/engine:envoy_base_engine_lib",
        "//library/java/io/envoyproxy/envoymobile/engine:envoy_engine_lib",
        "//library/java/org/chromium/net",
        "//library/java/org/cronvoy",
        "//library/kotlin/io/envoyproxy/envoymobile:envoy_interfaces_lib",
        "//library/kotlin/io/envoyproxy/envoymobile:envoy_lib",
    ],
)
