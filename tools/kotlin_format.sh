#!/bin/bash

set -e

bazel build $REMOTE_BUILD_OPTIONS \
      //library/kotlin/io/envoyproxy/envoymobile:envoy_lib_lint \
      //examples/kotlin/hello_world:hello_envoy_kt_lint
bazel build kotlin_format
