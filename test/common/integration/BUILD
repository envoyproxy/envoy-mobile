load("@envoy//bazel:envoy_build_system.bzl", "envoy_cc_test", "envoy_cc_test_library", "envoy_package")

licenses(["notice"])  # Apache 2

envoy_package()

envoy_cc_test(
    name = "client_integration_test",
    srcs = ["client_integration_test.cc"],
    repository = "@envoy",
    deps = [
        "//library/common/extensions/filters/http/local_error:config",
        "//library/common/extensions/filters/http/local_error:filter_cc_proto",
        "//library/common/http:client_lib",
        "//library/common/http:header_utility_lib",
        "//library/common/types:c_types_lib",
        "@envoy//test/common/http:common_lib",
        "@envoy//test/integration:http_integration_lib",
        "@envoy//test/server:utility_lib",
    ],
)

# interface libs for jni implementation
envoy_cc_test_library(
    name = "quic_test_server_interface_lib",
    repository = "@envoy",
    deps = [
        ":quic_test_server_interface_lib_no_stamp",
    ],
)

envoy_cc_test_library(
    name = "quic_test_server_interface_lib_no_stamp",
    srcs = [
        "quic_test_server.cc",
        "quic_test_server.h",
        "quic_test_server_interface.cc",
    ],
    hdrs = ["quic_test_server_interface.h"],
    data = [
        "@envoy//test/config/integration/certs",
    ],
    repository = "@envoy",
    deps = [
        "@envoy//source/common/quic:active_quic_listener_lib",
        "@envoy//source/common/quic:quic_factory_lib",
        "@envoy//source/common/quic:quic_stat_names_lib",
        "@envoy//source/common/stats:isolated_store_lib",
        "@envoy//test:test_pch",
        "@envoy//source/exe:process_wide_lib",
        "@envoy//source/common/common:logger_lib",
        "@envoy//test/integration:autonomous_upstream_lib",
        "@envoy//test/test_common:network_utility_lib",
        "@envoy_api//envoy/extensions/filters/common/matcher/action/v3:pkg_cc_proto",
        "@envoy_api//envoy/extensions/transport_sockets/quic/v3:pkg_cc_proto",
    ] + select({
        "@envoy//bazel:disable_signal_trace": [],
        "//conditions:default": [
            "@envoy//source/common/signal:sigaction_lib",
        ],
    }),
)
