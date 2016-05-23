call "C:/Program Files (x86)/Microsoft Visual Studio 14.0/VC/vcvarsall.bat"
@echo ON

@rem Build native corrade-rc
mkdir build && cd build || exit /b
cmake .. ^
    -DCMAKE_BUILD_TYPE=Release ^
    -DCMAKE_INSTALL_PREFIX=%APPVEYOR_BUILD_FOLDER%/deps ^
    -G Ninja || exit /b
cmake --build . || exit /b
cmake --build . --target install || exit /b
cd .. || exit /b

@rem Crosscompile
mkdir build-rt && cd build-rt || exit /b
cmake .. ^
    -DCORRADE_RC_EXECUTABLE=%APPVEYOR_BUILD_FOLDER%/deps/bin/corrade-rc.exe ^
    -DBUILD_STATIC=ON ^
    -DCMAKE_SYSTEM_NAME=WindowsStore ^
    -DCMAKE_SYSTEM_VERSION=10.0 ^
    -G "Visual Studio 14 2015" || exit /b
cmake --build . --config Release || exit /b
