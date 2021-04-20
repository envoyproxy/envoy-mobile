load("@envoy_mobile//bazel:swift_test.bzl", "envoy_mobile_swift_test")

licenses(["notice"])  # Apache 2

envoy_mobile_swift_test(
    name = "counter_impl_tests",
    srcs = [
        "CounterImplTests.swift",
    ],
    deps = [
        "//library/objective-c:envoy_engine_objc_lib",
    ],
)

envoy_mobile_swift_test(
    name = "distribution_impl_tests",
    srcs = [
        "DistributionImplTests.swift",
    ],
    deps = [
        "//library/objective-c:envoy_engine_objc_lib",
    ],
)

envoy_mobile_swift_test(
    name = "element_tests",
    srcs = [
        "ElementTests.swift",
    ],
)

envoy_mobile_swift_test(
    name = "gauge_impl_tests",
    srcs = [
        "GaugeImplTests.swift",
    ],
    deps = [
        "//library/objective-c:envoy_engine_objc_lib",
    ],
)

envoy_mobile_swift_test(
    name = "timer_impl_tests",
    srcs = [
        "TimerImplTests.swift",
    ],
    deps = [
        "//library/objective-c:envoy_engine_objc_lib",
    ],
)
