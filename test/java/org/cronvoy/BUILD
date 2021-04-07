load("@io_bazel_rules_kotlin//kotlin:kotlin.bzl", "kt_jvm_test")
load("@rules_jvm_external//:defs.bzl", "artifact")
load("@build_bazel_rules_android//android:rules.bzl", "android_local_test")

licenses(["notice"])  # Apache 2

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

android_local_test(
    name = "cronvoy_test",
    srcs = [
        "CronvoyEngineTest.java",
    ],
    jvm_flags = [
        "-Djava.library.path=library/common/jni",
    ],
    manifest = "AndroidManifest.xml",
    test_class = "org.cronvoy.CronvoyEngineTest",
    deps = EXTERNAL_DEPS + [
        ":cronvoy_android_lib",
    ],
)

android_library(
    name = "cronvoy_android_lib",
    manifest = "AndroidManifest.xml",
    exports = ["//library/java/org/cronvoy"],
    deps = [
        "//library/common/jni:libenvoy_jni.so",
        "//library/java/org/cronvoy",
    ],
)
