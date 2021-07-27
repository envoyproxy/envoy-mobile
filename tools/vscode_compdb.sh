#!/usr/bin/env bash

# Setting TEST_TMPDIR here so the compdb headers won't be overwritten by another bazel run
CC=clang TEST_TMPDIR=${BUILD_DIR:-/tmp}/envoy-mobile-compdb envoy/tools/gen_compilation_database.py --vscode //library/cc/... //library/common/... //test/cc/... //test/common/...

# Kill clangd to reload the compilation database
pkill clangd || :
