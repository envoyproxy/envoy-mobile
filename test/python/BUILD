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
    main = "runner.py",
    args = ["test/python/", "-s"],
    deps = [
        "//library/python:envoy_requests",
        requirement("pytest"),
        requirement("pytest-asyncio"),
        requirement("requests"),
    ],
    data = [
        "test_envoy_config_template.yaml",
    ],
    python_version = "PY3",
)

py_test(
    name = "runtest_integration",
    srcs = glob([
        "conftest.py",
        "echo_server.py",
        "runner.py",
        "integration/**/*.py",
    ]),
    main = "runner.py",
    args = ["test/python/"],
    deps = [
        "//library/python:envoy_requests",
        requirement("pytest"),
    ],
    data = [
        "test_envoy_config_template.yaml",
    ],
    python_version = "PY3",
)
