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

def envoy_mobile_so_to_jni_lib(name, native_dep):
    native_lib_name = ""
    if ":" in native_dep:
        native_lib_name = native_dep.split(":")[1].split('.so')[0]
    else:
        native_lib_name = native_dep.split('.so')[0]
    output =  "lib{}.jnilib".format(native_lib_name)

    return native.genrule(
        name = name,
        outs = [output],
        srcs = [native_dep],
        cmd = """
        so_file="lib{}.so"
        if [ ! -f $$so_file ]; then
            dir=$$(dirname $@)
            cp $< $$dir/lib{}.so
            chmod 755 $$dir/lib{}.so
        fi

        cp $< $@
        chmod 755 $@
        """.replace('{}', native_lib_name)
    )