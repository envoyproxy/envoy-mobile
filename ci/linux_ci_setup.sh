#!/bin/bash

set -e

# Copied from mac_ci_setup.sh

export HOMEBREW_NO_AUTO_UPDATE=1

function is_installed {
    brew ls --versions "$1" >/dev/null
}

function install {
    echo "Installing $1"
    if ! brew install "$1"; then
        echo "Failed to install $1"
        exit 1
    fi
}

# Disabled due to frequent CI failures for now.
#if ! brew update; then
#    echo "Failed to update homebrew"
#    exit 1
#fi

# NONINTERACTIVE=1 /bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"
# echo 'eval "$(/home/linuxbrew/.linuxbrew/bin/brew shellenv)"' >> /github/home/.profile
# eval "$(/home/linuxbrew/.linuxbrew/bin/brew shellenv)"

# DEPS="automake cmake coreutils libtool wget ninja"
# for DEP in ${DEPS}
# do
#     is_installed "${DEP}" || install "${DEP}"
# done

if [ -n "$CIRCLECI" ]; then
    # bazel uses jgit internally and the default circle-ci .gitconfig says to
    # convert https://github.com to ssh://git@github.com, which jgit does not support.
    mv ~/.gitconfig ~/.gitconfig_save
fi

./bazelw version

pushd "/github/home/.android"
if [ ! -d ./sdk/cmdline-tools/latest ]; then
	mkdir -p sdk/
	cmdline_file="commandlinetools-linux-7583922_latest.zip"
	curl -OL "https://dl.google.com/android/repository/$cmdline_file"
	unzip "$cmdline_file"
	mkdir -p sdk/cmdline-tools/latest
	mv cmdline-tools/* sdk/cmdline-tools/latest
fi

export ANDROID_HOME="$(realpath "$sdk_install_target/sdk")"
SDKMANAGER=$ANDROID_HOME/cmdline-tools/latest/bin/sdkmanager

$SDKMANAGER --uninstall "ndk-bundle"
echo "y" | $SDKMANAGER "ndk;21.4.7075529"
$SDKMANAGER --install "platforms;android-30"
ln -sfn $ANDROID_SDK_ROOT/ndk/21.4.7075529 "${ANDROID_SDK_ROOT}/ndk-bundle"

# Download and set up build-tools 30.0.3, 31.0.0 is missing dx.jar.
$SDKMANAGER --install "build-tools;30.0.2"
echo "ANDROID_NDK_HOME=$ANDROID_HOME/ndk/21.4.7075529" >> $GITHUB_ENV
