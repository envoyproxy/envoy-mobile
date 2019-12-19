#!/bin/bash

set -euo pipefail

bazel build --config=ios ios_dist

./tools/index-import.sh
