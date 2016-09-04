#!/bin/bash
set -ev

mkdir build && cd build
cmake .. \
    -DCMAKE_INSTALL_PREFIX=$HOME/deps \
    -DCMAKE_INSTALL_RPATH=$HOME/deps/lib \
    -DBUILD_TESTS=ON \
    -DCMAKE_BUILD_TYPE=Debug
make -j install
CORRADE_TEST_COLOR=ON ctest -V
cd ..

# Examples
mkdir build-examples && cd build-examples
cmake -DCMAKE_BUILD_TYPE=Debug -DCMAKE_PREFIX_PATH=$HOME/deps ../src/examples
make -j
