load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")

def swift_support():
    http_archive(
        name = "build_bazel_apple_support",
        sha256 = "c02a8c902f405e5ea12b815f426fbe429bc39a2628b290e50703d956d40f5542",
        strip_prefix = "apple_support-0.10.0",
        urls = ["https://github.com/bazelbuild/apple_support/archive/0.10.0.tar.gz"],
    )
