load("@io_bazel_rules_kotlin//kotlin/internal:toolchains.bzl", "define_kt_toolchain")

licenses(["notice"])  # Apache 2

alias(
    name = "ios_framework",
    actual = "//library/swift/src:ios_framework",
)

genrule(
    name = "ios_dist",
    srcs = ["//:ios_framework"],
    outs = ["ios_out"],
    cmd = """
unzip -o $< -d dist/
touch $@
""",
    stamp = True,
)

alias(
    name = "android_aar",
    actual = "//library/kotlin/src/io/envoyproxy/envoymobile:android_aar",
)

genrule(
    name = "android_zip",
    srcs = [
        "android_aar",
    ],
    visibility = ["//visibility:public"],
    tools = ["@bazel_tools//tools/zip:zipper"],
    outs = ["envoy_mobile.zip"],
    cmd = "$(location @bazel_tools//tools/zip:zipper) fc $@ $(SRCS)",
    stamp = True
)

genrule(
    name = "android_dist",
    srcs = [
        "android_aar",
    ],
    outs = ["output_in_dist_directory"],
    cmd = """
    for artifact in $(SRCS); do
        chmod 755 $$artifact
        mv $$artifact dist/
    done
    touch $@
    """,
    stamp = True,
)

define_kt_toolchain(
    name = "kotlin_toolchain",
    jvm_target = "1.8",
)

filegroup(
    name = "kotlin_lint_config",
    srcs = [".kotlinlint.yml"],
    visibility = ["//visibility:public"],
)
