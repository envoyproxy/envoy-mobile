licenses(["notice"])  # Apache 2

exports_files([
    "EnvoyEngine.h",
    "EnvoyEngineImpl.h",
    "EnvoyHttpStream.h",
    "EnvoyTypes.h",
])

objc_library(
    name = "envoy_engine_objc_lib",
    srcs = [
        "EnvoyEngine.h",
        "EnvoyEngineImpl.h",
        "EnvoyEngineImpl.m",
        "EnvoyHttpStream.h",
        "EnvoyHttpStream.m",
        "EnvoyTypes.h",
    ],
    hdrs = [
        #        "EnvoyEngine.h",
        #        "EnvoyEngineImpl.h",
        #        "EnvoyHttpStream.h",
        #        "EnvoyTypes.h",
    ],
    visibility = ["//visibility:public"],
    deps = ["//library/common:envoy_main_interface_lib"],
)
