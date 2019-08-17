"""
This rule provides a custom test runner for iOS tests,
allowing us to force specific versions of iOS and certain simulators.
"""

load(
    "@build_bazel_rules_apple//apple/testing/default_runner:ios_test_runner.bzl",
    "ios_test_runner",
)

ios_test_runner(
    name = "envoy_ios_test_runner",
    device_type = "iPhone Xs",
    os_version = "12.4",
)
