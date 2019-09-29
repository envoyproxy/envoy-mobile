def envoy_mobile_cc_jni_library(name, srcs = [], copts = [], linkopts = [], deps = []):
    # For aar generation
    native.cc_library(
        name = name,
        srcs = srcs,
        deps = deps,
        copts = copts,
        linkopts = linkopts,
    )

    # For Linux jni jar generation
    native.cc_library(
        name = name + "_linux",
        srcs = srcs + ["@local_jdk//:jni_header", "@local_jdk//:jni_md_header-linux"],
        deps = deps,
        copts = copts,
        includes = [
            "../../external/local_jdk/include",
            "../../external/local_jdk/include/linux",
        ],
    )

    # For OS X jar generation
    native.cc_library(
        name = name + "_darwin",
        srcs = srcs + [
            "@local_jdk//:jni_header",
            "@local_jdk//:jni_md_header-darwin",
        ],
        deps = deps,
        copts = copts,
        includes = [
            "../../external/local_jdk/include",
            "../../external/local_jdk/include/darwin",
        ],
    )
