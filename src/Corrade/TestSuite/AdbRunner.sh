#!/bin/bash
set -e

# Usage:
#  ./AdbRunner.sh /path/to/test/binary/dir executable-name additional files...
binary_dir=$1
filename=$2
# So the additional files are available in $@
shift && shift

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
if [ ! -z ${CORRADE_TEST_REPEAT_ALL+x} ]; then
    test_env="$test_env CORRADE_TEST_REPEAT_ALL=$CORRADE_TEST_REPEAT_ALL"
fi
if [ ! -z ${CORRADE_TEST_ABORT_ON_FAIL+x} ]; then
    test_env="$test_env CORRADE_TEST_ABORT_ON_FAIL=$CORRADE_TEST_ABORT_ON_FAIL"
fi
if [ ! -z ${CORRADE_TEST_NO_XFAIL+x} ]; then
    test_env="$test_env CORRADE_TEST_NO_XFAIL=$CORRADE_TEST_NO_XFAIL"
fi
if [ ! -z ${CORRADE_BENCHMARK+x} ]; then
    test_env="$test_env CORRADE_BENCHMARK=$CORRADE_BENCHMARK"
fi
if [ ! -z ${CORRADE_BENCHMARK_DISCARD+x} ]; then
    test_env="$test_env CORRADE_BENCHMARK_DISCARD=$CORRADE_BENCHMARK_DISCARD"
fi
if [ ! -z ${CORRADE_BENCHMARK_YELLOW+x} ]; then
    test_env="$test_env CORRADE_BENCHMARK_YELLOW=$CORRADE_BENCHMARK_YELLOW"
fi
if [ ! -z ${CORRADE_BENCHMARK_RED+x} ]; then
    test_env="$test_env CORRADE_BENCHMARK_RED=$CORRADE_BENCHMARK_RED"
fi

# Create a local temporary directory. Android doesn't have mktemp, so we have
# to assume that there is ever only one computer connected to a device /
# emulator and so mktemp always returns unique value.
tmpdir=$(mktemp -d /tmp/corrade-testsuite-XXXXX)
remote_tmpdir=/data/local$tmpdir

# The device / emulator might have stale temporary directories that could clash
# with the newly created one. But given the above they should not be used
# anymore so we remove them and then recreate the directory.
adb shell 'rm -rf '$remote_tmpdir'; mkdir '$remote_tmpdir

# Push the test executable and also all required files to the remote temporary
# directory, preserving directory structure
adb push "$binary_dir/$filename" $remote_tmpdir | tail -n 1
for file in "$@"; do
    # TODO: this will probably break horribly when the filenames contain spaces
    # and/or multiple @ characters (only the last should be taken). Sorry about
    # that, if you fix it and provide a patch, I'll be *very* happy.
    file_pair=(${file//@/ })
    dir=$(dirname ${file_pair[1]})
    adb shell "mkdir -p $remote_tmpdir/$dir"
    adb push "${file_pair[0]}" "$remote_tmpdir/${file_pair[1]}" | tail -n 1
done

# No comment. https://code.google.com/p/android/issues/detail?id=3254
adb shell $test_env' cd '$remote_tmpdir' && ./'$filename'; echo -n ADB_IS_SHIT:$?' | tee $tmpdir/adb.retval | grep -v ADB_IS_SHIT
returncode=$(grep ADB_IS_SHIT $tmpdir/adb.retval | cut -d':' -f2)

# Clean up after ourselves -- remove the temporary directories both on local
# machine and device / emulator. This is not done if any of the above fails,
# but that's okay -- it should stay there to be able to debug the problems
adb shell 'rm -r '$remote_tmpdir
rm -r $tmpdir

# Propagate the return code
exit $returncode
