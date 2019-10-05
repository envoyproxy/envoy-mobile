licenses(["notice"])  # Apache 2

load("@io_bazel_rules_kotlin//kotlin/internal:toolchains.bzl", "define_kt_toolchain")

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
    name = "android_dist",
    srcs = ["android_aar"],
    outs = ["android_out"],
    cmd = """
cp $< dist/envoy.aar
chmod 755 dist/envoy.aar
touch $@
""",
    stamp = True,
)

define_kt_toolchain(
    name = "kotlin_toolchain",
    jvm_target = "1.8",
)

config_setting(
    name = "jvm_linux",
    values = {
        "define": "os=linux",
    },
)

config_setting(
    name = "jvm_darwin",
    values = {
        "define": "darwin",
    },
)
