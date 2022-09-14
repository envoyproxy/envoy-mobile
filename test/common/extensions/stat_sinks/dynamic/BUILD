load("@envoy//bazel:envoy_build_system.bzl", "envoy_package")
load(
    "@envoy//test/extensions:extensions_build_system.bzl",
    "envoy_extension_cc_test",
)

licenses(["notice"])  # Apache 2

envoy_package()

envoy_extension_cc_test(
    name = "sink_test",
    srcs = ["sink_test.cc"],
    data = [
        ":shared_library",
    ],
    extension_names = ["envoy.stat_sinks.dynamic"],
    repository = "@envoy",
    deps = [
        "//library/common/extensions/stat_sinks/dynamic:sink_lib",
    ],
)

envoy_extension_cc_test(
    name = "config_test",
    srcs = ["config_test.cc"],
    extension_names = ["envoy.stat_sinks.dynamic"],
    repository = "@envoy",
    deps = [
        "//library/common/extensions/stat_sinks/dynamic:config",
    ],
)

envoy_extension_cc_test(
    name = "no_symbol_sink_test",
    srcs = ["sink_test.cc"],
    data = [
        ":shared_library",
    ],
    extension_names = ["envoy.stat_sinks.dynamic"],
    repository = "@envoy",
    deps = [
        "//library/common/extensions/stat_sinks/dynamic:sink_lib",
    ],
)

cc_binary(
    name = "shared_library",
    srcs = ["shared.cc"],
    linkshared = True,
    deps = [
        "//library/common/extensions/stat_sinks/dynamic:dynamic_sink_interface",
    ],
)
