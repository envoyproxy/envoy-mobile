load("@rules_python//python:packaging.bzl", "py_wheel")


version_bzl_template = """\
def python_abi():
    return "{python_abi}"
"""


# python binary detection shamelessly stolen from pybind11
# https://github.com/pybind/pybind11_bazel/blob/26973c0ff320cb4b39e45bc3e4297b82bc3a6c09/python_configure.bzl#L155-L171
# so that we have the same python version + detect the same ABI
#
# TODO(crockeo): roll our own / unify python version detection
_PYTHON_BIN_PATH = "PYTHON_BIN_PATH"


def _fail(msg):
    """Output failure message when auto configuration fails."""
    red = "\033[0;31m"
    no_color = "\033[0m"
    fail("%sPython Configuration Error:%s %s\n" % (red, no_color, msg))


def _get_python_bin(repository_ctx):
    """Gets the python bin path."""
    python_bin = repository_ctx.os.environ.get(_PYTHON_BIN_PATH)
    if python_bin != None:
        return python_bin

    python_short_name = "python" + repository_ctx.attr.python_version
    python_bin_path = repository_ctx.which(python_short_name)

    if python_bin_path != None:
        return str(python_bin_path)
    _fail("Cannot find python in PATH, please make sure " +
          "python is installed and add its directory in PATH, or --define " +
          "%s='/something/else'.\nPATH=%s" % (
              _PYTHON_BIN_PATH,
              repository_ctx.os.environ.get("PATH", ""),
          ))


def _get_python_abi(rctx, python_bin):
    result = rctx.execute([
        python_bin,
        "-c",
        "import platform;" +
        "assert platform.python_implementation() == 'CPython';" +
        "version = platform.python_version_tuple();" +
        "print(f'cp{version[0]}{version[1]}')",
    ])
    return result.stdout.splitlines()[0]

def _declare_python_version_impl(rctx):
    python_bin = _get_python_bin(rctx)
    python_abi = _get_python_abi(rctx, python_bin)
    rctx.file("BUILD")
    rctx.file("version.bzl", version_bzl_template.format(python_abi=python_abi))

declare_python_version = repository_rule(
    implementation = _declare_python_version_impl,
    attrs = {
        "python_version": attr.string(mandatory=True),
    },
    local = True,
)
