#!/bin/bash
set -ev

# Build native corrade-rc
mkdir build && cd build
cmake .. \
    -DCMAKE_INSTALL_PREFIX=$HOME/deps-native \
    -DCMAKE_BUILD_TYPE=Release \
    -DCORRADE_WITH_INTERCONNECT=OFF \
    -DCORRADE_WITH_PLUGINMANAGER=OFF \
    -DCORRADE_WITH_TESTSUITE=OFF \
    -DCORRADE_WITH_UTILITY=OFF \
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
    -DCORRADE_BUILD_TESTS=ON \
    -G Ninja
ninja

# Wait for emulator to start (done in parallel to build) and run tests
circle-android wait-for-boot
# `adb push` uploads timeout quite often, and then CircleCI waits 10 minutes
# until it aborts the job due to no output. CTest can do timeouts on its own,
# but somehow the default is 10M seconds, which is quite a lot honestly.
# Instead set the timeout to 15 seconds which should be enough even for very
# heavy future benchmarks, and try two more times if it gets stuck.
CORRADE_TEST_COLOR=ON ctest -V --timeout 15 --repeat after-timeout:3

# Test install, after running the tests as for them it shouldn't be needed
ninja install
