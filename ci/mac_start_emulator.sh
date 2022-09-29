#!/bin/bash

set -e

if [[ "$(/usr/bin/uname -m)" == "arm64" ]]
then
  arch="arm64-v8a"
else
  arch="x86_64"
fi

echo "y" | $ANDROID_HOME/cmdline-tools/7.0/bin/sdkmanager --install "system-images;android-29;google_apis;$arch"
echo "no" | $ANDROID_HOME/cmdline-tools/7.0/bin/avdmanager create avd -n test_android_emulator -k "system-images;android-29;google_apis;$arch" --force
ls $ANDROID_HOME/tools/bin/
nohup $ANDROID_HOME/emulator/emulator -partition-size 1024 -avd test_android_emulator -no-snapshot > /dev/null 2>&1 & $ANDROID_HOME/platform-tools/adb wait-for-device shell 'while [[ -z $(getprop sys.boot_completed | tr -d '\r') ]]; do sleep 1; done; input keyevent 82'
