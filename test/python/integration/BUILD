load("@rules_python//python:defs.bzl", "py_test")

licenses(["notice"])  # Apache 2

py_test(
    name = "test_send_headers",
    srcs = [
        "common.py",
        "test_send_headers.py",
    ],
    data = [
        "//library/python:envoy_engine.so",
    ],
    main = "test_send_headers.py",
    deps = [
        "//library/python/gevent_util",
    ],
)

py_test(
    name = "test_lifetimes",
    srcs = [
        "common.py",
        "test_lifetimes.py",
    ],
    data = [
        "//library/python:envoy_engine.so",
    ],
    main = "test_lifetimes.py",
)
