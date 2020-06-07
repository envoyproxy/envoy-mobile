load("@io_bazel_rules_kotlin//kotlin/internal:toolchains.bzl", "define_kt_toolchain")

licenses(["notice"])  # Apache 2

alias(
    name = "ios_framework",
    actual = "@envoy_mobile//library/swift/src:ios_framework",
)

genrule(
    name = "ios_dist",
    srcs = [":ios_framework"],
    outs = ["ios_out"],
    cmd = """
unzip -o $< -d dist/
touch $@
""",
    stamp = True,
)

alias(
    name = "android_aar",
    actual = "@envoy_mobile//library/kotlin/src/io/envoyproxy/envoymobile:android_aar_only_aar",
)

genrule(
    name = "javadocs",
    srcs = [],
    outs = ["stupid-javadoc.jar"],
    cmd = """
orig_dir=$$PWD
tmp_dir=$$(mktemp -d)
java -jar $(location @kotlin_dokka//jar) \
    $$orig_dir/library/java/src/ \
    $$orig_dir/library/kotlin/src/ \
    -format javadoc \
    -output $$tmp_dir > /dev/null
cd $$tmp_dir
zip -r $$orig_dir/$@ . > /dev/null
    """,
    tools = ["@kotlin_dokka//jar"],
    visibility = ["//visibility:public"],
)

genrule(
    name = "android_zip",
    srcs = [
        "@envoy_mobile//library/kotlin/src/io/envoyproxy/envoymobile:android_aar",
    ],
    outs = ["envoy_mobile.zip"],
    cmd = "$(location @envoy_mobile//bazel:zipper) fc $@ $(SRCS)",
    stamp = True,
    tools = ["@envoy_mobile//bazel:zipper"],
    visibility = ["//visibility:public"],
)

genrule(
    name = "android_dist",
    srcs = [
        "@envoy_mobile//library/kotlin/src/io/envoyproxy/envoymobile:android_aar",
    ],
    outs = ["output_in_dist_directory"],
    cmd = """
    for artifact in $(SRCS); do
        chmod 755 $$artifact
        cp $$artifact dist/
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
