#!/bin/bash
set -ev

git submodule update --init

# Crosscompile
mkdir build-emscripten && cd build-emscripten
cmake .. \
    -DCMAKE_TOOLCHAIN_FILE="../toolchains/generic/Emscripten-wasm.cmake" \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_CXX_FLAGS_RELEASE="-DNDEBUG -O1" \
    -DCMAKE_INSTALL_PREFIX=$HOME/deps \
    -DCMAKE_EXE_LINKER_FLAGS_RELEASE="-O1" \
    -DCORRADE_BUILD_TESTS=ON \
    $EXTRA_OPTS \
    -G Ninja
ninja

# Test
CORRADE_TEST_COLOR=ON ctest -V

# Test install, after running the tests as for them it shouldn't be needed
ninja install

cd ..

# Examples. This is, among other things, verifying sanity of FindCorrade.cmake
# especially when it comes to finding the right corrade-rc.
mkdir build-examples && cd build-examples
cmake ../src/examples \
    -DCMAKE_TOOLCHAIN_FILE="../toolchains/generic/Emscripten-wasm.cmake" \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_CXX_FLAGS_RELEASE="-DNDEBUG -O1" \
    -DCMAKE_FIND_ROOT_PATH=$HOME/deps \
    -DCMAKE_EXE_LINKER_FLAGS_RELEASE="-O1" \
    $EXTRA_OPTS \
    -G Ninja
ninja
