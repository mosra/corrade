#!/bin/bash
set -ev

git submodule update --init

# Build native corrade-rc
mkdir build && cd build
cmake .. \
    -DCMAKE_INSTALL_PREFIX=$HOME/deps-native \
    -DCORRADE_WITH_INTERCONNECT=OFF \
    -DCORRADE_WITH_PLUGINMANAGER=OFF \
    -DCORRADE_WITH_TESTSUITE=OFF \
    -DCORRADE_WITH_UTILITY=OFF \
    -DCMAKE_BUILD_TYPE=Release \
    -G Ninja
ninja install
cd ..

# Crosscompile
mkdir build-ios && cd build-ios
cmake .. \
    -DCMAKE_TOOLCHAIN_FILE=../toolchains/generic/iOS.cmake \
    -DCMAKE_OSX_SYSROOT=/Applications/Xcode.app/Contents/Developer/Platforms/iPhoneSimulator.platform/Developer/SDKs/iPhoneSimulator.sdk \
    -DCMAKE_OSX_ARCHITECTURES="x86_64" \
    -DCORRADE_RC_EXECUTABLE=$HOME/deps-native/bin/corrade-rc \
    -DCMAKE_INSTALL_PREFIX=$HOME/deps \
    -DCORRADE_BUILD_STATIC=ON \
    -DCORRADE_BUILD_TESTS=ON \
    -DCORRADE_TESTSUITE_TARGET_XCTEST=ON \
    -G Xcode
set -o pipefail && cmake --build . --config Release -j$XCODE_JOBS | xcbeautify
CORRADE_TEST_COLOR=ON ctest -V -C Release

# Test install, after running the tests as for them it shouldn't be needed
set -o pipefail && cmake --build . --config Release --target install -j$XCODE_JOBS | xcbeautify
