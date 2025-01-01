rem Workaround for CMake not wanting sh.exe on PATH for MinGW. AARGH.
set PATH=%PATH:C:\Program Files\Git\usr\bin;=%
set PATH=C:\mingw-w64\x86_64-7.2.0-posix-seh-rt_v5-rev1\mingw64\bin;%APPVEYOR_BUILD_FOLDER%\deps\bin;%PATH%

rem Build
mkdir build && cd build || exit /b
cmake .. ^
    -DCMAKE_CXX_FLAGS="--coverage" ^
    -DCMAKE_BUILD_TYPE=Debug ^
    -DCMAKE_INSTALL_PREFIX=%APPVEYOR_BUILD_FOLDER%/deps ^
    -DCORRADE_UTILITY_USE_ANSI_COLORS=ON ^
    -DCORRADE_BUILD_TESTS=ON ^
    -G Ninja || exit /b
cmake --build . || exit /b

rem Test
cd %APPVEYOR_BUILD_FOLDER%/build || exit /b
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

rem Examples. The --coverage flag needs to be specified as well otherwise
rem linking to CorradeMain will result in undefined reference to __gcov_init
rem and such.
cd %APPVEYOR_BUILD_FOLDER% || exit /b
mkdir build-examples && cd build-examples || exit /b
cmake ../src/examples ^
    -DCMAKE_CXX_FLAGS="--coverage" ^
    -DCMAKE_BUILD_TYPE=Debug ^
    -DCMAKE_PREFIX_PATH=%APPVEYOR_BUILD_FOLDER%/deps ^
    -G Ninja || exit /b
cmake --build . || exit /b

rem Coverage upload
cd %APPVEYOR_BUILD_FOLDER%
set GCOV=C:/mingw-w64/x86_64-7.2.0-posix-seh-rt_v5-rev1/mingw64/bin/gcov.exe
rem Keep in sync with circleci.yml, appveyor-desktop.bat and PKBUILD-coverage,
rem please. The --source-dir needs to be present in order to circumvent
rem     Warning: "../foo" cannot be normalized because of "..", so skip it.
rem that happens with CMake before 3.21 that doesn't yet pass full paths to
rem Ninja: https://github.com/mozilla/grcov/issues/1182
grcov build -t lcov --source-dir %APPVEYOR_BUILD_FOLDER%/build --keep-only "*/src/Corrade/*" --ignore "*/Test/*" --ignore "*/build/src/Corrade/*" -o coverage.info --excl-line LCOV_EXCL_LINE --excl-start LCOV_EXCL_START --excl-stop LCOV_EXCL_STOP  || exit /b
rem Official docs say "not needed for public repos", in reality not using the
rem token is "extremely flakey". What's best is that if the upload fails, the
rem damn thing exits with a success error code, and nobody cares:
rem https://github.com/codecov/codecov-circleci-orb/issues/139
rem https://community.codecov.com/t/commit-sha-does-not-match-circle-build/4266
codecov -f ./coverage.info -t 5f6a19a9-4a9b-4ee8-8a0b-c0cdfbbdcccd -X gcov || exit /b
