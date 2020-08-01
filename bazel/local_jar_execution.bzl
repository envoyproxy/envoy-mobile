load("@rules_cc//cc:defs.bzl", "cc_binary")

def local_jar_cc_binary(
        name,
        srcs = [],
        includes = [],
        deps = [],):
    cc_binary(
        name = name,
        srcs = srcs,
    copts = ["-std=c++17"],
    includes = includes,
    linkopts = [
        "-lm",
    ],
    linkshared = True,
    toolchains = ["@bazel_tools//tools/jdk:current_java_runtime"],
        deps = deps,
    )
