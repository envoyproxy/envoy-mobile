load("@envoy//bazel:envoy_build_system.bzl", "envoy_package")
load("@com_github_google_flatbuffers//:build_defs.bzl", "flatbuffer_cc_library", "flatbuffer_library_public")
load("@envoy_mobile//bazel:kotlin_lib.bzl", "envoy_mobile_kt_library")

licenses(["notice"])  # Apache 2

envoy_package()

flatbuffer_cc_library(
    name = "test_cc_fbs",
    srcs = ["test.fbs"],
)

flatbuffer_library_public(
    name = "test_kt_fbs_srcs",
    srcs = ["test.fbs"],
    # TODO(snowp): For some reason I can't get this to work with just one output file for java. For now we can avoid dealing with this since basically all use cases of this would result in multiple files for Java.
    outs = [
        "Test/SomeOtherType.kt",
        "Test/SomeType.kt",
    ],
    language_flag = "--kotlin",
)

envoy_mobile_kt_library(
    name = "test_kt_lib",
    srcs = [":test_kt_fbs_srcs"],
    deps = ["@maven//:com_google_flatbuffers_flatbuffers_java"],
)

flatbuffer_library_public(
    name = "test_swift_fbs_srcs",
    srcs = ["test.fbs"],
    outs = [
        "test_generated.swift",
    ],
    language_flag = "--swift",
)
