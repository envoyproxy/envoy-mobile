#!/bin/bash

set -e

# Setup basic requirements and install them.
sudo apt-get update
export DEBIAN_FRONTEND=noninteractive
sudo apt-get install -y wget software-properties-common make cmake git \
  unzip bc libtool ninja-build automake zip time golang \
  apt-transport-https

# clang 8.
wget -O - https://apt.llvm.org/llvm-snapshot.gpg.key | sudo apt-key add -
sudo apt-add-repository "deb https://apt.llvm.org/xenial/ llvm-toolchain-xenial-8 main"
sudo apt-get update
sudo apt-get install -y clang-8 clang-format-8 clang-tidy-8 lld-8 libc++-8-dev libc++abi-8-dev
ls -lh /usr/bin/clang*

# Bazel and related dependencies.
sudo apt-get install -y openjdk-8-jdk curl
echo "deb [arch=amd64] https://storage.googleapis.com/bazel-apt stable jdk1.8" | sudo tee /etc/apt/sources.list.d/bazel.list
curl https://bazel.build/bazel-release.pub.gpg | sudo apt-key add -
sudo apt-get update
sudo apt-get install -y bazel
sudo apt-get install -y aspell
sudo rm -rf /var/lib/apt/lists/*
