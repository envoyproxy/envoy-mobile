licenses(["notice"])  # Apache 2

load("//bazel:kotlin_lib.bzl", "envoy_mobile_android_library")
load("//bazel:envoy_build_system.bzl", "envoy_package")

envoy_package()

envoy_mobile_android_library(
    name = "envoy_engine_lib",
    srcs = ["EnvoyEngine.java"],
    custom_package = "io.envoyproxy.envoymobile.engine",
    manifest = "EnvoyEngineManifest.xml",
    deps = ["//library/common:envoy_jni_interface_lib"],
)
