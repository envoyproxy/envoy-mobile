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

def local_dynamic_jnilib(name, native_lib_name, native_deps):
    jni_lib = "lib{}.jnilib".format(native_lib_name)
    # Generate .jnilib file for OS X look up
    native.genrule(
        name = name,
        cmd = """
        cp $< $@
        """,
        outs = [jni_lib],
        srcs = native_deps,
        visibility = ["//visibility:public"],
    )

def local_dynamic_so(name, native_lib_name, native_deps):
    so_lib = "lib{}.so".format(native_lib_name)
    # Generate .so file for OS X look up
    native.genrule(
        name = name,
        cmd = """
        cp $< $@
        """,
        outs = [so_lib],
        srcs = native_deps,
        visibility = ["//visibility:public"],
    )
