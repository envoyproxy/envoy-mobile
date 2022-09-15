#!/bin/bash

set -e

./bazelw version

sdk_install_target="/github/home/.android"

mkdir "$sdk_install_target"
pushd "$sdk_install_target"

if [ ! -d ./sdk/cmdline-tools/latest ]; then
	mkdir -p sdk/
	cmdline_file="commandlinetools-linux-7583922_latest.zip"
	curl -OL "https://dl.google.com/android/repository/$cmdline_file"
	unzip "$cmdline_file"
	mkdir -p sdk/cmdline-tools/latest
	mv cmdline-tools/* sdk/cmdline-tools/latest
fi

export ANDROID_HOME="$(realpath "$sdk_install_target/sdk")"
export ANDROID_SDK_ROOT=ANDROID_HOME

SDKMANAGER=$ANDROID_HOME/cmdline-tools/latest/bin/sdkmanager
$SDKMANAGER --uninstall "ndk-bundle"

echo "y" | $SDKMANAGER "ndk;21.4.7075529"
$SDKMANAGER --install "platforms;android-30"
ln -sfn $ANDROID_SDK_ROOT/ndk/21.4.7075529 "${ANDROID_SDK_ROOT}/ndk-bundle"

# Download and set up build-tools 30.0.3, 31.0.0 is missing dx.jar.
$SDKMANAGER --install "build-tools;30.0.2"
echo "ANDROID_NDK_HOME=$ANDROID_HOME/ndk/21.4.7075529" >> $GITHUB_ENV
