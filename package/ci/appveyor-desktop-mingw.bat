rem Workaround for CMake not wanting sh.exe on PATH for MinGW. AARGH.
set PATH=%PATH:C:\Program Files\Git\usr\bin;=%
set PATH=C:\tools\mingw64\bin;%APPVEYOR_BUILD_FOLDER%\deps\bin;%PATH%

rem Build. Could not get Ninja to work, meh.
mkdir build && cd build || exit /b
cmake .. ^
    -DCMAKE_BUILD_TYPE=Release ^
    -DCMAKE_INSTALL_PREFIX=%APPVEYOR_BUILD_FOLDER%/deps ^
    -DBUILD_TESTS=ON ^
    -G "MinGW Makefiles" || exit /b
cmake --build . -- -j || exit /b
cmake --build . --target install -- -j || exit /b

rem Test
cd %APPVEYOR_BUILD_FOLDER%/build || exit /b
ctest -V || exit /b

rem Examples
cd %APPVEYOR_BUILD_FOLDER% || exit /b
mkdir build-examples && cd build-examples || exit /b
cmake ../src/examples ^
    -DCMAKE_BUILD_TYPE=Release ^
    -DCMAKE_PREFIX_PATH=%APPVEYOR_BUILD_FOLDER%/deps ^
    -G "MinGW Makefiles" || exit /b
cmake --build . -- -j || exit /b
