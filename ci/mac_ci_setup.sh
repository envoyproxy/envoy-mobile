#!/bin/bash

set -e

# Uninstall bazel so bazelisk has to be used
brew uninstall --force bazel

# Leverage Envoy upstream's setup scripts to avoid repeating here.
./envoy/ci/mac_ci_setup.sh

# https://github.com/Microsoft/azure-pipelines-image-generation/blob/master/images/macos/macos-10.15-Readme.md#xcode
sudo xcode-select --switch /Applications/Xcode_11.3.1.app
