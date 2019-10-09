licenses(["notice"])  # Apache 2

load("//bazel:kotlin_test.bzl", "envoy_mobile_jni_kt_test")

envoy_mobile_jni_kt_test(
    name = "grpc_codec_test",
    srcs = [
        "GrpcTest.kt",
    ],
    lib = "//library/common:envoy_jni_interface_lib_for_tests",
    deps = [
        "//library/kotlin/test/grpc:envoy_mobile_grpc_java_test_server_lib",
        "//library/kotlin/test/grpc:envoy_mobile_test_proto",
        "//library/kotlin/src/io/envoyproxy/envoymobile:envoy_interfaces_lib"
    ],
)
