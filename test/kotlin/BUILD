load("@envoy_mobile//bazel:kotlin_test.bzl", "envoy_mobile_jni_kt_test")

envoy_mobile_jni_kt_test(
    name = "engine_start_test",
    srcs = [
        "EnvoyEngineExampleTest.kt",
    ],
    lib_name = "main_lib",
    data = ["//library/common/jni:envoy_jni"],
    deps = [
        "//library/kotlin/io/envoyproxy/envoymobile:envoy_interfaces_lib",
    ],
)

cc_binary(
    name = "main_lib.so",
    srcs = ["main.cc"],
    copts = ["-std=c++14"],
    linkshared = True,
    linkopts = [
        "-lm",
    ],
    deps = ["//bazel:jni"],
)