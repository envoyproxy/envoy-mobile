#!/usr/bin/env bash

set -e

envoy/tools/check_format.py --add-excluded-prefixes=./envoy/ check
envoy/tools/format_python_tools.sh check
