load("@envoy//bazel:envoy_build_system.bzl", "envoy_cc_extension", "envoy_extension_package")
load("@envoy_api//bazel:api_build_system.bzl", "api_proto_package")

licenses(["notice"])  # Apache 2

envoy_extension_package()

envoy_cc_extension(
    name = "assertion_filter_lib",
    srcs = ["filter.cc"],
    hdrs = ["filter.h"],
    security_posture = "requires_trusted_downstream_and_upstream",
    repository = "@envoy",
    deps = [
        ":pkg_cc_proto",
        "@envoy//include/envoy/http:codes_interface",
        "@envoy//include/envoy/http:filter_interface",
        "@envoy//source/common/http:header_map_lib",
        "@envoy//source/extensions/common/matcher:matcher_lib",
        "@envoy//source/extensions/filters/http/common:pass_through_filter_lib",
    ],
)

envoy_cc_extension(
    name = "config",
    srcs = ["config.cc"],
    hdrs = ["config.h"],
    security_posture = "requires_trusted_downstream_and_upstream",
    repository = "@envoy",
    deps = [
        ":assertion_filter_lib",
        ":pkg_cc_proto",
        "@envoy//source/extensions/filters/http/common:factory_base_lib",
    ],
)
