load("@build_bazel_rules_swift//swift:swift.bzl", "swift_binary")

licenses(["notice"])  # Apache 2

swift_binary(
    name = "cli",
    srcs = glob(["*.swift"]),
    deps = ["//library/swift:ios_lib"],
)
