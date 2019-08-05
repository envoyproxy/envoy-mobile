licenses(["notice"])  # Apache 2

exports_files([
    "EnvoyEngine.h",
    "EnvoyTypes.h",
])

objc_library(
    name = "envoy_engine_objc_lib",
    srcs = [
        "EnvoyEngineImpl.h",
        "EnvoyEngineImpl.m",
        "EnvoyHttpStream.m",
    ],
    hdrs = [
        "EnvoyEngine.h",
        "EnvoyHttpStream.h",
        "EnvoyTypes.h",
    ],
    visibility = ["//visibility:public"],
    deps = ["//library/common:envoy_main_interface_lib"],
)
