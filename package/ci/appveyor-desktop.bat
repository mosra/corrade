if "%APPVEYOR_BUILD_WORKER_IMAGE%" == "Visual Studio 2022" call "C:/Program Files/Microsoft Visual Studio/2022/Community/VC/Auxiliary/Build/vcvarsall.bat" %PLATFORM% || exit /b
if "%APPVEYOR_BUILD_WORKER_IMAGE%" == "Visual Studio 2019" call "C:/Program Files (x86)/Microsoft Visual Studio/2019/Community/VC/Auxiliary/Build/vcvarsall.bat" %PLATFORM% || exit /b
if "%APPVEYOR_BUILD_WORKER_IMAGE%" == "Visual Studio 2017" call "C:/Program Files (x86)/Microsoft Visual Studio/2017/Community/VC/Auxiliary/Build/vcvarsall.bat" %PLATFORM% || exit /b
if "%APPVEYOR_BUILD_WORKER_IMAGE%" == "Visual Studio 2015" call "C:/Program Files (x86)/Microsoft Visual Studio 14.0/VC/vcvarsall.bat" %PLATFORM% || exit /b
set PATH=%APPVEYOR_BUILD_FOLDER%\deps\bin;%PATH%

rem - Need to explicitly specify a 64-bit target, otherwise CMake+Ninja can't
rem   figure that out -- https://gitlab.kitware.com/cmake/cmake/issues/16259
rem - For TestSuite we need to enable exceptions explicitly with /EH as these
rem   are currently disabled -- https://github.com/catchorg/Catch2/issues/1113
rem - The MSVC 2022 build has coverage enabled, for which the profile library
rem   has to be explicitly linked -- https://bugs.llvm.org/show_bug.cgi?id=40877,
rem   https://gitlab.kitware.com/cmake/cmake/-/issues/23437
if "%COMPILER%" == "msvc-clang" if "%APPVEYOR_BUILD_WORKER_IMAGE%" == "Visual Studio 2022" set COMPILER_EXTRA=-DCMAKE_C_COMPILER="C:/Program Files/Microsoft Visual Studio/2022/Community/VC/Tools/Llvm/bin/clang-cl.exe" -DCMAKE_CXX_COMPILER="C:/Program Files/Microsoft Visual Studio/2022/Community/VC/Tools/Llvm/bin/clang-cl.exe" -DCMAKE_LINKER="C:/Program Files/Microsoft Visual Studio/2022/Community/VC/Tools/Llvm/bin/lld-link.exe" -DCMAKE_C_FLAGS="-m64 /EHsc --coverage" -DCMAKE_CXX_FLAGS="-m64 /EHsc --coverage" -DCMAKE_EXE_LINKER_FLAGS="-libpath:\"C:/Program Files/Microsoft Visual Studio/2022/Community/VC/Tools/Llvm/x64/lib/clang/18/lib/windows\" \"C:/Program Files/Microsoft Visual Studio/2022/Community/VC/Tools/Llvm/x64/lib/clang/18/lib/windows/clang_rt.profile-x86_64.lib\"" -DCMAKE_SHARED_LINKER_FLAGS="-libpath:\"C:/Program Files/Microsoft Visual Studio/2022/Community/VC/Tools/Llvm/x64/lib/clang/18/lib/windows\" \"C:/Program Files/Microsoft Visual Studio/2022/Community/VC/Tools/Llvm/x64/lib/clang/18/lib/windows/clang_rt.profile-x86_64.lib\""
if "%COMPILER%" == "msvc-clang" if "%APPVEYOR_BUILD_WORKER_IMAGE%" == "Visual Studio 2019" set COMPILER_EXTRA=-DCMAKE_C_COMPILER="C:/Program Files (x86)/Microsoft Visual Studio/2019/Community/VC/Tools/Llvm/bin/clang-cl.exe" -DCMAKE_CXX_COMPILER="C:/Program Files (x86)/Microsoft Visual Studio/2019/Community/VC/Tools/Llvm/bin/clang-cl.exe" -DCMAKE_LINKER="C:/Program Files (x86)/Microsoft Visual Studio/2019/Community/VC/Tools/Llvm/bin/lld-link.exe" -DCMAKE_C_FLAGS="-m64 /EHsc" -DCMAKE_CXX_FLAGS="-m64 /EHsc"

rem Build
mkdir build && cd build || exit /b
cmake .. ^
    -DCMAKE_BUILD_TYPE=%CONFIGURATION% ^
    -DCMAKE_INSTALL_PREFIX=%APPVEYOR_BUILD_FOLDER%/deps ^
    -DCORRADE_UTILITY_USE_ANSI_COLORS=%ANSI_COLORS% ^
    -DCORRADE_BUILD_TESTS=ON ^
    -DCORRADE_BUILD_STATIC=%BUILD_STATIC% ^
    %COMPILER_EXTRA% -G Ninja || exit /b
cmake --build . || exit /b

rem Test
set CORRADE_TEST_COLOR=ON
rem On Windows, if an assertion or other issue happens, A DIALOG WINDOWS POPS
rem UP FROM THE CONSOLE. And then, for fucks sake, IT WAITS ENDLESSLY FOR YOU
rem TO CLOSE IT!! Such behavior is utterly stupid in a non-interactive setting
rem such as on this very CI, so I'm setting a timeout to 60 seconds to avoid
rem the CI job being stuck for an hour if an assertion happens. CTest's default
rem timeout is somehow 10M seconds, which is as useful as nothing at all.
ctest -V --timeout 120 || exit /b

rem Test install, after running the tests as for them it shouldn't be needed
cmake --build . --target install || exit /b

rem Examples
cd %APPVEYOR_BUILD_FOLDER% || exit /b
mkdir build-examples && cd build-examples || exit /b
cmake ../src/examples ^
    -DCMAKE_BUILD_TYPE=%CONFIGURATION% ^
    -DCMAKE_PREFIX_PATH=%APPVEYOR_BUILD_FOLDER%/deps ^
    %COMPILER_EXTRA% -G Ninja || exit /b
cmake --build . || exit /b

cd %APPVEYOR_BUILD_FOLDER%

rem Gather and upload coverage on the clang-cl MSVC 2022 build. Keep in sync
rem with circleci.yml, appveyor-desktop-mingw.sh and PKBUILD-coverage, please.
if "%COMPILER%" == "msvc-clang" if "%APPVEYOR_BUILD_WORKER_IMAGE%" == "Visual Studio 2022" grcov build -t lcov --keep-only "*/src/Corrade/*" --ignore "*/Test/*" --ignore "*/build/src/Corrade/*" -o coverage.info --excl-line LCOV_EXCL_LINE --excl-start LCOV_EXCL_START --excl-stop LCOV_EXCL_STOP || exit /b
rem Official docs say "not needed for public repos", in reality not using the
rem token is "extremely flakey". What's best is that if the upload fails, the
rem damn thing exits with a success error code, and nobody cares:
rem https://github.com/codecov/codecov-circleci-orb/issues/139
rem https://community.codecov.com/t/commit-sha-does-not-match-circle-build/4266
if "%COMPILER%" == "msvc-clang" if "%APPVEYOR_BUILD_WORKER_IMAGE%" == "Visual Studio 2022" codecov -f ./coverage.info -t 5f6a19a9-4a9b-4ee8-8a0b-c0cdfbbdcccd -X gcov || exit /b
