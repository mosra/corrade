#!/bin/bash
set -ev

# Build native corrade-rc
mkdir build && cd build
cmake .. \
    -DCMAKE_INSTALL_PREFIX=$HOME/deps-native \
    -DCMAKE_BUILD_TYPE=Release \
    -DWITH_INTERCONNECT=OFF \
    -DWITH_PLUGINMANAGER=OFF \
    -DWITH_TESTSUITE=OFF \
    -DWITH_UTILITY=OFF \
    -G Ninja
ninja install
cd ..

# Crosscompile
mkdir build-android-x86 && cd build-android-x86
cmake .. \
    -DCMAKE_SYSTEM_NAME=Android \
    -DCMAKE_SYSTEM_VERSION=29 \
    -DCMAKE_ANDROID_ARCH_ABI=x86 \
    -DCMAKE_ANDROID_NDK_TOOLCHAIN_VERSION=clang \
    -DCMAKE_ANDROID_STL_TYPE=c++_static \
    -DCMAKE_INSTALL_PREFIX=$HOME/deps \
    -DCMAKE_BUILD_TYPE=Release \
    -DCORRADE_RC_EXECUTABLE=$HOME/deps-native/bin/corrade-rc \
    -DBUILD_TESTS=ON \
    -G Ninja
ninja

# Wait for emulator to start (done in parallel to build) and run tests
circle-android wait-for-boot
CORRADE_TEST_COLOR=ON ctest -V

# Test install, after running the tests as for them it shouldn't be needed
ninja install
