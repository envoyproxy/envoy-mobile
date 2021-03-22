load("@rules_python//python:defs.bzl", "py_library")
load("@py_envoy_engine_deps//:requirements.bzl", "requirement")

licenses(["notice"])  # Apache 2

py_library(
    name = "gevent_util",
    srcs = [
        "__init__.py",
    ],
    visibility = ["//visibility:public"],
    deps = [
        requirement("gevent"),
    ],
)
