
#!/usr/bin/env bash

set -e

export PATH=/usr/lib/llvm-9/bin:$PATH
export CC=clang
export CXX=clang++

export REPOSITORY="@envoy"
export EXTRA_QUERY_PATHS="//test/..."
export BUILD_PATH="test/coverage/BUILD"
export ONLY_EXTRA_QUERY_PATHS="true"

envoy/test/run_envoy_bazel_coverage.sh
