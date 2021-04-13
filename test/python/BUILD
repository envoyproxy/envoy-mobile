load("@pip//:requirements.bzl", "requirement")
load("@rules_python//python:defs.bzl", "py_test")

licenses(["notice"])  # Apache 2

py_test(
    name = "runtest_unit",
    srcs = glob([
        "conftest.py",
        "echo_server.py",
        "runner.py",
        "unit/**/*.py",
    ]),
    args = [
        "test/python/",
        "-s",
    ],
    data = [
        "test_envoy_config_template.yaml",
    ],
    main = "runner.py",
    python_version = "PY3",
    deps = [
        "//library/python:envoy_requests",
        requirement("pytest"),
        requirement("pytest-asyncio"),
        requirement("requests"),
    ],
)

py_test(
    name = "runtest_integration",
    srcs = glob([
        "conftest.py",
        "echo_server.py",
        "runner.py",
        "integration/**/*.py",
    ]),
    args = ["test/python/"],
    data = [
        "test_envoy_config_template.yaml",
    ],
    main = "runner.py",
    python_version = "PY3",
    deps = [
        "//library/python:envoy_requests",
        requirement("pytest"),
    ],
)
