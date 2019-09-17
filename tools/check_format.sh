#!/usr/bin/env bash

set -e

ENVOY_FORMAT_ACTION="$1"
if [ -z "$ENVOY_FORMAT_ACTION" ]; then
  echo "No action specified, defaulting to check"
  ENVOY_FORMAT_ACTION="check"
fi

# TODO(mattklein123): WORKSPACE is excluded due to warning about @bazel_tools reference. Fix here
#                     or in the upstream checker.
envoy/tools/check_format.py \
    --add-excluded-prefixes ./envoy/ ./envoy_build_config/extensions_build_config.bzl ./WORKSPACE ./dist/Envoy.framework/ ./library/common/config_template.cc \
    --skip_envoy_build_rule_check "$ENVOY_FORMAT_ACTION" \
    --namespace_check_excluded_paths ./examples/ ./library/java/ ./library/kotlin ./library/objective-c \
    --build_fixer_check_excluded_paths ./BUILD ./dist ./examples ./library/java ./library/kotlin ./library/objective-c ./library/swift
