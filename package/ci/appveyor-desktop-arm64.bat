if "%APPVEYOR_BUILD_WORKER_IMAGE%" == "Visual Studio 2022" call "C:/Program Files/Microsoft Visual Studio/2022/Community/VC/Auxiliary/Build/vcvarsall.bat" x64 || exit /b
rem This is what should make the *native* corrade-rc getting found
set PATH=%APPVEYOR_BUILD_FOLDER%\deps-native\bin;%PATH%

rem Build native corrade-rc
mkdir build && cd build || exit /b
cmake .. ^
    -DCMAKE_BUILD_TYPE=Release ^
    -DCMAKE_INSTALL_PREFIX=%APPVEYOR_BUILD_FOLDER%/deps-native ^
    -DCORRADE_WITH_INTERCONNECT=OFF ^
    -DCORRADE_WITH_PLUGINMANAGER=OFF ^
    -DCORRADE_WITH_TESTSUITE=OFF ^
    -DCORRADE_WITH_UTILITY=OFF ^
    -G Ninja || exit /b
cmake --build . --target install || exit /b
cd .. || exit /b

if "%APPVEYOR_BUILD_WORKER_IMAGE%" == "Visual Studio 2022" call "C:/Program Files/Microsoft Visual Studio/2022/Community/VC/Auxiliary/Build/vcvarsall.bat" x64_arm64 || exit /b

rem Crosscompile. Explicitly set CMAKE_SYSTEM_NAME to have CMAKE_CROSSCOMPILING
rem enabled, which then triggers finding of the cross-compiled corrade-rc. It's
rem enabled even when CMAKE_SYSTEM_NAME is the same as CMAKE_HOST_SYSTEM_NAME:
rem  https://cmake.org/cmake/help/latest/variable/CMAKE_CROSSCOMPILING.html
rem Ideally CMake would detect that a cross-compilation is happening on its own
rem but it doesn't :(
mkdir build-arm64 && cd build-arm64 || exit /b
cmake .. ^
    -DCMAKE_BUILD_TYPE=%CONFIGURATION% ^
    -DCMAKE_INSTALL_PREFIX=%APPVEYOR_BUILD_FOLDER%/deps ^
    -DCMAKE_SYSTEM_NAME=Windows ^
    -DCORRADE_UTILITY_USE_ANSI_COLORS=%ANSI_COLORS% ^
    -DCORRADE_BUILD_TESTS=ON ^
    -DCORRADE_BUILD_STATIC=%BUILD_STATIC% ^
    %COMPILER_EXTRA% -G Ninja || exit /b
cmake --build . || exit /b

rem Test install
cmake --build . --target install || exit /b

rem Examples. Explicitly pass path to the native corrade-rc as it otherwise
rem picks the ARM build. OTOH I want to have its build tested for ARM, so I
rem can't just disable it above.
cd %APPVEYOR_BUILD_FOLDER% || exit /b
mkdir build-examples && cd build-examples || exit /b
cmake ../src/examples ^
    -DCMAKE_BUILD_TYPE=%CONFIGURATION% ^
    -DCMAKE_PREFIX_PATH=%APPVEYOR_BUILD_FOLDER%/deps ^
    -DCMAKE_SYSTEM_NAME=Windows ^
    -DCORRADE_RC_EXECUTABLE=%APPVEYOR_BUILD_FOLDER%/deps-native/bin/corrade-rc.exe ^
    %COMPILER_EXTRA% -G Ninja || exit /b
cmake --build . || exit /b
