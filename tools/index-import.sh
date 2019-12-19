#!/bin/bash

set -euo pipefail

readonly bazel_root="^/private/var/tmp/_bazel_.+?/.+?/execroot/[^/]+"
readonly bazel_bin="^(?:$bazel_root/)?bazel-out/.+?/bin"

readonly bazel_object="$bazel_bin/.*/_objs/.*/(.+?)[.]o$"
readonly xcode_object="$CONFIGURATION_TEMP_DIR/envoy-mobile.build/Objects-normal/$ARCHS/\$1.o"

readonly bazel_external="^(?:$bazel_root/)?external/"
readonly xcode_external="$SRCROOT/bazel-$(basename "$SRCROOT")/external/"

set -x
time ./index-import \
    -remap "$bazel_object=$xcode_object" \
    -remap "$bazel_external=$xcode_external" \
    -remap "$bazel_root=$SRCROOT" \
    ./tmp/indexstore \
    "$BUILD_DIR"/../../Index/DataStore
