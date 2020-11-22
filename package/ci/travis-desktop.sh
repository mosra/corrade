#!/bin/bash
set -ev

mkdir build && cd build
# Not using CXXFLAGS in order to avoid affecting dependencies
cmake .. \
    -DCMAKE_CXX_FLAGS="$CMAKE_CXX_FLAGS" \
    -DCMAKE_INSTALL_PREFIX=$HOME/deps \
    -DCMAKE_INSTALL_RPATH=$HOME/deps/lib \
    -DBUILD_TESTS=ON \
    -DBUILD_DEPRECATED=$BUILD_DEPRECATED \
    -DBUILD_STATIC=$BUILD_STATIC \
    -DCMAKE_BUILD_TYPE=$CONFIGURATION \
    -G Ninja
ninja
ASAN_OPTIONS="color=always" LSAN_OPTIONS="color=always" TSAN_OPTIONS="color=always suppressions=$(pwd)/../package/ci/threadsanitizer.conf" CORRADE_TEST_COLOR=ON ctest -V

# Test install, after running the tests as for them it shouldn't be needed
ninja install

cd ..

# Examples
mkdir build-examples && cd build-examples
cmake ../src/examples \
    -DCMAKE_CXX_FLAGS="$CMAKE_CXX_FLAGS" \
    -DCMAKE_BUILD_TYPE=$CONFIGURATION \
    -DCMAKE_PREFIX_PATH=$HOME/deps \
    -G Ninja
ninja
