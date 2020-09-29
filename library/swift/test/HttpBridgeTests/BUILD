load("@envoy_mobile//bazel:swift_test.bzl", "envoy_mobile_swift_test")

licenses(["notice"])  # Apache 2

envoy_mobile_swift_test(
    name = "cancel_stream_test",
    srcs = [
        "CancelStreamTest.swift",
        "DemoFilter.swift",
    ],
    deps = [
        "//library/objective-c:envoy_engine_objc_lib",
    ],
)

envoy_mobile_swift_test(
    name = "receive_data_test",
    srcs = [
        "ReceiveDataTest.swift",
        "DemoFilter.swift",
    ],
    deps = [
        "//library/objective-c:envoy_engine_objc_lib",
    ],
)

envoy_mobile_swift_test(
    name = "receive_error_test",
    srcs = [
        "ReceiveErrorTest.swift",
        "DemoFilter.swift",
    ],
    deps = [
        "//library/objective-c:envoy_engine_objc_lib",
    ],
)

envoy_mobile_swift_test(
    name = "send_data_test",
    srcs = [
        "SendDataTest.swift",
        "DemoFilter.swift",
    ],
    deps = [
        "//library/objective-c:envoy_engine_objc_lib",
    ],
)

envoy_mobile_swift_test(
    name = "send_headers_test",
    srcs = [
        "SendHeadersTest.swift",
        "DemoFilter.swift",
    ],
    deps = [
        "//library/objective-c:envoy_engine_objc_lib",
    ],
)

envoy_mobile_swift_test(
    name = "send_trailers_test",
    srcs = [
        "SendTrailersTest.swift",
        "DemoFilter.swift",
    ],
    deps = [
        "//library/objective-c:envoy_engine_objc_lib",
    ],
)