licenses(["notice"])  # Apache 2

load("@io_bazel_rules_kotlin//kotlin:kotlin.bzl", "kt_android_library")

android_binary(
    name = "hello_envoy",
    custom_package = "io.envoyproxy.envoymobile.helloenvoy",
    manifest = "AndroidManifest.xml",
    deps = [
        "hello_envoy_java_lib"
    ],
)


kt_android_library(
    name = "hello_envoy_java_lib",
    srcs = [
        "MainActivity.java",
        "EmptyFile.kt",
    ],
    custom_package = "io.envoyproxy.envoymobile.helloenvoy",
    manifest = "AndroidManifest.xml",
    resource_files = [
        "res/layout/activity_main.xml",
    ],
    deps = [
        "//dist:envoy_mobile_android",
        "//examples/kotlin/shared:hello_envoy_shared_lib",
        "@androidsdk//com.android.support:appcompat-v7-25.0.0",
        "@androidsdk//com.android.support:recyclerview-v7-25.0.0",
    ],
)
