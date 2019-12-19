#!/bin/bash

set -euo pipefail

# Xcode expects the compiler to create .d files.
for arg in "$@"; do
    if [[ $arg == *.d ]]; then
        touch "$arg"
    fi
done
