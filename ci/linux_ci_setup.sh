#!/bin/bash

set -e

# Set up basic requirements and install them.
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

# Bazel.
sudo apt-get install -y curl
echo "deb [arch=amd64] https://storage.googleapis.com/bazel-apt stable jdk1.8" | sudo tee /etc/apt/sources.list.d/bazel.list
curl https://bazel.build/bazel-release.pub.gpg | sudo apt-key add -
sudo apt-get update
sudo apt-get install -y bazel
sudo rm -rf /var/lib/apt/lists/*

# ndk 20
wget -c https://dl.google.com/android/repository/android-ndk-r20-linux-x86_64.zip
unzip android-ndk-r20-linux-x86_64.zip
sudo rm -rf /usr/local/lib/android/sdk/ndk-bundle
sudo mv android-ndk-r20-linux-x86_64 /usr/local/lib/android/sdk/ndk-bundle
