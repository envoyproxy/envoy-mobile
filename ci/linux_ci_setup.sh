#!/bin/bash

set -e

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
  unzip bc libtool ninja-build automake zip time lcov \
  apt-transport-https

# clang 10
wget -O - https://apt.llvm.org/llvm-snapshot.gpg.key | sudo apt-key add -
sudo apt-add-repository "deb https://apt.llvm.org/xenial/ llvm-toolchain-xenial-10 main"
sudo apt-get update
#sudo apt-get install -y clang-10 lld-10 libc++-10-dev libc++abi-10-dev
sudo apt-get install -y lld-10 libc++-10-dev libc++abi-10-dev

#LLVM_RELEASE="clang+llvm-${LLVM_VERSION}-${LLVM_DISTRO}"
LLVM_RELEASE="clang+llvm-10.0.0-x86_64-linux-gnu-ubuntu-18.04"
LLVM_SHA256SUM="b25f592a0c00686f03e3b7db68ca6dc87418f681f4ead4df4745a01d9be63843"
LLVM_DOWNLOAD_PREFIX=${LLVM_DOWNLOAD_PREFIX:-https://github.com/llvm/llvm-project/releases/download/llvmorg-}
#download_and_check "${LLVM_RELEASE}.tar.xz" "${LLVM_DOWNLOAD_PREFIX}${LLVM_VERSION}/${LLVM_RELEASE}.tar.xz" "${LLVM_SHA256SUM}"
download_and_check "${LLVM_RELEASE}.tar.xz" "${LLVM_DOWNLOAD_PREFIX}10.0.0/${LLVM_RELEASE}.tar.xz" "${LLVM_SHA256SUM}"
tar Jxf "${LLVM_RELEASE}.tar.xz"
mv "./${LLVM_RELEASE}" /opt/llvm
chown -R root:root /opt/llvm
rm "./${LLVM_RELEASE}.tar.xz"

sudo update-alternatives --remove-all clang
sudo update-alternatives --remove-all clang++
sudo update-alternatives --install /usr/bin/clang clang /opt/llvm/bin/clang-10 100
sudo update-alternatives --install /usr/bin/clang++ clang++ /opt/llvm/bin/clang++ 100

sudo apt-get install gnupg2
gpg --version

# buildifier
sudo wget -O /usr/local/bin/buildifier https://github.com/bazelbuild/buildtools/releases/download/2.2.1/buildifier
sudo chmod +x /usr/local/bin/buildifier

# bazelisk
sudo wget -O /usr/local/bin/bazel https://github.com/bazelbuild/bazelisk/releases/download/v0.0.8/bazelisk-linux-amd64
sudo chmod +x /usr/local/bin/bazel
