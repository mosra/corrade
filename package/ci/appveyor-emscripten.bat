call "C:/Program Files (x86)/Microsoft Visual Studio 14.0/VC/vcvarsall.bat" || exit /b
rem Workaround for CMake not wanting sh.exe on PATH for MinGW. AARGH.
set PATH=%PATH:C:\Program Files\Git\usr\bin;=%
set PATH=C:\emscripten;%APPVEYOR_BUILD_FOLDER%\deps-native\bin;%PATH%
call "C:\emscripten\emsdk_env.bat" || exit /b
git submodule update --init || exit /b

rem Build native corrade-rc
mkdir build && cd build || exit /b
cmake .. ^
    -DCMAKE_BUILD_TYPE=Release ^
    -DCMAKE_INSTALL_PREFIX=%APPVEYOR_BUILD_FOLDER%/deps-native ^
    -DWITH_INTERCONNECT=OFF ^
    -DWITH_PLUGINMANAGER=OFF ^
    -DWITH_TESTSUITE=OFF ^
    -G Ninja || exit /b
cmake --build . || exit /b
cmake --build . --target install || exit /b
cd .. || exit /b

rem Crosscompile
mkdir build-emscripten && cd build-emscripten || exit /b
cmake .. ^
    -DCORRADE_RC_EXECUTABLE=%APPVEYOR_BUILD_FOLDER%/deps-native/bin/corrade-rc.exe ^
    -DCMAKE_TOOLCHAIN_FILE="../toolchains/generic/Emscripten.cmake" ^
    -DEMSCRIPTEN_PREFIX="C:/emscripten/emscripten/1.35.0" ^
    -DCMAKE_BUILD_TYPE=Release ^
    -DCMAKE_CXX_FLAGS_RELEASE="-DNDEBUG -O1" ^
    -DCMAKE_EXE_LINKER_FLAGS_RELEASE="-O1" ^
    -DBUILD_TESTS=ON ^
    -G "MinGW Makefiles" || exit /b
cmake --build . -- -j2 || exit /b

rem Test
ctest -V || exit /b
