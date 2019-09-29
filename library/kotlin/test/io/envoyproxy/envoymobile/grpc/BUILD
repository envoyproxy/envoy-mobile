licenses(["notice"])  # Apache 2

load("//bazel:kotlin_test.bzl", "darwin_jni_kt_test")
load("@io_bazel_rules_kotlin//kotlin:kotlin.bzl", "kt_jvm_test")

darwin_jni_kt_test(
    name = "darwin_grpc_codec_test",
    srcs = [
        "GrpcTest.kt",
    ],
    lib = "//library/common:envoy_jni_interface_lib",
    deps = [
        "//library/kotlin/src/io/envoyproxy/envoymobile:envoy_interfaces_lib",
        "//library/kotlin/test/grpc:envoy_mobile_grpc_java_test_server_lib",
    ],
)
