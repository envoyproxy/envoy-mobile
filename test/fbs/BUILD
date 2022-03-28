load("@envoy//bazel:envoy_build_system.bzl", "envoy_package")
load("@com_github_google_flatbuffers//:build_defs.bzl", "flatbuffer_library_public", "flatbuffer_cc_library")
load("@rules_java//java:defs.bzl", "java_library")

licenses(["notice"])  # Apache 2

envoy_package()

flatbuffer_cc_library(
    name = "test_cc_fbs",
    srcs = ["test.fbs"],
)

flatbuffer_library_public(
    name = "test_java_fbs_srcs",
    outs = ["Test/SomeType.java"],
    srcs = ["test.fbs"],
    out_prefix = ["Test"],
    language_flag = "-j",
)

java_library(
    name = "test_java_fbs",
    srcs = [":test_java_fbs_srcs"],
    deps = ["@maven//:com_google_flatbuffers_flatbuffers_java"],
)

flatbuffer_library_public(
    name = "test_swift_fbs_srcs",
    outs = [
	    "test_generated.swift",
    ],
    srcs = ["test.fbs"],
    language_flag = "--swift",
)
