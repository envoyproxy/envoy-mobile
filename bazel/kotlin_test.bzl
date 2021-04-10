load("@io_bazel_rules_kotlin//kotlin:kotlin.bzl", "kt_jvm_test")

def _internal_kt_test(name, srcs, deps = [], data = [], jvm_flags = []):
    # This is to work around the issue where we have specific implementation functionality which
    # we want to avoid consumers to use but we want to unit test
    dep_srcs = []
    for dep in deps:
        # We'll resolve only the targets in `//library/kotlin/io/envoyproxy/envoymobile`
        if dep.startswith("//library/kotlin/io/envoyproxy/envoymobile"):
            dep_srcs.append(dep + "_srcs")
        elif dep.startswith("//library/java/io/envoyproxy/envoymobile"):
            dep_srcs.append(dep + "_srcs")

    kt_jvm_test(
        name = name,
        test_class = "io.envoyproxy.envoymobile.bazel.EnvoyMobileTestSuite",
        srcs = srcs + dep_srcs,
        deps = [
            "//bazel:envoy_mobile_test_suite",
            "@maven//:org_assertj_assertj_core",
            "@maven//:junit_junit",
            "@maven//:org_mockito_mockito_inline",
            "@maven//:org_mockito_mockito_core",
        ] + deps,
        data = data,
        jvm_flags = jvm_flags,
    )

# A basic macro to make it easier to declare and run kotlin tests which depend on a JNI lib
# This will create the native .so binary (for linux) and a .jnilib (for OS X) look up
def envoy_mobile_jni_kt_test(name, srcs, native_lib_name = "", native_dep = [], deps = []):
    jni_lib = "lib{}.jnilib".format(native_lib_name)
    so_lib = "lib{}.so".format(native_lib_name)

    # Generate .jnilib file for OS X look up
    native.genrule(
        name = jni_lib,
        cmd = """
        cp $< $@
        """,
        outs = [jni_lib],
        srcs = native_dep,
        visibility = ["//visibility:public"],
    )

        # Generate .jnilib file for OS X look up
    native.genrule(
        name = so_lib,
        cmd = """
        cp $< $@
        """,
        outs = [so_lib],
        srcs = native_dep,
        visibility = ["//visibility:public"],
    )

    _internal_kt_test(name, srcs, deps, data = [jni_lib, so_lib], jvm_flags = ["-Djava.library.path=../.."])

# A basic macro to make it easier to declare and run kotlin tests
#
# Ergonomic improvements include:
# 1. Avoiding the need to declare the test_class which requires a fully qualified class name (example below)
# 2. Avoiding the need to redeclare common unit testing dependencies like JUnit
# 3. Ability to run more than one test file per target
# 4. Ability to test internal envoy mobile entities
# Usage example:
# load("//bazel:kotlin_test.bzl", "envoy_mobile_kt_test)
#
# envoy_mobile_kt_test(
#     name = "example_kotlin_test",
#     srcs = [
#         "ExampleTest.kt",
#     ],
# )
def envoy_mobile_kt_test(name, srcs, deps = []):
    _internal_kt_test(name, srcs, deps)
