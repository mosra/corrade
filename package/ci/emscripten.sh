#!/bin/bash
set -ev

git submodule update --init

# Crosscompile
mkdir build-emscripten && cd build-emscripten
cmake .. \
    -DCMAKE_TOOLCHAIN_FILE="../toolchains/generic/Emscripten-wasm.cmake" \
    `# Building as Debug always, as Release optimizations take a long time` \
    `# and make no sense on the CI. Thus, benchmark output will not be` \
    `# really meaningful, but we still want to run them to catch issues.` \
    -DCMAKE_BUILD_TYPE=Debug \
    -DCMAKE_INSTALL_PREFIX=$HOME/deps \
    -DCORRADE_BUILD_TESTS=ON \
    $EXTRA_OPTS \
    -G Ninja
ninja $NINJA_JOBS

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
    -DCMAKE_BUILD_TYPE=Debug \
    -DCMAKE_FIND_ROOT_PATH=$HOME/deps \
    $EXTRA_OPTS \
    -G Ninja
ninja $NINJA_JOBS
