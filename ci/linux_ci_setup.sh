#!/bin/bash

set -e

function download_and_check () {
  local to=$1
  local url=$2
  local sha256=$3

  curl -fsSL --output "${to}" "${url}"
  echo "${sha256}  ${to}" | sha256sum --check
}

# Set up basic requirements and install them.
# workaround https://askubuntu.com/questions/41605/trouble-downloading-packages-list-due-to-a-hash-sum-mismatch-error
sudo rm -rf /var/lib/apt/lists/*

# We have seen problems with heroku's keys.
# We do not use heroku, but it is pre-installed in the github actions machines.
curl https://cli-assets.heroku.com/apt/release.key | sudo apt-key add -

# https://github.com/bazelbuild/bazel/issues/11470#issuecomment-633205152
curl https://bazel.build/bazel-release.pub.gpg | sudo apt-key add -

sudo apt-get clean
sudo apt-get update

export DEBIAN_FRONTEND=noninteractive
sudo apt-get install -y wget software-properties-common make cmake git \
  unzip bc libtool ninja-build automake zip time \
  apt-transport-https

# clang 8.
wget -O - https://apt.llvm.org/llvm-snapshot.gpg.key | sudo apt-key add -
sudo apt-add-repository "deb https://apt.llvm.org/xenial/ llvm-toolchain-xenial-8 main"
sudo apt-get update
sudo apt-get install -y clang-8 lld-8 libc++-8-dev libc++abi-8-dev

sudo update-alternatives --remove-all clang
sudo update-alternatives --remove-all clang++
sudo update-alternatives --install /usr/bin/clang clang /usr/bin/clang-8 100
sudo update-alternatives --install /usr/bin/clang++ clang++ /usr/bin/clang++-8 100

sudo apt-get install gnupg2
gpg --version

# buildifier
sudo wget -O /usr/local/bin/buildifier https://github.com/bazelbuild/buildtools/releases/download/2.2.1/buildifier
sudo chmod +x /usr/local/bin/buildifier

# bazelisk
sudo wget -O /usr/local/bin/bazel https://github.com/bazelbuild/bazelisk/releases/download/v0.0.8/bazelisk-linux-amd64
sudo chmod +x /usr/local/bin/bazel

# Install lcov
LCOV_VERSION=1.14
download_and_check lcov-${LCOV_VERSION}.tar.gz https://github.com/linux-test-project/lcov/releases/download/v${LCOV_VERSION}/lcov-${LCOV_VERSION}.tar.gz \
  14995699187440e0ae4da57fe3a64adc0a3c5cf14feab971f8db38fb7d8f071a
tar zxf lcov-${LCOV_VERSION}.tar.gz
make -C lcov-${LCOV_VERSION} install
rm -rf "lcov-${LCOV_VERSION}" "./lcov-${LCOV_VERSION}.tar.gz"
