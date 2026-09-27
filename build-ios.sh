#!/bin/sh
# Configure an Xcode project for helloworld on iOS.
# usage: ./build-ios.sh sim|device
#   sim    -> build-ios-sim (arm64 simulator, QT_IOS_SIM)
#   device -> build-ios     (arm64 device, QT_IOS_DEVICE, needs IOS_DEVELOPMENT_TEAM)

QT_IOS_SIM="${QT_IOS_SIM:-$HOME/Development/3rdparty.ios-sim/Qt/6.9.1/ios}"
QT_IOS_DEVICE="${QT_IOS_DEVICE:-$HOME/Development/qt6-gles-ios/3rdparty.ios/Qt/6.9.0/ios}"

# Qt's default LaunchScreen.storyboard needs Xcode's iOS platform component
# (Xcode > Settings > Components) just to compile; the app runs full screen without it.
LAUNCH_SCREEN="-DQT_NO_SET_DEFAULT_IOS_LAUNCH_SCREEN=${QT_NO_SET_DEFAULT_IOS_LAUNCH_SCREEN:-ON}"

case "$1" in
  sim)
    "$QT_IOS_SIM/bin/qt-cmake" -G Xcode \
      -DQT_HOST_PATH="$QT_IOS_SIM/../macos" \
      -DCMAKE_OSX_SYSROOT=iphonesimulator \
      -DCMAKE_OSX_ARCHITECTURES=arm64 \
      $LAUNCH_SCREEN \
      -S . -B build-ios-sim
    ;;
  device)
    if [ -z "$IOS_DEVELOPMENT_TEAM" ]; then
      echo "IOS_DEVELOPMENT_TEAM must be set for device builds" >&2
      exit 1
    fi
    "$QT_IOS_DEVICE/bin/qt-cmake" -G Xcode \
      -DQT_HOST_PATH="$QT_IOS_DEVICE/../macos" \
      -DCMAKE_OSX_SYSROOT=iphoneos \
      -DCMAKE_OSX_ARCHITECTURES=arm64 \
      -DCMAKE_XCODE_ATTRIBUTE_DEVELOPMENT_TEAM="$IOS_DEVELOPMENT_TEAM" \
      $LAUNCH_SCREEN \
      -S . -B build-ios
    ;;
  *)
    echo "usage: $0 sim|device" >&2
    exit 1
    ;;
esac
