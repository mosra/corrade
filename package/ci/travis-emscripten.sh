#!/bin/bash
set -ev

# Get ninja-build
wget https://github.com/ninja-build/ninja/releases/download/v1.8.2/ninja-linux.zip
unzip ninja-linux.zip
mv ninja /usr/local/bin/
ninja --version

git submodule update --init

# Build native corrade-rc
mkdir build && cd build || exit /b
cmake .. \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_MAKE_PROGRAM=ninja \
    -DCMAKE_INSTALL_PREFIX=$HOME/deps-native \
    -DWITH_INTERCONNECT=OFF \
    -DWITH_PLUGINMANAGER=OFF \
    -DWITH_TESTSUITE=OFF \
    -DWITH_UTILITY=OFF \
    -G Ninja
ninja install
cd ..

# Crosscompile
mkdir build-emscripten && cd build-emscripten
cmake .. \
    -DCMAKE_MAKE_PROGRAM=ninja \
    -DCORRADE_RC_EXECUTABLE=$HOME/deps-native/bin/corrade-rc \
    -DCMAKE_TOOLCHAIN_FILE="../toolchains/generic/Emscripten-wasm.cmake" \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_CXX_FLAGS_RELEASE="-DNDEBUG -O1" \
    -DCMAKE_INSTALL_PREFIX=$HOME/deps \
    -DCMAKE_EXE_LINKER_FLAGS_RELEASE="-O1" \
    -DBUILD_TESTS=ON \
    -G Ninja
ninja -j2

# Test
CORRADE_TEST_COLOR=ON ctest -V

# Test install, after running the tests as for them it shouldn't be needed
ninja install
