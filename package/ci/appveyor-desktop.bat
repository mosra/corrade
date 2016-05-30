call "C:/Program Files (x86)/Microsoft Visual Studio 14.0/VC/vcvarsall.bat" || exit /b

rem Build
mkdir build && cd build || exit /b
cmake .. ^
    -DCMAKE_BUILD_TYPE=Release ^
    -DCMAKE_INSTALL_PREFIX=%APPVEYOR_BUILD_FOLDER%/install ^
    -DUTILITY_USE_ANSI_COLORS=%ANSI_COLORS% ^
    -DBUILD_TESTS=ON ^
    -G Ninja || exit /b
cmake --build . || exit /b
cmake --build . --target install || exit /b
set PATH=%APPVEYOR_BUILD_FOLDER%/install/bin;%PATH%

rem Test
cd %APPVEYOR_BUILD_FOLDER%/build || exit /b
set CORRADE_TEST_COLOR=ON
ctest -V || exit /b

rem Examples
cd %APPVEYOR_BUILD_FOLDER% || exit /b
mkdir build-examples && cd build-examples || exit /b
cmake ../src/examples ^
    -DCMAKE_BUILD_TYPE=Release ^
    -DCMAKE_PREFIX_PATH=%APPVEYOR_BUILD_FOLDER%/install ^
    -G Ninja || exit /b
cmake --build . || exit /b
