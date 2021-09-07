load("//bazel:kotlin_lib.bzl", "envoy_mobile_so_to_jni_lib")
load("@rules_cc//cc:defs.bzl", "cc_library", "cc_test")
load("@envoy//bazel:envoy_build_system.bzl", "envoy_cc_library", "envoy_package")

licenses(["notice"])  # Apache 2

envoy_package()

### Targets for local execution
## OS X binary (.jnilib) for NDK testing
#envoy_mobile_so_to_jni_lib_test(
#    name = "libquic_test_server_jni.jnilib",
#    native_dep = "libquic_test_server_jni.so",
#)

# dynamic library for the Envoy Mobile aar
cc_binary(
    name = "libquic_test_server_jni.so",
    testonly = True,
    srcs = [
        "quic_test_server_jni_interface.cc",
        "@local_jdk//:jni_header",
    ],
    copts = ["-std=c++17"],
    linkopts = [
    ],
    linkshared = True,
    deps = [
        "//bazel:jni",
        "//library/common:envoy_main_interface_lib",
        "//library/common/api:c_types",
        "//library/common/jni:base_java_jni_lib",
        "//library/common/jni:java_jni_support",
        "//test/common/integration:quic_test_server_interface_lib",
    ],
)
