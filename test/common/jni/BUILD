load("@envoy//bazel:envoy_build_system.bzl", "envoy_package")

licenses(["notice"])  # Apache 2

envoy_package()

# dynamic library for the Envoy Mobile aar
cc_binary(
    name = "libquic_test_server_jni.so",
    testonly = True,
    srcs = [
        "quic_test_server_jni_interface.cc",
        "@local_jdk//:jni_header",
    ],
    copts = ["-std=c++17"],
    linkopts = ["-lm"],
    linkshared = True,
    deps = [
        "//library/common:envoy_main_interface_lib",
        "//library/common/api:c_types",
        "//library/common/jni:base_java_jni_lib",
        "//test/common/integration:quic_test_server_interface_lib",
        "@envoy_mobile_extra_jni_deps//:extra_jni_dep",
    ],
)
