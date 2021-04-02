load("@io_bazel_rules_kotlin//kotlin:kotlin.bzl", "kt_jvm_test")

java_binary(
  name = "app",
  srcs = ["App.java"],
  main_class = 'test.App',
  data = [":libnative.so", ":libnative.jnilib"],
  jvm_flags = ["-Djava.library.path=test"],
)

genrule(
    name = "libnative.jnilib",
    outs = ["libnative.jnilib"],
    srcs = ["libnative.so"],
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
    srcs = ["AppTest.kt", "App.java"],
    data = [":libnative.so", ":libnative.jnilib"],
    test_class = "io.envoyproxy.envoymobile.bazel.EnvoyMobileTestSuite",
    deps = [
        "app",
        "//bazel:envoy_mobile_test_suite",
        "@maven//:org_assertj_assertj_core",
        "@maven//:junit_junit",
        "@maven//:org_mockito_mockito_inline",
        "@maven//:org_mockito_mockito_core",

    ],
    jvm_flags = ["-Djava.library.path=test"],
)