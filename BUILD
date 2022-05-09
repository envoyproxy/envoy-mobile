load("@io_bazel_rules_kotlin//kotlin/internal:toolchains.bzl", "define_kt_toolchain")

licenses(["notice"])  # Apache 2

alias(
    name = "ios_dist",
    actual = "//library/swift:ios_framework",
)

alias(
    name = "android_aar",
    actual = "//library/kotlin/io/envoyproxy/envoymobile:envoy_aar",
    visibility = ["//visibility:public"],
)

aar_import(
    name = "envoy_mobile_android",
    aar = "//library/kotlin/io/envoyproxy/envoymobile:envoy_aar",
    visibility = ["//visibility:public"],
)

alias(
    name = "android_dist_ci",
    actual = "//library/kotlin/io/envoyproxy/envoymobile:envoy_aar_with_artifacts",
)

filegroup(
    name = "android_dist",
    srcs = [
        "//library/kotlin/io/envoyproxy/envoymobile:envoy_aar",
        "//library/kotlin/io/envoyproxy/envoymobile:envoy_aar_objdump_collector",
        "//library/kotlin/io/envoyproxy/envoymobile:envoy_aar_pom_xml",
    ],
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

filegroup(
    name = "editor_config",
    srcs = [".editorconfig"],
    visibility = ["//visibility:public"],
)

genrule(
    name = "kotlin_format",
    srcs = ["//:editor_config"],
    outs = ["kotlin_format.txt"],
    cmd = """
    $(location @kotlin_formatter//file) --android "**/*.kt" \
        --reporter=plain --reporter=checkstyle,output=$@ \
        --editorconfig=$(location //:editor_config)
    """,
    tools = ["@kotlin_formatter//file"],
)

genrule(
    name = "kotlin_format_fix",
    srcs = ["//:editor_config"],
    outs = ["kotlin_format_fix.txt"],
    cmd = """
    $(location @kotlin_formatter//file) -F --android "**/*.kt" \
        --reporter=plain --reporter=checkstyle,output=$@ \
        --editorconfig=$(location //:editor_config)
    """,
    tools = ["@kotlin_formatter//file"],
)
