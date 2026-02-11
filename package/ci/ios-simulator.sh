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
    `# Make libc++ remove transitive includes, both for faster build times` \
    `# and to detect if we're missing a transitive include. Works with` \
    `# libc++ 16+, which is used by Xcode 15 (i.e., will get used on the` \
    `# next CircleCI executor update).` \
    -DCMAKE_CXX_FLAGS="-D_LIBCPP_REMOVE_TRANSITIVE_INCLUDES" \
    -DCORRADE_BUILD_STATIC=ON \
    -DCORRADE_BUILD_TESTS=ON \
    -DCORRADE_TESTSUITE_TARGET_XCTEST=ON \
    -G Xcode
set -o pipefail && cmake --build . --config Release -j$XCODE_JOBS | xcbeautify

# TODO enable again once https://github.com/mosra/corrade/pull/176 is resolved
#CORRADE_TEST_COLOR=ON ctest -V -C Release

# Test install, after running the tests as for them it shouldn't be needed
set -o pipefail && cmake --build . --config Release --target install -j$XCODE_JOBS | xcbeautify
