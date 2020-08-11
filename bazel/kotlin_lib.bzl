load("@io_bazel_rules_kotlin//kotlin:kotlin.bzl", "kt_jvm_library")

def envoy_mobile_kt_library(name, visibility = None, srcs = [], deps = []):
    # These source files must be re-exported to the kotlin custom library rule to ensure their
    # inclusion. This is used to work around testing visibility.
    native.filegroup(
        name = name + "_srcs",
        srcs = srcs,
        visibility = visibility,
    )

    kt_jvm_library(
        name = name,
        srcs = srcs,
        deps = deps,
        visibility = visibility,
    )
