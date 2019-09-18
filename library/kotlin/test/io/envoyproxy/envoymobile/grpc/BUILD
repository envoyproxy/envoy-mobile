licenses(["notice"])  # Apache 2

load("//bazel:kotlin_test.bzl", "envoy_mobile_kt_test")
load("@io_bazel_rules_kotlin//kotlin:kotlin.bzl", "kt_jvm_test")

kt_jvm_test(
    name = "grpc_codec_test",
    test_class = "io.envoyproxy.envoymobile.bazel.EnvoyMobileTestSuite",
    srcs = [
        "GrpcTest.kt",
    ],
    deps = [
        "//library/kotlin/test/grpc:envoy_mobile_grpc_java_test_server_lib",

#        "//library/java/src/io/envoyproxy/envoymobile/engine:envoy_jvm_base_engine_lib", # jni lib
        "//library/kotlin/src/io/envoyproxy/envoymobile:envoy_kt_jvm_lib",

        "//bazel:envoy_mobile_test_suite",
        "@maven//:org_assertj_assertj_core",
        "@maven//:junit_junit",
        "@maven//:org_mockito_mockito_inline",
        "@maven//:org_mockito_mockito_core",
    ],
)
#
#
#kt_jvm_test(
#    name = name,
#    test_class = "io.envoyproxy.envoymobile.bazel.EnvoyMobileTestSuite",
#    srcs = srcs + dep_srcs,
#    deps = [
#        "//bazel:envoy_mobile_test_suite",
#        "@maven//:org_assertj_assertj_core",
#        "@maven//:junit_junit",
#        "@maven//:org_mockito_mockito_inline",
#        "@maven//:org_mockito_mockito_core",
#    ] + deps,
#)