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
    -DCMAKE_BUILD_TYPE=Debug
make -j install
ASAN_OPTIONS="color=always" LSAN_OPTIONS="color=always" CORRADE_TEST_COLOR=ON ctest -V
cd ..

# Examples
mkdir build-examples && cd build-examples
cmake -DCMAKE_CXX_FLAGS="$CMAKE_CXX_FLAGS" -DCMAKE_BUILD_TYPE=Debug -DCMAKE_PREFIX_PATH=$HOME/deps ../src/examples
make -j
