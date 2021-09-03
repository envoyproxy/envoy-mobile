#!/bin/bash

set -e

bazel build $@ \
      //library/kotlin/io/envoyproxy/envoymobile:envoy_lib_lint \
      //examples/kotlin/hello_world:hello_envoy_kt_lint
bazel build kotlin_format
