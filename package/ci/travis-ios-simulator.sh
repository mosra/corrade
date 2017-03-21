#!/bin/bash
set -ev

git submodule update --init

# Build native corrade-rc
mkdir build && cd build
cmake .. \
    -DCMAKE_INSTALL_PREFIX=$HOME/deps-native \
    -DCMAKE_INSTALL_RPATH=$HOME/deps-native/lib \
    -DWITH_INTERCONNECT=OFF \
    -DWITH_PLUGINMANAGER=OFF \
    -DWITH_TESTSUITE=OFF \
    -DCMAKE_BUILD_TYPE=Release
make -j install
cd ..

# Crosscompile
mkdir build-ios && cd build-ios
cmake .. \
    -DCMAKE_TOOLCHAIN_FILE=../toolchains/generic/iOS.cmake \
    -DCMAKE_OSX_SYSROOT=/Applications/Xcode.app/Contents/Developer/Platforms/iPhoneSimulator.platform/Developer/SDKs/iPhoneSimulator.sdk \
    -DCMAKE_OSX_ARCHITECTURES="x86_64" \
    -DCORRADE_RC_EXECUTABLE=$HOME/deps-native/bin/corrade-rc \
    -DBUILD_STATIC=ON \
    -DBUILD_TESTS=ON \
    -DTESTSUITE_TARGET_XCTEST=ON \
    -G Xcode
set -o pipefail && cmake --build . --config Release | xcpretty
CORRADE_TEST_COLOR=ON ctest -V -C Release
