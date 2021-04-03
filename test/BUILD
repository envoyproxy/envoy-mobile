load("@io_bazel_rules_kotlin//kotlin:kotlin.bzl", "kt_jvm_test", "kt_jvm_library")

kt_jvm_library(
  name = "app",
  srcs = ["App.kt"],
)

genrule(
    name = "java_jni_lib.jnilib",
    outs = ["libenvoy_jni.jnilib"],
    srcs = ["//library/common/jni:java_libenvoy_jni.so"],
    cmd = """
    cp -f $< $@
    """
)

cc_binary(
  name = "libnative.so",
  srcs = ["native.cc"],
  linkshared = 1,
  deps = ["@bazel_tools//tools/jdk:jni"],
)

kt_jvm_test(
    name = "app_test",
    srcs = ["AppTest.kt"],
    data = ["//library/common/jni:java_libenvoy_jni.so", "java_jni_lib.jnilib"],
    test_class = "io.envoyproxy.envoymobile.bazel.EnvoyMobileTestSuite",
    deps = [
        "//library/kotlin/io/envoyproxy/envoymobile:envoy_interfaces_lib",
        "//bazel:envoy_mobile_test_suite",
        "@maven//:org_assertj_assertj_core",
        "@maven//:junit_junit",
        "@maven//:org_mockito_mockito_inline",
        "@maven//:org_mockito_mockito_core",

    ],
    jvm_flags = ["-Djava.library.path=test"],
)