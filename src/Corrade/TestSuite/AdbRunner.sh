#!/bin/bash
set -e

if [ "$(adb get-state | tr -d '\r\n')" != "device" ]; then
    echo "ERROR: no device connected"
    exit 1
fi

# Propagate relevant test environment variables. If CORRADE_TEST_COLOR is not
# set, detect isatty() on client-side and pass it through
test_env=
if [ ! -z ${CORRADE_TEST_COLOR+x} ]; then
    test_env="$test_env CORRADE_TEST_COLOR=$CORRADE_TEST_COLOR"
elif [ -t 1 ]; then
    test_env="$test_env CORRADE_TEST_COLOR=ON"
else
    test_env="$test_env CORRADE_TEST_COLOR=OFF"
fi
if [ ! -z ${CORRADE_TEST_SKIP_TESTS+x} ]; then
    test_env="$test_env CORRADE_TEST_SKIP_TESTS=$CORRADE_TEST_SKIP_TESTS"
fi
if [ ! -z ${CORRADE_TEST_SKIP_BENCHMARKS+x} ]; then
    test_env="$test_env CORRADE_TEST_SKIP_BENCHMARKS=$CORRADE_TEST_SKIP_BENCHMARKS"
fi
if [ ! -z ${CORRADE_TEST_SHUFFLE+x} ]; then
    test_env="$test_env CORRADE_TEST_SHUFFLE=$CORRADE_TEST_SHUFFLE"
fi
if [ ! -z ${CORRADE_TEST_REPEAT_EVERY+x} ]; then
    test_env="$test_env CORRADE_TEST_REPEAT_EVERY=$CORRADE_TEST_REPEAT_EVERY"
fi
if [ ! -z ${CORRADE_TEST_NO_XFAIL+x} ]; then
    test_env="$test_env CORRADE_TEST_NO_XFAIL=$CORRADE_TEST_NO_XFAIL"
fi

adb push $1 /data/local/tmp
# No comment. https://code.google.com/p/android/issues/detail?id=3254
adb shell $test_env' /data/local/tmp/'$2'; echo -n ADB_IS_SHIT:$?; rm /data/local/tmp/'$2 | tee /tmp/adb.retval | grep -v ADB_IS_SHIT
exit $(grep ADB_IS_SHIT /tmp/adb.retval | cut -d':' -f2)
