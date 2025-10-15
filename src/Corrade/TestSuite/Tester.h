#ifndef Corrade_TestSuite_Tester_h
#define Corrade_TestSuite_Tester_h
/*
    This file is part of Corrade.

    Copyright © 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016,
                2017, 2018, 2019, 2020, 2021, 2022, 2023, 2024, 2025
              Vladimír Vondruš <mosra@centrum.cz>

    Permission is hereby granted, free of charge, to any person obtaining a
    copy of this software and associated documentation files (the "Software"),
    to deal in the Software without restriction, including without limitation
    the rights to use, copy, modify, merge, publish, distribute, sublicense,
    and/or sell copies of the Software, and to permit persons to whom the
    Software is furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included
    in all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
    THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
    FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
    DEALINGS IN THE SOFTWARE.
*/

/** @file
 * @brief Class @ref Corrade::TestSuite::Tester, @ref Corrade::TestSuite::TestCaseDescriptionSourceLocation, macros @ref CORRADE_TEST_MAIN(), @ref CORRADE_VERIFY(), @ref CORRADE_COMPARE(), @ref CORRADE_COMPARE_AS(), @ref CORRADE_COMPARE_WITH(), @ref CORRADE_EXPECT_FAIL(), @ref CORRADE_EXPECT_FAIL_IF(), @ref CORRADE_INFO(), @ref CORRADE_WARN(), @ref CORRADE_FAIL(), @ref CORRADE_FAIL_IF(), @ref CORRADE_SKIP(), @ref CORRADE_SKIP_IF_NO_ASSERT(), @ref CORRADE_SKIP_IF_NO_DEBUG_ASSERT(), @ref CORRADE_ITERATION(), @ref CORRADE_BENCHMARK()
 */

#include <cstdint>
#include <initializer_list>

#include "Corrade/Containers/Pointer.h"
#include "Corrade/TestSuite/Comparator.h"
#include "Corrade/TestSuite/Compare/FloatingPoint.h"
#include "Corrade/TestSuite/visibility.h"
#include "Corrade/Utility/Debug.h"
#include "Corrade/Utility/Macros.h"

#ifdef CORRADE_BUILD_DEPRECATED
/* Some arguments used to be a std::string, so provide implicit conversion to a
   StringView */
#include "Corrade/Containers/StringStl.h"
#endif

namespace Corrade { namespace TestSuite {

namespace Implementation {
    /* First try to convert the actual type to expected, if that fails, try
       std::common_type */
    template<class Actual, class Expected, bool = std::is_convertible<Actual, Expected>::value> struct CommonType {
        typedef Expected Type;
    };
    template<class Actual, class Expected> struct CommonType<Actual, Expected, false> {
        typedef typename std::common_type<Actual, Expected>::type Type;
    };
}

class TestCaseDescriptionSourceLocation;

/**
@brief Base class for tests and benchmarks

Supports colored output, instanced (or data-driven) tests, repeated tests (e.g.
for testing race conditions) and benchmarks, which can either use one of the
builtin measurement functions (such as wall time, CPU time or CPU cycle count)
or any user-provided custom measurement function (for example measuring
allocations, memory usage, GPU timings etc.). In addition, the behavior of the
test execution can be configured  via many command-line and environment
options.

Make sure to first go through the @ref testsuite tutorial for initial overview
and step-by-step introduction. Below is a more detailed description of all
provided functionality.

@section TestSuite-Tester-basic Basic testing workflow

A test starts with deriving the @ref Tester class. The test cases are
parameter-less @cpp void @ce member functions that are added using @ref addTests()
in the constructor and the @cpp main() @ce function is created using
@ref CORRADE_TEST_MAIN(). The goal is to have as little boilerplate as
possible, thus the test usually  consists of only one `*.cpp` file with no
header and the derived type is a @cpp struct @ce to avoid having to write
@cpp public @ce keywords. It's also advised to wrap everything except the
@ref CORRADE_TEST_MAIN() macro in an unnamed namespace, as that will make the
compiler tell you about accidentally unused test cases or variables.

@snippet testsuite-basic.cpp 0

The above gives the following output:

@include testsuite-basic.ansi

Actual testing is done via various @ref CORRADE_VERIFY(), @ref CORRADE_COMPARE(),
@ref CORRADE_COMPARE_AS() and other macros. If some comparison in given test
case fails, a @cb{.ansi} [1;31mFAIL @ce with concrete file, line and
additional diagnostic is printed to the output and the test case is exited
without executing the remaining statements. Otherwise, if all comparisons in
given test case pass, an @cb{.ansi} [1;39mOK @ce is printed. The main
difference between these macros is the kind of diagnostic output they print
when comparison fails --- for example a simple expression failure reported by
@ref CORRADE_VERIFY() is enough when checking for non-@cpp nullptr @ce value,
but for comparing two strings you may want to use @ref CORRADE_COMPARE() so you
can not only see that they differ, but also *how* they differ.

Additionally there are @ref CORRADE_SKIP(), @ref CORRADE_EXPECT_FAIL() and
@ref CORRADE_EXPECT_FAIL_IF() control flow helpers that allow you to say for
example that a particular test was skipped due to missing functionality on
given platform (printing a @cb{.ansi} [1;39mSKIP @ce in the output and exiting
the test case right after the statement) or documenting that some algorithm
produces incorrect result due to a bug, printing an @cb{.ansi} [1;33mXFAIL @ce.
Passing a test while failure is expected is treated as an error
(@cb{.ansi} [1;31mXPASS @ce), which can be helpful to ensure the assumptions
in the tests don't get stale. Expected failures can be also disabled globally
via a command-line option `--no-xfail` or via environment variable,
@ref TestSuite-Tester-command-line "see below".

Finally, while it's possible to use @ref Utility::Debug and any other APIs for
printing to the standard output, using the @ref CORRADE_INFO() or
@ref CORRADE_WARN() macros will make the output prefixed with
@cb{.ansi} [1;39mINFO @ce or @cb{.ansi} [1;33mWARN @ce, name of the test case
as well as file/line information. The @ref CORRADE_FAIL_IF() macro is then
useful as an alternative to @ref CORRADE_VERIFY() / @ref CORRADE_COMPARE() when
the implicit diagnostic message is insufficient --- if the condition fails,
it'll just print given message prefixed with @cb{.ansi} [1;31mFAIL @ce and the
test case is exited.

The only reason why those are macros and not member functions is the ability to
gather class/function/file/line/expression information via the preprocessor for
printing the test output and exact location of possible test failure. If none
of the @ref CORRADE_VERIFY(), @ref CORRADE_COMPARE() plus variants,
@ref CORRADE_FAIL_IF() or @ref CORRADE_SKIP()  macros is encountered when
running the test case, the test case is reported as invalid, with
@cb{.ansi} [1;33m? @ce in the output, and that causes the whole test run to
fail as well. This is done in order to prevent accidents where nothing actually
gets verified.

The test cases are numbered in the output and those numbers can be used on the
command-line to whitelist/blacklist the test cases with `--only`/`--skip`,
randomly reorder them using `--shuffle` and more,
@ref TestSuite-Tester-command-line "see below" for details. In total, when all
test cases pass, the executable exits with `0` return code, in case of failure
or invalid test case it exits with `1` to make it possible to run the tests in
batch (such as CMake CTest). By default, after a failure, the testing continues
with the other test cases, you can abort after first failure using
`--abort-on-fail` command-line option.

Useful but not immediately obvious is the possibility to use templated member
functions as test cases, for example when testing a certain algorithm on
different data types:

@snippet testsuite-templated.cpp 0

And the corresponding output:

@include testsuite-templated.ansi

This works with all `add*()` functions though please note that current C++11
compilers (at least GCC and Clang) are not able to properly detect class type
when passing only templated functions to it, so you may need to specify the
type explicitly. Also, there is no easy and portable way to get function name
with template parameters so by default it will be just the function name, but
you can call @ref setTestCaseName() to specify a full name.

@section TestSuite-Tester-instanced Instanced tests

Often you have an algorithm which you need to test on a variety of inputs or
corner cases. One solution is to use a for-cycle inside the test case to test
on all inputs, but then the diagnostic on error will not report which input is
to blame. Another solution is to duplicate the test case multiple times for all
the different inputs, but that becomes a maintenance nightmare pretty quickly.
Making the function with a non-type template parameter is also a solution, but
that's not possible to do for all types and with huge input sizes it is often
not worth the increased compilation times. Fortunately, there is an
@ref addInstancedTests() that comes for the rescue:

@snippet testsuite-instanced.cpp 0

Corresponding output:

@include testsuite-instanced.ansi

The tester class just gives you an instance index via @ref testCaseInstanceId()
and it's up to you whether you use it as an offset to some data array or
generate an input using it, the above example is just a hint how one might use
it. Each instance is printed to the output separately and if one instance
fails, it doesn't stop the other instances from being executed. Similarly to
the templated tests, @ref setTestCaseDescription() allows you to set a
human-readable description of given instance. If not called, the instances are
just numbered in the output.

See also the @ref TestCaseDescriptionSourceLocation class for improved
file/line diagnostics for instanced test cases.

@section TestSuite-Tester-iteration-annotations Testing in a loop

While instanced tests are usually the go-to solution when testing on a larger
set of data, sometimes you need to loop over a few values and check them one by
one. When such test fails, it's often hard to know which particular value
caused the failure. To fix that, you can use the @ref CORRADE_ITERATION() macro
to annotate current iteration in case of a failure. It works with any type
printable via @ref Utility::Debug and handles nested loops as well. Silly
example:

@snippet testsuite-iteration.cpp 0

On failure, the iteration value(s) will be printed next to the file/line info:

@include testsuite-iteration.ansi

This macro isn't limited to just loops, it can be used to provide more context
to just any check. See also @ref Compare::Container for a convenient way of
comparing container contents.

@section TestSuite-Tester-repeated Repeated tests

A complementary feature to instanced tests are repeated tests using
@ref addRepeatedTests(), useful for example to repeatedly call one function
10000 times to increase probability of potential race conditions. The
difference from instanced tests is that all repeats are treated as executing
the same code and thus only the overall result is reported in the output. Also
unlike instanced tests, if a particular repeat fails, no further repeats are
executed. The test output contains the number of executed repeats after the
test case name, prefixed by @cb{.ansi} [1;39m@ @ce. Example of testing race
conditions with multiple threads accessing the same variable:

@snippet testsuite-repeated.cpp 0

Depending on various factors, here is one possible output:

@include testsuite-repeated.ansi

Similarly to @ref testCaseInstanceId() there is @ref testCaseRepeatId() which
gives repeat index. Use with care, however, as the repeated tests are assumed
to execute the same code every time. On
@ref TestSuite-Tester-command-line "the command line" it is possible to
increase repeat count via `--repeat-every`. In addition there is `--repeat-all`
which behaves as like all `add*()` functions in the constructor were called
multiple times in a loop. Combined with `--shuffle` this can be used to run the
test cases multiple times in a random order to uncover potential unwanted
interactions and order-dependent bugs.

It's also possible to combine instanced and repeated tests using
@ref addRepeatedInstancedTests().

@section TestSuite-Tester-advanced-comparisons Advanced comparisons

While the diagnostic provided by @ref CORRADE_COMPARE() is definitely better
than just knowing that something failed, the @ref CORRADE_COMPARE_AS() and
@ref CORRADE_COMPARE_WITH() macros allow for advanced comparison features in
specialized cases. The @ref Compare namespace contains various builtin
comparators, some of which are listed below. It's also possible to implement
custom comparators for your own use cases --- see the @ref Comparator class for
details.

<ul>
<li>
@ref Compare::Container and @relativeref{Compare,SortedContainer} can be used
for convenient comparison of large containers. Compared to plain
@ref CORRADE_COMPARE() Its diagnostic will show where exactly the containers
differ, which becomes useful with large data sizes:

@snippet TestSuite.cpp Compare-Container
</li>
<li>
@ref Compare::String provides multi-line diffs; @relativeref{Compare,StringHasPrefix}, @relativeref{Compare,StringHasSuffix},
@relativeref{Compare,StringContains} and @relativeref{Compare,StringNotContains}
provide a better diagnostic compared to e.g. checking
@ref Containers::StringView::hasPrefix() with @ref CORRADE_VERIFY():

@snippet TestSuite.cpp Compare-StringHasPrefix
</li>
<li>
@ref Compare::File, @relativeref{Compare,FileToString} and
@relativeref{Compare,StringToFile} allow you to compare files without having to
manually read them:

@snippet TestSuite.cpp Compare-File
</li>
<li>
@ref Compare::Less, @relativeref{Compare,Greater}, @relativeref{Compare,Around}
and others from the @ref Corrade/TestSuite/Compare/Numeric.h header allow you
to do numeric comparisons that again provide better failure reporting compared
to checking an expression result with @ref CORRADE_VERIFY():

@snippet TestSuite.cpp Compare-around-just-one
</li>
<li>
Finally, [Magnum](https://magnum.graphics) has more comparators, including a
@m_class{m-doc-external} [DebugTools::CompareImage](https://doc.magnum.graphics/magnum/classMagnum_1_1DebugTools_1_1CompareImage.html)
that's able to fuzzy-compare two images pixel-by-pixel and visualize the
difference as an ASCII art directly in the console.
</li>
</ul>

@section TestSuite-Tester-save-diagnostic Saving diagnostic files

On comparison failure, it's sometimes desirable to inspect the generated data
with an external tool. Or, in case the expected test data need to be updated,
it's easier to copy over the generated data to the original file than applying
changes manually. To make this easier without needing to add file-saving to the
test itself, pass a path to the `--save-diagnostic`
@ref TestSuite-Tester-command-line "command-line option". Comparators that
operate with files (such as @ref Compare::File or @ref Compare::StringToFile)
will then use this path to save the actual data under the same filename as the
expected file, notifying you about the operation with a
@cb{.ansi} [1;32mSAVED @ce message:

@include testsuite-save-diagnostic.ansi

Note that this functionality is not restricted to just saving the actual
compared data (or to comparison failures) --- third-party comparators can use
it for generating diffs or providing further diagnostic meant to be viewed
externally. See the @ref TestSuite-Comparator-save-diagnostic "Comparator"
class for further information.

@section TestSuite-Tester-benchmark Benchmarks

Besides verifying code correctness, it's possible to measure code performance.
Unlike correctness tests, the benchmark results are hard to reason about using
only automated means, so there are no macros for verifying benchmark results
and instead the measured values are just printed to the output for users to
see. Benchmarks can be added using @ref addBenchmarks(), the actual benchmark
loop is marked by @ref CORRADE_BENCHMARK() and the results are printed to
output with @cb{.ansi} [1;39mBENCH @ce identifier. Example benchmark comparing
performance of inverse square root implementations:

@snippet testsuite-benchmark.cpp 0

Note that it's not an error to add one test/benchmark multiple times --- here
it is used to have the same code benchmarked with different timers. Possible
output:

@include testsuite-benchmark.ansi

The number passed to @ref addBenchmarks() is equivalent to repeat count passed
to @ref addRepeatedTests() and specifies measurement sample count. The number
passed to @ref CORRADE_BENCHMARK() is number of iterations of the inner loop in
one sample measurement to amortize the overhead and error caused by clock
precision --- the faster the measured code is the more iterations it needs. The
measured value is then divided by that number to represent cost of a single
iteration. The @ref testCaseRepeatId() returns current sample index and can be
used to give some input variation to the test. By default the benchmarks
measure wall clock time, see @ref BenchmarkType for other types of builtin
benchmarks. The default benchmark type can be also overridden
@ref TestSuite-Tester-command-line "on the command-line" via `--benchmark`.

It's possible to use all @ref CORRADE_VERIFY(), @ref CORRADE_COMPARE() etc.
verification macros inside the benchmark to check pre/post-conditions. If one
of them fails, the benchmark is treated in the output just like a failing test,
with no benchmark results being printed out. Keep in mind, however, that those
macros have some overhead, so try to not use them inside the benchmark loop.

The benchmark output is calculated from all samples except the first discarded
samples. By default that's one sample, `--benchmark-discard` and `--repeat-every`
@ref TestSuite-Tester-command-line "command-line options" can be used to
override how many samples are taken and how many of them are discarded at
first. In the output, the used sample count and sample size is printed after
test case name, prefixed with `@`. The output contains mean value and a sample
standard deviation, calculated as: @f[
    \begin{array}{rcl}
        \bar{x} & = & \dfrac{1}{N} \sum\limits_{i=1}^N x_i \\ \\
        \sigma_x & = & \sqrt{\dfrac{1}{N-1} \sum\limits_{i=1}^N \left( x_i - \bar{x} \right)^2}
    \end{array}
@f]

Different benchmark type have different units. Depending on value magnitude,
larger units may be used as documented in @ref BenchmarkUnits. For easier
visual recognition of the values, by default the sample standard deviation is
colored yellow if it is larger than 5% of the absolute value of the mean and
red if it  is larger than 25% of the absolute value of the mean. This can be
overridden @ref TestSuite-Tester-command-line "on the command-line" via
`--benchmark-yellow` and `--benchmark-red`. See
@ref TestSuite-Tester-running-benchmark-noise below for various ways of
achieving more stable results.

It's possible to have instanced benchmarks as well, see
@ref addInstancedBenchmarks().

@section TestSuite-Tester-benchmark-custom Custom benchmarks

It's possible to specify a custom pair of functions for intiating the benchmark
and returning the result using @ref addCustomBenchmarks(). The benchmark end
function returns an unsigned 64-bit integer indicating measured amount in units
given by @ref BenchmarkUnits. To further describe the value being measured you
can call @ref setBenchmarkName() in the benchmark begin function. Contrived
example of benchmarking number of copies when using @ref std::vector::push_back():

@snippet testsuite-benchmark-custom.cpp 0

Running the benchmark shows that calling @ref std::vector::push_back() "push_back()"
for 10 thousand elements actually causes the copy constructor to be called 26
thousand times:

@include testsuite-benchmark-custom.ansi

@section TestSuite-Tester-setup-teardown Specifying setup/teardown routines

While the common practice in C++ is to use RAII for resource lifetime
management, sometimes you may need to execute arbitrary code at the beginning
and end of each test case. For this, all @ref addTests(),
@ref addInstancedTests(), @ref addRepeatedTests(), @ref addRepeatedInstancedTests(),
@ref addBenchmarks(), @ref addInstancedBenchmarks(), @ref addCustomBenchmarks()
and @ref addCustomInstancedBenchmarks() have an overload that is additionally
taking a pair of parameter-less @cpp void @ce functions for setup and teardown.
Both functions are called before and after each test case run, independently on
whether the test case passed or failed.

@section TestSuite-Tester-exceptions Catching exceptions

If a test case fails with an unhandled exception, a @cb{.ansi} [1;31mTHROW @ce
is printed on the output, together with a (platform-specific mangled) name of
the exception type and contents of @ref std::exception::what(). No file/line
info is provided in this case, as it's not easily possible to know where the
exception originated from. Only exceptions derived from @ref std::exception are
caught to avoid interfering with serious issues such as memory access errors.
If catching unhandled exceptions is not desired (for example when you want to
do a post-mortem debugging of the stack trace leading to the exception), it can
be disabled with the `--no-catch` @ref TestSuite-Tester-command-line "command-line option".

Apart from the above, the test suite doesn't provide any builtin exception
support --- if it's needed to verify that an exception was or wasn't thrown,
the user is expected to implement a @cpp try {} catch @ce block inside the test
case and verify the desired properties directly.

@section TestSuite-Tester-command-line Command-line options

Command-line options that make sense to be set globally for multiple test cases
are also configurable via environment variables for greater flexibility when
for example running the tests in a batch via `ctest`.

Usage:

@code{.shell-session}
./my-test [-h|--help] [-c|--color on|off|auto] [--skip N1,N2-N3…]
    [--skip-tests] [--skip-benchmarks] [--only N1,N2-N3…] [--shuffle]
    [--repeat-every N] [--repeat-all N] [--abort-on-fail] [--no-xfail]
    [--no-catch] [--save-diagnostic PATH] [--verbose] [--benchmark TYPE]
    [--benchmark-discard N] [--benchmark-yellow N] [--benchmark-red N]
@endcode

Arguments:

-   `-h`, `--help` --- display this help message and exit
-   `-c`, `--color on|off|auto` --- colored output (environment:
    `CORRADE_TEST_COLOR`, default: `auto`). The `auto` option enables color
    output in case an interactive terminal is detected. Note that on Windows it
    is possible to output colors only directly to an interactive terminal
    unless @ref CORRADE_UTILITY_USE_ANSI_COLORS is defined.
-   `--skip N1,N2-N3…` --- skip test cases with given numbers. See
    @ref Utility::String::parseNumberSequence() for syntax description.
-   `--skip-tests` --- skip all tests (environment:
    `CORRADE_TEST_SKIP_TESTS=ON|OFF`)
-   `--skip-benchmarks` --- skip all benchmarks (environment:
    `CORRADE_TEST_SKIP_BENCHMARKS=ON|OFF`)
-   `--only N1,N2-N3…` --- run only test cases with given numbers. See
    @ref Utility::String::parseNumberSequence() for syntax description.
-   `--shuffle` --- randomly shuffle test case order (environment:
    `CORRADE_TEST_SHUFFLE=ON|OFF`)
-   `--repeat-every N` --- repeat every test case N times (environment:
    `CORRADE_TEST_REPEAT_EVERY`, default: `1`)
-   `--repeat-all N` --- repeat all test cases N times (environment:
    `CORRADE_TEST_REPEAT_ALL`, default: `1`)
-   `-X`, `--abort-on-fail` --- abort after first failure (environment:
    `CORRADE_TEST_ABORT_ON_FAIL=ON|OFF`)
-   `--no-xfail` --- disallow expected failures (environment:
    `CORRADE_TEST_NO_XFAIL=ON|OFF`)
-   `--no-catch` --- don't catch standard exceptions (environment:
    `CORRADE_TEST_NO_CATCH=ON|OFF`)
-   `-S`, `--save-diagnostic PATH` --- save diagnostic files to given path
    (environment: `CORRADE_TEST_SAVE_DIAGNOSTIC`)
-   `-v`, `--verbose` --- enable verbose output (environment:
    `CORRADE_TEST_VERBOSE=ON|OFF`). Note that there isn't any corresponding
    "quiet" option, if you want to see just the failures, redirect standard
    output away.
-   `--benchmark TYPE` --- default benchmark type (environment:
    `CORRADE_TEST_BENCHMARK`). Supported benchmark types:
    -   `wall-time` --- wall time spent
    -   `cpu-time` --- CPU time spent
    -   `cpu-cycles` --- CPU cycles spent (x86 only, gives zero result
        elsewhere)
-   `--benchmark-discard N` --- discard first N measurements of each benchmark
    (environment: `CORRADE_TEST_BENCHMARK_DISCARD`, default: `1`)
-   `--benchmark-yellow N` --- deviation threshold for marking benchmark yellow
    (environment: `CORRADE_TEST_BENCHMARK_YELLOW`, default: `0.05`)
-   `--benchmark-red N` --- deviation threshold for marking benchmark red
    (environment: `CORRADE_TEST_BENCHMARK_RED`, default: `0.25`)

@section TestSuite-Tester-running Compiling and running tests

In general, just compiling the executable and linking it to the TestSuite
library is enough, no further setup is needed. When running, the test produces
output to standard output / standard error and exits with non-zero code in case
of a test failure.

@subsection TestSuite-Tester-running-cmake Using CMake

If you are using CMake, there's a convenience @ref corrade-cmake-add-test "corrade_add_test()"
CMake macro that creates the executable, links `Corrade::TestSuite` library to
it and adds it to CTest. Besides that it is able to link other arbitrary
libraries to the executable and specify a list of files that the tests used. It
provides additional useful features on various platforms:

-   On Windows, the macro links the test executable to the @ref main "Corrade::Main"
    library for ANSI color support, UTF-8 argument parsing and UTF-8 output
    encoding.
-   If compiling for Emscripten, using @ref corrade-cmake-add-test "corrade_add_test()"
    makes CTest run the resulting `*.js` file via Node.js. Also it is able to
    bundle all files specified in `FILES` into the virtual Emscripten
    filesystem, making it easy to run file-based tests on this platform; all
    environment options are passed through as well. The macro also creates a
    runner for manual testing in a browser, see
    @ref TestSuite-Tester-running-emscripten-browser "below" for more
    information.
-   If Xcode projects are generated via CMake and @ref CORRADE_TESTSUITE_TARGET_XCTEST
    is enabled, @ref corrade-cmake-add-test "corrade_add_test()" makes the test
    executables in a way compatible with XCTest, making it easy to run them
    directly from Xcode. Running the tests via `ctest` will also use XCTest.
-   If building for Android, using @ref corrade-cmake-add-test "corrade_add_test()"
    will make CTest upload the test executables and all files specified in `FILES`
    onto the device or emulator via `adb`, run it there with all environment
    options passed through as well and transfers test results back to the host.
    @attention If a test fails, the executable and all files it references is
        kept in `/data/local/tmp` to make it possible to debug / re-run the
        test directly. Note that in the extreme case this may cause the
        internal storage to get filled over time. Rebooting the device or
        doing system cleanup will wipe the temp directory.

.

Example of using the @ref corrade-cmake-add-test "corrade_add_test()" macro is
below. The test executable will get built from the specified source with the
libJPEG library linked and the `*.jpg` files will be available on desktop,
Emscripten and Android in path specified in `JPEG_TEST_DIR` that was saved into
the `configure.h` file inside current build directory:

@snippet testsuite.cmake 0

@subsection TestSuite-Tester-running-android Manually running the tests on Android

If not using CMake CTest, Android tests can be run manually. When you have
developer-enabled Android device connected or Android emulator running, you can
use ADB to upload the built test to device temp directory and run it there:

@code{.sh}
adb push <path-to-the-test-build>/MyTest /data/local/tmp
adb shell /data/local/tmp/MyTest
@endcode

You can also use @cb{.sh} adb shell @ce to log directly into the device shell
and continue from there. All @ref TestSuite-Tester-command-line "command-line arguments"
are supported.

@note Keep in mind that older versions of ADB and Android do not correctly
    propagate the exit code to caller, which may result in your test failures
    being silently ignored. See [Android Issue 3254](https://web.archive.org/web/20160806094132/https://code.google.com/p/android/issues/detail?id=3254)
    for possible workarounds. The @ref corrade-cmake-add-test "corrade_add_test()"
    CMake macro also works around this issue.

@subsection TestSuite-Tester-running-emscripten Manually running the tests on Emscripten

When not using CMake CTest, Emscripten tests can be run directly using Node.js.
Emscripten sideloads the WebAssembly or asm.js binary files from current
working directory, so it's needed to @cb{.sh} cd @ce into the test build
directory first:

@code{.sh}
cd <test-build-directory>
node MyTest.js
@endcode

See also the `--embed-files` [emcc option](https://kripken.github.io/emscripten-site/docs/porting/files/packaging_files.html)
for a possibility to bundle test files with the executable.

@subsection TestSuite-Tester-running-emscripten-browser Running Emscripten tests in a browser

Besides running tests using Node.js, it's possible to run each test case
manually in a browser. Browsers require the executables to be accessed via a
webserver --- if you have Python installed, you can simply start serving the
contents of your build directory using the following command:

@code{.sh}
cd <test-build-directory>
python -m http.server
@endcode

The webserver is then available at http://localhost:8000. It supports directory
listing, so you can navigate to each test case runner HTML file (look for e.g.
`MyTest.html`). Unfortunately it's at the moment not possible to run all
browser tests in a batch or automate the process in any other way.

@subsection TestSuite-Tester-running-benchmark-noise Mitigating noise in CPU benchmark results

CPU frequency scaling, which is often enabled by default for power saving
reasons, can add a lot of noise to benchmarks that measure time. Picking a
higher iteration and repeat count in @ref CORRADE_BENCHMARK() and
@ref addBenchmarks() has the effect of putting more strain on the system,
forcing it to run at a higher frequency for longer period of time, which
together with having more data to average tends to produce more stable results.
However it's often impractical to hardcode them to a too high value as it hurts
iteration times, and the repeat count needed for stable results may vary wildly
between debug and release builds.

To quickly increase repeat count when running the test it's possible to use
the `--repeat-every` @ref TestSuite-Tester-command-line "command-line option"
or the corresponding environment variable. The `--repeat-all` option, possibly
combined with `--shuffle`, will result in benchmarks being run and appearing in
the output several times and possibly in random order, which could uncover
various otherwise hard-to-detect implicit dependencies between the code being
measured and application or system state such as cold caches.

On Linux or Android the test runner will attempt to query the CPU frequency
scaling governor. If it's not set to `performance`, the benchmark output will
contain a warning, with `-v` / `--verbose` showing a concrete suggestion how to
fix it. Switching to a performance governor can be done with `cpupower` on
Linux:

@code{.sh}
sudo cpupower frequency-set --governor performance
@endcode

An equivalent command on Android is the following,
which requires a rooted device:
*/
/// <b></b>
///
/// @code{.sh}
/// echo performance | tee /sys/devices/system/cpu/cpu*/cpufreq/scaling_governor
/// @endcode
///
class CORRADE_TESTSUITE_EXPORT Tester {
    public:
        /**
         * @brief Tester configuration
         *
         * @see @ref Tester::Tester()
         */
        class CORRADE_TESTSUITE_EXPORT TesterConfiguration {
            public:
                explicit TesterConfiguration() noexcept;

                /** @brief Copy constructor */
                TesterConfiguration(const TesterConfiguration&);

                /** @brief Move constructor */
                TesterConfiguration(TesterConfiguration&&) noexcept;

                ~TesterConfiguration();

                /** @brief Copy assignment */
                TesterConfiguration& operator=(const TesterConfiguration&);

                /** @brief Move assignment */
                TesterConfiguration& operator=(TesterConfiguration&&) noexcept;

                /** @brief Skipped argument prefixes */
                /* The getter is used only by tests so the Array allocation is
                   fine */
                Containers::Array<Containers::StringView> skippedArgumentPrefixes() const;

                /**
                 * @brief Set skipped argument prefixes
                 *
                 * Useful to allow passing command-line arguments elsewhere
                 * without having the tester complaining about them.
                 *
                 * Views that have both @ref Containers::StringViewFlag::Global
                 * and @relativeref{Containers::StringViewFlag,NullTerminated}
                 * set (such as coming from a @ref Containers::StringView
                 * literal) will be used without having to make an owned string
                 * copy internally.
                 * @see @ref arguments()
                 */
                TesterConfiguration& setSkippedArgumentPrefixes(std::initializer_list<Containers::StringView> prefixes);
                /* So people aren't forced to include StringView if they don't want */
                TesterConfiguration& setSkippedArgumentPrefixes(std::initializer_list<const char*> prefixes); /**< @overload */

                #if defined(__linux__) || defined(DOXYGEN_GENERATING_OUTPUT)
                /**
                 * @brief Where to check for active CPU scaling governor
                 * @partialsupport Available only on Linux.
                 */
                Containers::StringView cpuScalingGovernorFile() const;

                /**
                 * @brief Set where to check for active CPU scaling governor
                 *
                 * Running benchmarks on a system with dynamic CPU scaling
                 * makes the measurements very noisy. If that's detected, a
                 * warning is printed in the output. Defaults to
                 * `/sys/devices/system/cpu/cpu{}/cpufreq/scaling_governor`,
                 * where `{}` is replaced with CPU ID; if the file doesn't
                 * exist, no check is done.
                 *
                 * A view that has both @ref Containers::StringViewFlag::Global
                 * and @relativeref{Containers::StringViewFlag,NullTerminated}
                 * set (such as coming from a @ref Containers::StringView
                 * literal) will be used without having to make an owned string
                 * copy internally.
                 * @partialsupport Available only on Linux.
                 */
                TesterConfiguration& setCpuScalingGovernorFile(Containers::StringView filename);
                /* This one doesn't have a const char* overload as it's
                   unlikely to be used except for the Tester test itself */
                #endif

            private:
                friend Tester;

                /* Don't want to include an Array here because we don't need
                   it in any public APIs */
                struct Data;
                Containers::Pointer<Data> _data;
        };

        /**
         * @brief Alias for debug output
         *
         * For convenient debug output inside test cases (instead of using
         * the fully qualified name):
         *
         * @snippet TestSuite.cpp Tester-Debug
         *
         * @see @ref Warning, @ref Error
         */
        typedef Corrade::Utility::Debug Debug;

        /**
         * @brief Alias for warning output
         *
         * See @ref Debug for more information.
         */
        typedef Corrade::Utility::Warning Warning;

        /**
         * @brief Alias for error output
         *
         * See @ref Debug for more information.
         */
        typedef Corrade::Utility::Error Error;

        /**
         * @brief Benchmark type
         *
         * @see @ref TestSuite-Tester-benchmark, @ref addBenchmarks(),
         *      @ref addInstancedBenchmarks()
         */
        enum class BenchmarkType {
            /* 0 reserved for test cases */

            /**
             * Default. Equivalent to @ref BenchmarkType::WallTime, but can be
             * overridden on command-line using the `--benchmark` option.
             */
            Default = 1,

            /**
             * Wall time. Suitable for measuring events in microseconds and up.
             * While the reported time is in nanoseconds, the actual timer
             * granularity may differ from platform to platform. To measure
             * shorter events, increase number of iterations passed to
             * @ref CORRADE_BENCHMARK() to amortize the error or use a
             * different benchmark type.
             */
            WallTime = 2,

            /**
             * CPU time. Suitable for measuring most events (microseconds and
             * up). While the reported time is in nanoseconds, the actual timer
             * granularity may differ from platform to platform (for example on
             * Windows the CPU clock is reported in multiples of 100 ns). To
             * measure shorter events, increase number of iterations passed to
             * @ref CORRADE_BENCHMARK() to amortize the error or use a
             * different clock.
             * @partialsupport On @ref CORRADE_TARGET_WINDOWS_RT "Windows RT"
             *      gives zero result.
             */
            CpuTime = 3,

            /**
             * CPU cycle count. Suitable for measuring sub-millisecond events,
             * but note that on newer architectures the cycle counter frequency
             * is constant and thus measured value is independent on CPU
             * frequency, so it in fact measures time and not the actual cycles
             * spent. See for example
             * https://randomascii.wordpress.com/2011/07/29/rdtsc-in-the-age-of-sandybridge/
             * for more information.
             * @partialsupport Supported only on @ref CORRADE_TARGET_X86 "x86"
             *      and GCC/Clang or MSVC (using RDTSC), on other platforms
             *      gives zero result.
             */
            CpuCycles = 4
        };

        /**
         * @brief Custom benchmark units
         *
         * Unit of measurements outputted from custom benchmarks.
         * @see @ref TestSuite-Tester-benchmark-custom,
         *      @ref addCustomBenchmarks(), @ref addCustomInstancedBenchmarks()
         */
        enum class BenchmarkUnits {
            /* Values should not overlap with BenchmarkType. When adding more,
               be sure to expand the internal TestCaseType enum as well. */

            /**
             * Time in nanoseconds. Depending on the magnitude, the value is
             * shown as `ns`, `µs`, `ms` and `s`.
             */
            Nanoseconds = 100,

            /**
             * Processor cycle count. Depending on the magnitude, the value is
             * shown as `C`, `kC`, `MC` and `GC` (with a multiplier of 1000).
             */
            Cycles = 101,

            /**
             * Processor instruction count. Depending on the magnitude, the
             * value is shown as `I`, `kI`, `MI` and `GI` (with a multiplier of
             * 1000).
             */
            Instructions = 102,

            /**
             * Memory (in bytes). Depending on the magnitude, the value is
             * shown as `B`, `kB`, `MB` and `GB` (with a multiplier of 1024).
             */
            Bytes = 103,

            /**
             * Generic count. Depending on the magnitude, the value is shown
             * with no suffix or with `k`, `M` or `G` (with a multiplier of
             * 1000).
             */
            Count = 104,

            /**
             * Ratio expressed in 1/1000s. The value is shown divided by 1000
             * and depending on the magnitude it's shown with no suffix or with
             * `k`, `M` or `G` (with a multiplier of 1000).
             * @m_since_latest
             */
            RatioThousandths = 105,

            /**
             * Percentage expressed in 1/1000s. The value is shown divided by
             * 1000 and with a `%` suffix. In the unfortunate scenario where
             * the magnitude reaches 1000 and more, it's shown with `k`, `M` or
             * `G` (with a multiplier of 1000).
             * @m_since_latest
             */
            PercentageThousandths = 106
        };

        /**
         * @brief Constructor
         * @param configuration     Optional configuration
         */
        explicit Tester(const TesterConfiguration& configuration = TesterConfiguration{});

        /**
         * @brief Command-line arguments
         *
         * Populated by @ref CORRADE_TEST_MAIN(). Note that the argument value
         * is usually immutable (thus @cpp const char* const * @ce), it's
         * however exposed as just @cpp char** @ce to make passing to 3rd party
         * APIs easier.
         */
        Containers::Pair<Containers::Reference<int>, char**> arguments();

        /**
         * @brief Add test cases
         *
         * Adds one or more test cases to be executed. It's not an error to
         * call this function multiple times or add one test case more than
         * once.
         * @see @ref TestSuite-Tester-basic, @ref addInstancedTests()
         */
        template<class Derived> void addTests(std::initializer_list<void(Derived::*)()> tests) {
            addRepeatedTests<Derived>(tests, 1);
        }

        /**
         * @brief Add repeated test cases
         *
         * Unlike the above function repeats each of the test cases until it
         * fails or @p repeatCount is reached. Useful for stability or resource
         * leak checking. Each test case appears in the output log only once.
         * It's not an error to call this function multiple times or add a
         * particular test case more than once --- in that case it will appear
         * in the output log once for each occurrence in the list.
         * @see @ref TestSuite-Tester-repeated, @ref addInstancedTests(),
         *      @ref addRepeatedInstancedTests()
         */
        template<class Derived> void addRepeatedTests(std::initializer_list<void(Derived::*)()> tests, std::size_t repeatCount) {
            addRepeatedTests<Derived>(tests, repeatCount, nullptr, nullptr);
        }

        /**
         * @brief Add test cases with explicit setup and teardown functions
         * @param tests         List of test cases to run
         * @param setup         Setup function
         * @param teardown      Teardown function
         *
         * In addition to the behavior of @ref addTests() above, the @p setup
         * function is called before every test case in the list and the
         * @p teardown function is called after every test case in the list,
         * regardless of whether it passed, failed or was skipped. Using
         * verification macros in @p setup or @p teardown function is not
         * allowed. It's not an error to call this function multiple times or
         * add one test case more than once.
         * @see @ref TestSuite-Tester-basic,
         *      @ref TestSuite-Tester-setup-teardown, @ref addInstancedTests()
         */
        template<class Derived> void addTests(std::initializer_list<void(Derived::*)()> tests, void(Derived::*setup)(), void(Derived::*teardown)()) {
            addRepeatedTests<Derived>(tests, 1, setup, teardown);
        }

        /**
         * @brief Add repeated test cases with explicit setup and teardown functions
         *
         * Unlike the above function repeats each of the test cases until it
         * fails or @p repeatCount is reached. Useful for stability or resource
         * leak checking. The @p setup and @p teardown functions are called
         * again for each repeat of each test case. Each test case appears in
         * the output log only once. It's not an error to call this function
         * multiple times or add a particular test case more than once --- in
         * that case it will appear in the output log once for each occurrence
         * in the list.
         * @see @ref TestSuite-Tester-repeated,
         *      @ref TestSuite-Tester-setup-teardown, @ref addInstancedTests(),
         *      @ref addRepeatedInstancedTests()
         */
        template<class Derived> void addRepeatedTests(std::initializer_list<void(Derived::*)()> tests, std::size_t repeatCount, void(Derived::*setup)(), void(Derived::*teardown)()) {
            for(auto test: tests)
                addTestCaseInternal({~std::size_t{}, repeatCount, static_cast<TestCase::Function>(test), static_cast<TestCase::Function>(setup), static_cast<TestCase::Function>(teardown)});
        }

        /**
         * @brief Add instanced test cases
         *
         * Unlike @ref addTests(), this function runs each of the test cases
         * @p instanceCount times. Useful for data-driven tests. Each test case
         * appears in the output once for each instance. It's not an error to
         * call this function multiple times or add one test case more than
         * once --- in that case it will appear once for each instance of each
         * occurrence in the list.
         * @see @ref TestSuite-Tester-instanced, @ref testCaseInstanceId(),
         *      @ref setTestCaseDescription()
         */
        template<class Derived> void addInstancedTests(std::initializer_list<void(Derived::*)()> tests, std::size_t instanceCount) {
            addRepeatedInstancedTests<Derived>(tests, 1, instanceCount);
        }

        /**
         * @brief Add repeated instanced test cases
         *
         * Unlike the above function repeats each of the test case instances
         * until it fails or @p repeatCount is reached. Useful for stability or
         * resource leak checking. Each test case appears in the output once
         * for each instance. It's not an error to call this function multiple
         * times or add one test case more than once --- in that case it will
         * appear once for each instance of each occurrence in the list.
         * @see @ref TestSuite-Tester-repeated,
         *      @ref TestSuite-Tester-instanced, @ref addInstancedTests(),
         *      @ref addRepeatedInstancedTests()
         */
        template<class Derived> void addRepeatedInstancedTests(std::initializer_list<void(Derived::*)()> tests, std::size_t repeatCount, std::size_t instanceCount) {
            addRepeatedInstancedTests<Derived>(tests, repeatCount, instanceCount, nullptr, nullptr);
        }

        /**
         * @brief Add instanced test cases with explicit setup and teardown functions
         * @param tests         List of test cases to run
         * @param instanceCount Instance count
         * @param setup         Setup function
         * @param teardown      Teardown function
         *
         * In addition to the behavior of @ref addInstancedTests() above, the
         * @p setup function is called before every instance of every test case
         * in the list and the @p teardown function is called after every
         * instance of every test case in the list, regardless of whether it
         * passed, failed or was skipped. Using verification macros in @p setup
         * or @p teardown function is not allowed. It's not an error to call
         * this function multiple times or add one test case more than once ---
         * in that case it will appear once for each instance of each occurrence
         * in the list.
         * @see @ref TestSuite-Tester-instanced,
         *      @ref TestSuite-Tester-setup-teardown
         */
        template<class Derived> void addInstancedTests(std::initializer_list<void(Derived::*)()> tests, std::size_t instanceCount, void(Derived::*setup)(), void(Derived::*teardown)()) {
            addRepeatedInstancedTests<Derived>(tests, 1, instanceCount, setup, teardown);
        }

        /**
         * @brief Add repeated instanced test cases with explicit setup and teardown functions
         *
         * Unlike the above function repeats each of the test case instances
         * until it fails or @p repeatCount is reached. Useful for stability or
         * resource leak checking. The @p setup and @p teardown functions are
         * called again for each repeat of each instance of each test case. The
         * test case appears in the output once for each instance. It's not an
         * error to call this function multiple times or add one test case more
         * than once --- in that case it will appear once for each instance of
         * each occurrence in the list.
         * @see @ref TestSuite-Tester-repeated,
         *      @ref TestSuite-Tester-instanced,
         *      @ref TestSuite-Tester-setup-teardown, @ref addInstancedTests(),
         *      @ref addRepeatedInstancedTests()
         */
        template<class Derived> void addRepeatedInstancedTests(std::initializer_list<void(Derived::*)()> tests, std::size_t repeatCount, std::size_t instanceCount, void(Derived::*setup)(), void(Derived::*teardown)()) {
            for(auto test: tests) for(std::size_t i = 0; i != instanceCount; ++i)
                addTestCaseInternal({i, repeatCount, static_cast<TestCase::Function>(test), static_cast<TestCase::Function>(setup), static_cast<TestCase::Function>(teardown)});
        }

        /**
         * @brief Add benchmarks
         * @param benchmarks        List of benchmarks to run
         * @param batchCount        Batch count
         * @param benchmarkType     Benchmark type
         *
         * For each added benchmark measures the time spent executing code
         * inside a statement or block denoted by @ref CORRADE_BENCHMARK(). It
         * is possible to use all verification macros inside the benchmark. The
         * @p batchCount parameter specifies how many batches will be run to
         * make the measurement more precise, while the batch size parameter
         * passed to @ref CORRADE_BENCHMARK() specifies how many iterations
         * will be done in each batch to minimize overhead. It's not an error
         * to call this function multiple times or add one benchmark more than
         * once.
         * @see @ref TestSuite-Tester-benchmark, @ref addInstancedBenchmarks()
         */
        template<class Derived> void addBenchmarks(std::initializer_list<void(Derived::*)()> benchmarks, std::size_t batchCount, BenchmarkType benchmarkType = BenchmarkType::Default) {
            addBenchmarks<Derived>(benchmarks, batchCount, nullptr, nullptr, benchmarkType);
        }

        /**
         * @brief Add benchmarks with explicit setup and teardown functions
         * @param benchmarks        List of benchmarks to run
         * @param batchCount        Batch count
         * @param setup             Setup function
         * @param teardown          Teardown function
         * @param benchmarkType     Benchmark type
         *
         * In addition to the behavior of @ref addBenchmarks() above, the
         * @p setup function is called before every batch of every benchmark in
         * the list and the @p teardown function is called after every batch of
         * every benchmark in the list, regardless of whether it passed, failed
         * or was skipped. Using verification macros in @p setup or @p teardown
         * function is not allowed. It's not an error to call this function
         * multiple times or add one benchmark more than once.
         * @see @ref TestSuite-Tester-benchmark,
         *      @ref TestSuite-Tester-setup-teardown,
         *      @ref addInstancedBenchmarks()
         */
        template<class Derived> void addBenchmarks(std::initializer_list<void(Derived::*)()> benchmarks, std::size_t batchCount, void(Derived::*setup)(), void(Derived::*teardown)(), BenchmarkType benchmarkType = BenchmarkType::Default) {
            addCustomBenchmarks<Derived>(benchmarks, batchCount, setup, teardown, nullptr, nullptr, BenchmarkUnits(int(benchmarkType)));
        }

        /**
         * @brief Add custom benchmarks
         * @param benchmarks        List of benchmarks to run
         * @param batchCount        Batch count
         * @param benchmarkBegin    Benchmark begin function
         * @param benchmarkEnd      Benchmark end function
         * @param benchmarkUnits    Benchmark units
         *
         * Unlike the above functions uses user-supplied measurement functions.
         * The @p benchmarkBegin parameter starts the measurement, the
         * @p benchmarkEnd parameter ends the measurement and returns measured
         * value, which is in @p units. It's not an error to call this function
         * multiple times or add one benchmark more than once.
         * @see @ref TestSuite-Tester-benchmark-custom,
         *      @ref addCustomInstancedBenchmarks()
         */
        template<class Derived> void addCustomBenchmarks(std::initializer_list<void(Derived::*)()> benchmarks, std::size_t batchCount, void(Derived::*benchmarkBegin)(), std::uint64_t(Derived::*benchmarkEnd)(), BenchmarkUnits benchmarkUnits) {
            addCustomBenchmarks<Derived>(benchmarks, batchCount, nullptr, nullptr, static_cast<TestCase::BenchmarkBegin>(benchmarkBegin), static_cast<TestCase::BenchmarkEnd>(benchmarkEnd), benchmarkUnits);
            /* "warning: parameter ‘benchmarkBegin’ set but not used", is that
               some GCC 13 regression? I thought such silly warnings where a
               static_cast made a variable look like being unused were bugs
               from the GCC 4.8 and MSVC 2015 era. Happens on GCC 14 *and* 15
               as well, but I still hope version 16 fixes it. */
            #if defined(CORRADE_TARGET_GCC) && !defined(CORRADE_TARGET_CLANG) && __GNUC__ >= 13 && __GNUC__ < 16
            static_cast<void>(benchmarkBegin);
            static_cast<void>(benchmarkEnd);
            #endif
        }

        /**
         * @brief Add custom benchmarks with explicit setup and teardown functions
         * @param benchmarks        List of benchmarks to run
         * @param batchCount        Batch count
         * @param setup             Setup function
         * @param teardown          Teardown function
         * @param benchmarkBegin    Benchmark begin function
         * @param benchmarkEnd      Benchmark end function
         * @param benchmarkUnits    Benchmark units
         *
         * In addition to the behavior of @ref addCustomBenchmarks() above, the
         * @p setup function is called before every batch of every benchmark in
         * the list and the @p teardown function is called after every batch of
         * every benchmark in the list, regardless of whether it passed, failed
         * or was skipped. Using verification macros in @p setup or @p teardown
         * function is not allowed. It's not an error to call this function
         * multiple times or add one benchmark more than once.
         * @see @ref TestSuite-Tester-benchmark-custom,
         *      @ref TestSuite-Tester-setup-teardown,
         *      @ref addCustomInstancedBenchmarks()
         */
        template<class Derived> void addCustomBenchmarks(std::initializer_list<void(Derived::*)()> benchmarks, std::size_t batchCount, void(Derived::*setup)(), void(Derived::*teardown)(), void(Derived::*benchmarkBegin)(), std::uint64_t(Derived::*benchmarkEnd)(), BenchmarkUnits benchmarkUnits) {
            for(auto benchmark: benchmarks)
                addTestCaseInternal({~std::size_t{}, batchCount, static_cast<TestCase::Function>(benchmark), static_cast<TestCase::Function>(setup), static_cast<TestCase::Function>(teardown), static_cast<TestCase::BenchmarkBegin>(benchmarkBegin), static_cast<TestCase::BenchmarkEnd>(benchmarkEnd), TestCaseType(int(benchmarkUnits))});
        }

        /**
         * @brief Add instanced benchmarks
         * @param benchmarks        List of benchmarks to run
         * @param batchCount        Batch count
         * @param instanceCount     Instance count
         * @param benchmarkType     Benchmark type
         *
         * Unlike @ref addBenchmarks(), this function runs each of the
         * benchmarks @p instanceCount times. Useful for data-driven tests.
         * Each test case appears in the output once for each instance. It's
         * not an error to call this function multiple times or add one
         * benchmark more than once --- in that case it will appear once for
         * each instance of each occurrence in the list.
         * @see @ref TestSuite-Tester-benchmark,
         *      @ref TestSuite-Tester-instanced, @ref testCaseInstanceId(),
         *      @ref setTestCaseDescription()
         */
        template<class Derived> void addInstancedBenchmarks(std::initializer_list<void(Derived::*)()> benchmarks, std::size_t batchCount, std::size_t instanceCount, BenchmarkType benchmarkType = BenchmarkType::Default) {
            addInstancedBenchmarks<Derived>(benchmarks, batchCount, instanceCount, nullptr, nullptr, benchmarkType);
        }

        /**
         * @brief Add instanced benchmarks with explicit setup and teardown functions
         * @param benchmarks        List of benchmarks to run
         * @param batchCount        Batch count
         * @param instanceCount     Instance count
         * @param setup             Setup function
         * @param teardown          Teardown function
         * @param benchmarkType     Benchmark type
         *
         * In addition to the behavior of above function, the @p setup function
         * is called before every instance of every batch of every benchmark in
         * the list and the @p teardown function is called after every instance
         * of every batch of every benchmark in the list, regardless of whether
         * it passed, failed or was skipped. Using verification macros in
         * @p setup or @p teardown function is not allowed. It's not an error
         * to call this function multiple times or add one benchmark more than
         * once --- in that case it will appear once for each instance of each
         * occurrence in the list.
         * @see @ref TestSuite-Tester-benchmark,
         *      @ref TestSuite-Tester-instanced,
         *      @ref TestSuite-Tester-setup-teardown
         */
        template<class Derived> void addInstancedBenchmarks(std::initializer_list<void(Derived::*)()> benchmarks, std::size_t batchCount, std::size_t instanceCount, void(Derived::*setup)(), void(Derived::*teardown)(), BenchmarkType benchmarkType = BenchmarkType::Default) {
            addCustomInstancedBenchmarks<Derived>(benchmarks, batchCount, instanceCount, setup, teardown, nullptr, nullptr, BenchmarkUnits(int(benchmarkType)));
        }

        /**
         * @brief Add custom instanced benchmarks
         * @param benchmarks        List of benchmarks to run
         * @param batchCount        Batch count
         * @param instanceCount     Instance count
         * @param benchmarkBegin    Benchmark begin function
         * @param benchmarkEnd      Benchmark end function
         * @param benchmarkUnits    Benchmark units
         *
         * Unlike the above functions uses user-supplied measurement functions.
         * The @p benchmarkBegin parameter starts the measurement, the
         * @p benchmarkEnd parameter ends the measurement and returns measured
         * value, which is in @p units. It's not an error to call this function
         * multiple times or add one benchmark more than once --- in that case
         * it will appear once for each instance of each occurrence in the list.
         * @see @ref TestSuite-Tester-benchmark-custom,
         *      @ref TestSuite-Tester-instanced
         */
        template<class Derived> void addCustomInstancedBenchmarks(std::initializer_list<void(Derived::*)()> benchmarks, std::size_t batchCount, std::size_t instanceCount, void(Derived::*benchmarkBegin)(), std::uint64_t(Derived::*benchmarkEnd)(), BenchmarkUnits benchmarkUnits) {
            addCustomInstancedBenchmarks<Derived>(benchmarks, batchCount, instanceCount, nullptr, nullptr, benchmarkBegin, benchmarkEnd, benchmarkUnits);
        }

        /**
         * @brief Add custom instanced benchmarks with explicit setup and teardown functions
         * @param benchmarks        List of benchmarks to run
         * @param batchCount        Batch count
         * @param instanceCount     Batch count
         * @param setup             Setup function
         * @param teardown          Teardown function
         * @param benchmarkBegin    Benchmark begin function
         * @param benchmarkEnd      Benchmark end function
         * @param benchmarkUnits    Benchmark units
         *
         * In addition to the behavior of @ref addCustomBenchmarks() above, the
         * @p setup function is called before every batch of every benchmark in
         * the list and the @p teardown function is called after every batch of
         * every benchmark in the list, regardless of whether it passed, failed
         * or was skipped. Using verification macros in @p setup or @p teardown
         * function is not allowed. It's not an error to call this function
         * multiple times or add one benchmark more than once --- in that case
         * it will appear once for each instance of each occurrence in the list.
         * @see @ref TestSuite-Tester-benchmark-custom,
         *      @ref TestSuite-Tester-instanced,
         *      @ref TestSuite-Tester-setup-teardown
         */
        template<class Derived> void addCustomInstancedBenchmarks(std::initializer_list<void(Derived::*)()> benchmarks, std::size_t batchCount, std::size_t instanceCount, void(Derived::*setup)(), void(Derived::*teardown)(), void(Derived::*benchmarkBegin)(), std::uint64_t(Derived::*benchmarkEnd)(), BenchmarkUnits benchmarkUnits) {
            for(auto benchmark: benchmarks) for(std::size_t i = 0; i != instanceCount; ++i)
                addTestCaseInternal({i, batchCount, static_cast<TestCase::Function>(benchmark), static_cast<TestCase::Function>(setup), static_cast<TestCase::Function>(teardown), static_cast<TestCase::BenchmarkBegin>(benchmarkBegin), static_cast<TestCase::BenchmarkEnd>(benchmarkEnd), TestCaseType(int(benchmarkUnits))});
        }

        /**
         * @brief Test case ID
         *
         * Returns ID of the test case that is currently executing, starting
         * from `1`.  Expects that this function is called from within a test
         * case or its corresponding setup/teardown function.
         */
        std::size_t testCaseId() const;

        /**
         * @brief Test case instance ID
         *
         * Returns instance ID of the instanced test case that is currently
         * executing, starting from `0`. Expects that this function is called
         * from within an instanced test case or its corresponding
         * setup/teardown function.
         * @see @ref addInstancedTests()
         */
        std::size_t testCaseInstanceId() const;

        /**
         * @brief Test case repeat ID
         *
         * Returns repeat ID of the repeated test case that is currently
         * executing, starting from `0`. Expects that this function is called
         * from within a repeated test case or its corresponding setup/teardown
         * function.
         * @see @ref addRepeatedTests()
         */
        std::size_t testCaseRepeatId() const;

        /**
         * @brief Test name
         * @m_since_latest
         */
        Containers::StringView testName() const;

        /**
         * @brief Set custom test name
         *
         * By default the test name is gathered together with test filename by
         * the @ref CORRADE_TEST_MAIN() macro and is equivalent to
         * fully-qualified class name.
         *
         * A view that has both @ref Containers::StringViewFlag::Global and
         * @relativeref{Containers::StringViewFlag,NullTerminated} set (such as
         * coming from a @ref Containers::StringView literal) will be used
         * without having to make an owned string copy internally.
         * @see @ref setTestCaseName(), @ref setTestCaseTemplateName(),
         *      @ref setTestCaseDescription()
         */
        void setTestName(Containers::StringView name);
        /* So people aren't forced to include StringView if they don't want */
        void setTestName(const char* name); /**< @overload */

        /**
         * @brief Test case name
         * @m_since_latest
         */
        Containers::StringView testCaseName() const;

        /**
         * @brief Set custom test case name
         *
         * By default the test case name is gathered in the check macros and is
         * equivalent to the following:
         *
         * @snippet TestSuite.cpp Tester-setTestCaseName
         *
         * A view that has both @ref Containers::StringViewFlag::Global and
         * @relativeref{Containers::StringViewFlag,NullTerminated} set (such as
         * coming from a @ref Containers::StringView literal) will be used
         * without having to make an owned string copy internally.
         * @see @ref setTestCaseTemplateName(), @ref setTestName(),
         *      @ref setTestCaseDescription(), @ref CORRADE_FUNCTION
         */
        void setTestCaseName(Containers::StringView name);
        /* So people aren't forced to include StringView if they don't want */
        void setTestCaseName(const char* name); /**< @overload */

        /**
         * @brief Test case template name
         * @m_since_latest
         */
        Containers::StringView testCaseTemplateName() const;

        /**
         * @brief Set test case template name
         * @m_since{2019,10}
         *
         * Useful to distinguish different specializations of the same templated
         * test case. Equivalent to the following called from inside the test
         * case:
         *
         * @snippet TestSuite.cpp Tester-setTestCaseTemplateName
         *
         * A view that has both @ref Containers::StringViewFlag::Global and
         * @relativeref{Containers::StringViewFlag,NullTerminated} set (such as
         * coming from a @ref Containers::StringView literal) will be used
         * without having to make an owned string copy internally.
         * @see @ref setTestCaseName(), @ref setTestName(),
         *      @ref setTestCaseDescription(), @ref CORRADE_FUNCTION
         */
        void setTestCaseTemplateName(Containers::StringView name);
        /* So people aren't forced to include StringView if they don't want */
        void setTestCaseTemplateName(const char* name); /**< @overload */

        /**
         * @overload
         * @m_since_latest
         *
         * Useful for test cases that are templated with more than one
         * parameter. Names are joined with `,`.
         *
         * Unlike with @ref setTestCaseTemplateName(Containers::StringView), a
         * new string for the joined result is created always so presence of
         * any @ref Containers::StringViewFlags in passed views doesn't matter.
         */
        void setTestCaseTemplateName(Containers::ArrayView<const Containers::StringView> names);
        void setTestCaseTemplateName(std::initializer_list<Containers::StringView> names); /**< @overload */
        /* So people aren't forced to include StringView if they don't want */
        void setTestCaseTemplateName(Containers::ArrayView<const char* const> names); /**< @overload */
        void setTestCaseTemplateName(std::initializer_list<const char*> names); /**< @overload */

        /**
         * @brief Test case description
         * @m_since_latest
         */
        Containers::StringView testCaseDescription() const;

        /**
         * @brief Set test case description
         *
         * Additional text displayed after the test case name. By default
         * the description is empty for non-instanced test cases and instance
         * ID for instanced test cases. If you use
         * @ref setTestCaseDescription(const TestCaseDescriptionSourceLocation&)
         * instead, output messages will contain also the file/line where the
         * instanced test case data were defined. See the
         * @ref TestCaseDescriptionSourceLocation class documentation for an
         * example.
         *
         * A view that has both @ref Containers::StringViewFlag::Global and
         * @relativeref{Containers::StringViewFlag,NullTerminated} set (such as
         * coming from a @ref Containers::StringView literal) will be used
         * without having to make an owned string copy internally.
         * @see @ref setTestName(), @ref setTestCaseName(),
         *      @ref setTestCaseTemplateName()
         */
        void setTestCaseDescription(Containers::StringView description);
        /* So people aren't forced to include StringView if they don't want */
        void setTestCaseDescription(const char* description); /**< @overload */

        /**
         * @brief Set test case description with source location
         * @m_since_latest
         *
         * Compared to @ref setTestCaseDescription(Containers::StringView),
         * output messages printed for the test case will contain also the
         * file/line where the instanced test case data were defined. See the
         * @ref TestCaseDescriptionSourceLocation class documentation for an
         * example.
         */
        void setTestCaseDescription(const TestCaseDescriptionSourceLocation& description);

        /**
         * @brief Test case description
         * @m_since_latest
         */
        Containers::StringView benchmarkName() const;

        /**
         * @brief Set benchmark name
         *
         * In case of @ref addCustomBenchmarks() and @ref addCustomInstancedBenchmarks()
         * provides the name for the unit measured, for example @cpp "wall time" @ce.
         *
         * A view that has both @ref Containers::StringViewFlag::Global and
         * @relativeref{Containers::StringViewFlag,NullTerminated} set (such as
         * coming from a @ref Containers::StringView literal) will be used
         * without having to make an owned string copy internally.
         */
        void setBenchmarkName(Containers::StringView name);
        /* So people aren't forced to include StringView if they don't want */
        void setBenchmarkName(const char* name); /**< @overload */

    protected:
        ~Tester();

    #ifdef DOXYGEN_GENERATING_OUTPUT
    private:
    #else
    public:
    #endif
        class Printer;

        /* Called from CORRADE_TEST_MAIN(). argc is grabbed via a mutable
           reference and argv is grabbed as non-const in order to allow the
           users modifying the argument list (and GLUT requires that) */
        static void registerArguments(int& argc, const char* const* argv);

        /* Called from all CORRADE_*() macros */
        static Tester& instance();

        /* Called from CORRADE_TEST_MAIN() */
        int exec();

        /* Overload needed for testing */
        int exec(Tester* previousTester, std::ostream* logOutput, std::ostream* errorOutput);

        /* Compare two identical types without explicit type specification */
        template<class T> void compare(const char* actual, const T& actualValue, const char* expected, const T& expectedValue) {
            compareAs<T, T, T>(actual, actualValue, expected, expectedValue);
        }

        /* Compare two different types without explicit type specification */
        template<class Actual, class Expected> void compare(const char* actual, const Actual& actualValue, const char* expected, const Expected& expectedValue) {
            compareAs<typename Implementation::CommonType<Actual, Expected>::Type, Actual, Expected>(actual, actualValue, expected, expectedValue);
        }

        /* Compare two different types with explicit templated type
           specification (e.g. Compare::Containers). This allows the user to
           call only `CORRADE_COMPARE_AS(a, b, Compare::Containers)` without
           explicitly specifying the type, e.g.
           `CORRADE_COMPARE_AS(a, b, Compare::Containers<std::vector<int>>)` */
        template<template<class> class T, class Actual, class Expected> void compareAs(const char* actual, const Actual& actualValue, const char* expected, const Expected& expectedValue) {
            compareAs<T<typename Implementation::CommonType<Actual, Expected>::Type>, Actual, Expected>(actual, actualValue, expected, expectedValue);
        }

        /* Compare two different types with explicit type specification */
        template<class T, class U, class V> void compareAs(const char* actual, const U& actualValue, const char* expected, const V& expectedValue) {
            compareWith(Comparator<T>(), actual, actualValue, expected, expectedValue);
        }

        /* Compare two different types with explicit comparator specification */
        template<class T, class U, class V> void compareWith(Comparator<T>& comparator, const char* actual, const U& actualValue, const char* expected, const V& expectedValue);
        template<class T, class U, class V> void compareWith(Comparator<T>&& comparator, const char* actual, const U& actualValue, const char* expected, const V& expectedValue) {
            return compareWith<T, U, V>(comparator, actual, actualValue, expected, expectedValue);
        }

        template<class T> void verify(const char* expression, T&& value);

        /* Called from CORRADE_TEST_MAIN(). The filename is __FILE__ and name
           is a stringified class name, thus they're assumed to be global. */
        void registerTest(const char* filename, const char* name, bool isDebugBuild = false);

        /* Called from CORRADE_SKIP() */
        [[noreturn]] void skip(const Printer& printer);

        /* Called from CORRADE_INFO(), CORRADE_WARN(). The line is passed this
           way and not through registerTestCase() as there it's used to detect
           if any checks were made (failing the test if not) and these two
           macros don't actually check anything. */
        void infoOrWarn(const Printer& printer, std::size_t line, bool warn);
        /* Called from CORRADE_FAIL_IF() */
        void failIf(const Printer& printer, bool fail);

        /* For types with explicit bool conversion */
        template<class T> void failIf(const Printer& printer, T&& fail) {
            failIf(printer, fail ? true : false);
        }

        class CORRADE_TESTSUITE_EXPORT Printer {
            public:
                /* Used as implicit from Magnum's OpenGLTester, watch out */
                template<class F> /*implicit*/ Printer(F&& printer): Printer{} {
                    printer(debug());
                }

                ~Printer();

            private:
                friend Tester;

                Printer();
                Debug debug();

                /* There's a std::ostringstream inside (yes, ew); don't want
                   that in a header */
                struct Data;
                Containers::Pointer<Data> _data;
        };

        class CORRADE_TESTSUITE_EXPORT ExpectedFailure: public Printer {
            public:
                template<class F> explicit ExpectedFailure(F&& printer, bool enabled = true): ExpectedFailure{enabled} {
                    printer(debug());
                }

                /* For types with explicit bool conversion */
                template<class F, class T> explicit ExpectedFailure(F&& printer, T&& enabled): ExpectedFailure{enabled ? true : false} {
                    printer(debug());
                }

                ~ExpectedFailure();

            private:
                explicit ExpectedFailure(bool enabled);
        };

        class CORRADE_TESTSUITE_EXPORT IterationPrinter: public Printer {
            public:
                template<class F> IterationPrinter(F&& printer): IterationPrinter{} {
                    printer(debug());
                }

                ~IterationPrinter();

            private:
                friend Tester;

                IterationPrinter();

                IterationPrinter* _parent;
        };

        /* Called from all CORRADE_*() verification/skip/... macros. The
           variant without line info is for macros that shouldn't count as
           checks (such as CORRADE_ITERATION()) and thus if a test case
           contains only those, it should be reported as an error.

           The name is CORRADE_FUNCTION and thus assumed to be global. */
        void registerTestCase(const char* name);
        void registerTestCase(const char* name, int line);

    private:
        class Exception {};
        class SkipException {};

        enum class TestCaseType {
            Test = 0,
            DefaultBenchmark = int(BenchmarkType::Default),
            WallTimeBenchmark = int(BenchmarkType::WallTime),
            CpuTimeBenchmark = int(BenchmarkType::CpuTime),
            CpuCyclesBenchmark = int(BenchmarkType::CpuCycles),
            CustomTimeBenchmark = int(BenchmarkUnits::Nanoseconds),
            CustomCycleBenchmark = int(BenchmarkUnits::Cycles),
            CustomInstructionBenchmark = int(BenchmarkUnits::Instructions),
            CustomMemoryBenchmark = int(BenchmarkUnits::Bytes),
            CustomCountBenchmark = int(BenchmarkUnits::Count),
            CustomRatioThousandthsBenchmark = int(BenchmarkUnits::RatioThousandths),
            CustomPercentageThousandthsBenchmark = int(BenchmarkUnits::PercentageThousandths),
        };

        struct TestCase {
            typedef void (Tester::*Function)();
            typedef void (Tester::*BenchmarkBegin)();
            typedef std::uint64_t (Tester::*BenchmarkEnd)();

            /*implicit*/ TestCase(std::size_t instanceId, std::size_t repeatCount, Function test, Function setup, Function teardown): instanceId{instanceId}, repeatCount{repeatCount}, test{test}, setup{setup}, teardown{teardown}, benchmarkBegin{}, benchmarkEnd{}, type{TestCaseType::Test} {}

            /*implicit*/ TestCase(std::size_t instanceId, std::size_t repeatCount, Function test, Function setup, Function teardown, BenchmarkBegin benchmarkBegin, BenchmarkEnd benchmarkEnd, TestCaseType type): instanceId{instanceId}, repeatCount{repeatCount}, test{test}, setup{setup}, teardown{teardown}, benchmarkBegin{benchmarkBegin}, benchmarkEnd{benchmarkEnd}, type{type} {}

            std::size_t instanceId, repeatCount;
            Function test, setup, teardown;
            BenchmarkBegin benchmarkBegin;
            BenchmarkEnd benchmarkEnd;
            TestCaseType type;
        };

    #ifndef DOXYGEN_GENERATING_OUTPUT
    public:
    #endif
        class CORRADE_TESTSUITE_EXPORT BenchmarkRunner {
            public:
                explicit BenchmarkRunner(TestCase::BenchmarkBegin begin, TestCase::BenchmarkEnd end): _instance(Tester::instance()), _end{end} {
                    (_instance.*begin)();
                }

                ~BenchmarkRunner();

                /* The end() has to be deinlined because it accesses the
                   privately defined Tester::_state; the begin(), which returns
                   nullptr, has to be deinlined because GCC 15 in optimized
                   builds compiles away the null pointer arithmetic, causing it
                   to iterate forever. Very useful optimization, thank you.

                   Those being non-inline shouldn't be a problem as the
                   range-for loop in CORRADE_BENCHMARK() should cache them both
                   internally. */
                const char* begin() const;
                const char* end() const;

            private:
                /* Caching the instance here to avoid potentially slow global
                   variable access */
                Tester& _instance;
                TestCase::BenchmarkEnd _end;
        };

        /* Called from CORRADE_BENCHMARK() */
        BenchmarkRunner createBenchmarkRunner(std::size_t batchSize);

    private:
        /* Need to preserve as much unique name as possible here, just `State`
           would cause conflicts with user-defined State types */
        struct TesterState;

        static int* _argc;
        static const char* const* _argv;

        CORRADE_TESTSUITE_LOCAL void printTestCaseLabel(Debug& out, const char* status, Debug::Color statusColor, Debug::Color labelColor);
        CORRADE_TESTSUITE_LOCAL void printFileLineInfo(Debug& out);
        /* Used from CORRADE_INFO() / CORRADE_WARN() which don't count as
           checks and thus don't record line info (which is then used to detect
           whether any checks were made, yeah I know, abusing irrelevant state
           for this) so it has to be supplied in a different way */
        CORRADE_TESTSUITE_LOCAL void printFileLineInfo(Debug& out, std::size_t line);
        void verifyInternal(const char* expression, bool value);
        void printComparisonMessageInternal(ComparisonStatusFlags flags, const char* actual, const char* expected, void(*printer)(void*, ComparisonStatusFlags, Debug&, const char*, const char*), void(*saver)(void*, ComparisonStatusFlags, Debug&, const Containers::StringView&), void* comparator);

        void wallTimeBenchmarkBegin();
        std::uint64_t wallTimeBenchmarkEnd();

        void cpuTimeBenchmarkBegin();
        std::uint64_t cpuTimeBenchmarkEnd();

        void cpuCyclesBenchmarkBegin();
        std::uint64_t cpuCyclesBenchmarkEnd();

        void addTestCaseInternal(const TestCase& testCase);

        Containers::Pointer<TesterState> _state;
};

/**
@brief Instanced test case description with source location
@m_since_latest

When used instead of @ref Containers::StringView or @cpp const char* @ce to
define @ref TestSuite-Tester-instanced "instanced test case descriptions", any
messages printed to the output will contain also the file/line info of where
the instance data were defined in addition of file/line location from where the
message originated:

@m_class{m-code-figure}

@parblock

@snippet testsuite-description-source-location.cpp 0

<b></b>

@m_class{m-nopad m-console-wrap}

@include testsuite-description-source-location.ansi

@endparblock

Useful especially in combination with terminals that are capable of treating
the location information as a direct link to an IDE or text editor --- clicking
on @cb{.shell-session} …/PathTest.cpp:55 @ce will open the editor at the line
containing the @cpp "two extensions" @ce test case instance.

At the moment, this feature is available on GCC at least since version 4.8
(although it may not be giving correct results until version 12), Clang 9+ and
MSVC 2019 16.6 and newer. Elsewhere it behaves like if just a regular
@ref Tester::setTestCaseDescription(Containers::StringView) was used. You can
check for its availability using the
@ref CORRADE_SOURCE_LOCATION_BUILTINS_SUPPORTED predefined macro.

@see @ref Tester, @ref testsuite
*/
class CORRADE_TESTSUITE_EXPORT TestCaseDescriptionSourceLocation {
    public:
        /** @brief Constructor */
        /*implicit*/ TestCaseDescriptionSourceLocation(Containers::StringView description
            #ifdef CORRADE_SOURCE_LOCATION_BUILTINS_SUPPORTED
            #if defined(CORRADE_TARGET_GCC) || defined(CORRADE_TARGET_CLANG) || defined(CORRADE_TARGET_MSVC)
            /* Not using std::experimental::source_location because it's not in
               libc++ 9 yet and GCC version has a C++14 usage of constexpr */
            , int line = __builtin_LINE()
            #else
            #error this needs to be implemented for new compilers
            #endif
            #endif
        );
        /** @overload */
        /* So people aren't forced to include StringView if they don't want */
        /*implicit*/ TestCaseDescriptionSourceLocation(const char* description
            #ifdef CORRADE_SOURCE_LOCATION_BUILTINS_SUPPORTED
            #if defined(CORRADE_TARGET_GCC) || defined(CORRADE_TARGET_CLANG) || defined(CORRADE_TARGET_MSVC)
            /* Not using std::experimental::source_location because it's not in
               libc++ 9 yet and GCC version has a C++14 usage of constexpr */
            , int line = __builtin_LINE()
            #else
            #error this needs to be implemented for new compilers
            #endif
            #endif
        );

        /** @brief Conversion to a string view */
        operator Containers::StringView() const;

    private:
        friend Tester;

        /* A silly way to avoid including StringView */
        const char* _data;
        std::size_t _size;
        Containers::StringViewFlags _flags;
        #ifdef CORRADE_SOURCE_LOCATION_BUILTINS_SUPPORTED
        int _line;
        #endif
};

#ifndef DOXYGEN_GENERATING_OUTPUT
/* Done here because CORRADE_IS_DEBUG_BUILD is defined by the buildsystem when
   compiling the test (as opposed to defined when compiling this library) */
#ifdef CORRADE_IS_DEBUG_BUILD
#define _CORRADE_TESTSUITE_IS_DEBUG_BUILD true
#else
#define _CORRADE_TESTSUITE_IS_DEBUG_BUILD false
#endif
#endif

/** @hideinitializer
@brief Create `main()` function for given @ref Corrade::TestSuite::Tester "TestSuite::Tester" subclass

Populates @ref Corrade::TestSuite::Tester::arguments() "TestSuite::Tester::arguments()",
instantiates @p Class, executes the test cases and returns from @cpp main() @ce
with code based on the test results. This macro has to be used outside of any
namespace.
*/
#ifdef CORRADE_TESTSUITE_TARGET_XCTEST
/* Needs to have a separate definition to silence the -Wmissing-prototypes
   warning */
#define CORRADE_TEST_MAIN(Class)                                            \
    int CORRADE_VISIBILITY_EXPORT corradeTestMain(int, char**);             \
    int corradeTestMain(int argc, char** argv) {                            \
        Corrade::TestSuite::Tester::registerArguments(argc, argv);          \
        Class t;                                                            \
        t.registerTest(__FILE__, #Class, _CORRADE_TESTSUITE_IS_DEBUG_BUILD); \
        return t.exec();                                                    \
    }
#else
#define CORRADE_TEST_MAIN(Class)                                            \
    int main(int argc, char** argv) {                                       \
        Corrade::TestSuite::Tester::registerArguments(argc, argv);          \
        Class t;                                                            \
        t.registerTest(__FILE__, #Class, _CORRADE_TESTSUITE_IS_DEBUG_BUILD); \
        return t.exec();                                                    \
    }
#endif

/** @hideinitializer
@brief Verify an expression in a test case
@param ...      Expression to verify

If the expression is not true, the expression is printed and execution of given
test case is terminated. Example usage:

@snippet TestSuite.cpp CORRADE_VERIFY

It is possible to use @ref CORRADE_VERIFY() also on objects with
@cpp explicit operator bool() @ce without doing explicit conversion (e.g. using
@cpp !! @ce), for example:

@snippet TestSuite.cpp CORRADE_VERIFY-explicit

This macro is meant to be called in a test case in a
@ref Corrade::TestSuite::Tester "TestSuite::Tester" subclass. It's possible to
also call it in a helper function or lambda called from inside a test case,
however note that the very first call to a `CORRADE_*()` macro captures the
caller function name for the test output, which may not be desired when being
in a helper function or a lambda. To circumvent that, either call a dummy
@cpp CORRADE_VERIFY(true) @ce at the top of your test case, or explicitly call
@ref Corrade::TestSuite::Tester::setTestCaseName() "setTestCaseName()" with
either a hardcoded name or e.g. @ref CORRADE_FUNCTION.
@see @ref CORRADE_COMPARE(), @ref CORRADE_COMPARE_AS(), @ref CORRADE_FAIL_IF()
*/
#define CORRADE_VERIFY(...)                                                 \
    do {                                                                    \
        Corrade::TestSuite::Tester::instance().registerTestCase(CORRADE_FUNCTION, __LINE__); \
        Corrade::TestSuite::Tester::instance().verify(#__VA_ARGS__, __VA_ARGS__); \
    } while(false)

/** @hideinitializer
@brief Compare two values in a test case
@param actual       Calculated value
@param expected     Ground truth value

If the values are not the same, they are printed for comparison and execution
of given test case is terminated. Example usage:

@snippet TestSuite.cpp CORRADE_COMPARE

Comparison of floating-point types is by default done as a fuzzy-compare, see
@ref Corrade::TestSuite::Comparator<float> "TestSuite::Comparator<float>" and
@ref Corrade::TestSuite::Comparator<double> "TestSuite::Comparator<double>" for
details.

Note that this macro is usable only if the type passed to it is printable via
@ref Corrade::Utility::Debug "Utility::Debug". It is meant to be called in a
test case in a @ref Corrade::TestSuite::Tester "TestSuite::Tester" subclass.
It's possible to also call it in a helper function or lambda called from inside
a test case with some caveats. See @ref CORRADE_VERIFY() for details.
@see @ref CORRADE_COMPARE_AS(), @ref CORRADE_FAIL_IF()
*/
/* Unlike CORRADE_VERIFY(), while `expected` could be `...` as well and thus
   avoid the need to wrap the second value in parentheses (which is exceedingly
   common with {}-constructed values), it's not done as accidental use of a
   comma in the first value could lead to very cryptic errors -- for example
   `CORRADE_COMPARE(foo<T, U>(), b)` would treat `foo<T` as the first and
   `U>(), b` as the second argument and I don't even want to know what the
   compiler will yell about when seeing that. */
#define CORRADE_COMPARE(actual, expected)                                   \
    do {                                                                    \
        Corrade::TestSuite::Tester::instance().registerTestCase(CORRADE_FUNCTION, __LINE__); \
        Corrade::TestSuite::Tester::instance().compare(#actual, actual, #expected, expected); \
    } while(false)

/** @hideinitializer
@brief Compare two values in a test case with explicitly specified type
@param actual       Calculated value
@param expected     Ground truth value
@param ...          Type to compare as

Casts the values to a specified typ first and then continues the same as
@ref CORRADE_COMPARE(). If the values are not the same, they are printed for
comparison and execution of given test case is terminated. Example usage:

@snippet TestSuite.cpp CORRADE_COMPARE_AS

Note that this macro is usable only if the type passed to it is printable via
@ref Corrade::Utility::Debug "Utility::Debug" and is convertible to given type.
@ref CORRADE_COMPARE_AS() and @ref CORRADE_COMPARE_WITH() can be also used for
advanced comparisons with custom comparators, see
@ref TestSuite-Tester-advanced-comparisons for more information.

This macro is meant to be called in a test case in a
@ref Corrade::TestSuite::Tester "TestSuite::Tester" subclass. It's possible
to also call it in a helper function or lambda called from inside a test case
with some caveats. See @ref CORRADE_VERIFY() for details.
@see @ref CORRADE_VERIFY(), @ref CORRADE_FAIL_IF(),
    @ref Corrade::TestSuite::Comparator "TestSuite::Comparator"
*/
/* Similarly as above, doing `CORRADE_COMPARE_AS(foo<T, U>(), b, int)` would
   lead to `foo<T` being the first argument, `U>()` the second and `b, int` the
   third and lead to extremely cryptic compiler errors. However there's no
   other way how to pass templated types such as `std::map<int, bool>` (except
   for forcing people to do a typedef and pass that) so we have to leave that
   here even with the risk of the arguments being understood wrong in
   pathological cases. C preprocessor FTW. */
#define CORRADE_COMPARE_AS(actual, expected, ...)                           \
    do {                                                                    \
        Corrade::TestSuite::Tester::instance().registerTestCase(CORRADE_FUNCTION, __LINE__); \
        Corrade::TestSuite::Tester::instance().compareAs<__VA_ARGS__>(#actual, actual, #expected, expected); \
    } while(false)

/** @hideinitializer
@brief Compare two values in a test case with explicitly specified comparator
@param actual               Calculated value
@param expected             Ground truth value
@param comparatorInstance   Instance of a comparator to compare with

A variant of @ref CORRADE_COMPARE_AS() that takes a comparator instance instead
of type, useful when you need to pass additional parameters to the comparator.
See @ref TestSuite-Tester-advanced-comparisons for a high-level introduction
and an example. If the comparison fails, a comparator-specific diagnostic is
printed and execution of given test case is terminated. Example usage:

@snippet TestSuite.cpp CORRADE_COMPARE_WITH

Note that this macro is usable only if the type passed to it is compatible with
given comparator, and in some cases the comparator may require the type to also
be printable with @ref Corrade::Utility::Debug "Utility::Debug".

This macro is meant to be called in a test case in
a @ref Corrade::TestSuite::Tester "TestSuite::Tester" subclass. It's possible
to also call it in a helper function or lambda called from inside a test case
with some caveats. See @ref CORRADE_VERIFY() for details.
@see @ref CORRADE_VERIFY(), @ref CORRADE_COMPARE(), @ref CORRADE_FAIL_IF(),
    @ref Corrade::TestSuite::Comparator "TestSuite::Comparator"
*/
#define CORRADE_COMPARE_WITH(actual, expected, comparatorInstance)          \
    do {                                                                    \
        Corrade::TestSuite::Tester::instance().registerTestCase(CORRADE_FUNCTION, __LINE__); \
        Corrade::TestSuite::Tester::instance().compareWith((comparatorInstance).comparator(), #actual, actual, #expected, expected); \
    } while(false)

/** @hideinitializer
@brief Expect failure in a test case in all following checks in the same scope
@param message Message which will be printed as an indication of an expected
    failure

Expects a failure in all following @ref CORRADE_VERIFY(), @ref CORRADE_COMPARE(),
@ref CORRADE_COMPARE_AS(), @ref CORRADE_COMPARE_WITH() and @ref CORRADE_FAIL_IF()
checks in the same scope. Implicitly it will be until the end of the function,
but you can limit the scope by placing relevant checks in a separate block. If
any check following the macro in the same scope passes, an error will be
printed to the output.

@snippet TestSuite.cpp CORRADE_EXPECT_FAIL

The @p message can be formatted in the same way as in @ref CORRADE_ASSERT(),
including stream output operators.

This macro is meant to be called in a test case in a
@ref Corrade::TestSuite::Tester "TestSuite::Tester" subclass. It's possible to
also call it in a helper function or lambda called from inside a test case with
some caveats. See @ref CORRADE_VERIFY() for details.
@see @ref CORRADE_EXPECT_FAIL_IF()
*/
/* Although message could be a `...` like with CORRADE_ITERATION(), it's not
   done for consistency with CORRADE_EXPECT_FAIL_IF(), see the notice there
   for more info. */
#define CORRADE_EXPECT_FAIL(message)                                        \
    Corrade::TestSuite::Tester::ExpectedFailure _CORRADE_HELPER_PASTE(expectedFailure, __LINE__){(Corrade::TestSuite::Tester::instance().registerTestCase(CORRADE_FUNCTION), [&](Corrade::Utility::Debug&& _CORRADE_HELPER_PASTE(expectFailDebug, __LINE__)) { \
        _CORRADE_HELPER_PASTE(expectFailDebug, __LINE__) << message;        \
    })}

/** @hideinitializer
@brief Conditionally expect failure in a test case in all following checks in the same scope
@param condition    The failure is expected only if the condition evaluates to
    @cpp true @ce
@param message      Message which will be printed as an indication of an
    expected failure

With @ref CORRADE_EXPECT_FAIL() it's not possible to write code such as this,
because the scope of expected failure will end at the end of the @cpp if @ce
block:

@snippet TestSuite.cpp CORRADE_EXPECT_FAIL_IF-wrong

The solution is to use @cpp CORRADE_EXPECT_FAIL_IF() @ce:

@snippet TestSuite.cpp CORRADE_EXPECT_FAIL_IF

Similarly to @ref CORRADE_VERIFY(), it is possible to use
@ref CORRADE_EXPECT_FAIL_IF() also on objects with @cpp explicit operator bool @ce
without doing explicit conversion (e.g. using @cpp !! @ce).

The @p message can be formatted in the same way as in @ref CORRADE_ASSERT(),
including stream output operators.

This macro is meant to be called in a test case in a
@ref Corrade::TestSuite::Tester "TestSuite::Tester" subclass. It's possible to
also call it in a helper function or lambda called from inside a test case with
some caveats. See @ref CORRADE_VERIFY() for details.
*/
/* Although message could be a `...` like with CORRADE_ITERATION(), it's not
   done in order to avoid CORRADE_EXPECT_FAIL_IF(std::is_same<T, int>{}, "...")
   being interpreted in a wrong way (in particular, the __VA_ARGS__ being
   `int>{}, "..."`. Same is done in CORRADE_COMPARE(), for example, but not
   in CORRADE_SKIP() or CORRADE_ITERATION() as those have just one argument. */
#define CORRADE_EXPECT_FAIL_IF(condition, message)                          \
    Corrade::TestSuite::Tester::ExpectedFailure _CORRADE_HELPER_PASTE(expectedFailure, __LINE__)((Corrade::TestSuite::Tester::instance().registerTestCase(CORRADE_FUNCTION), [&](Corrade::Utility::Debug&& _CORRADE_HELPER_PASTE(expectFailIfDebug, __LINE__)) { \
        _CORRADE_HELPER_PASTE(expectFailIfDebug, __LINE__) << message;      \
    }), condition)

/** @hideinitializer
@brief Print an info message
@param ...      Message to print
@m_since_latest

Compared to using @relativeref{Corrade,Utility::Debug} directly, the message
will be prefixed with @cb{.ansi} [1;39mINFO @ce, test case name and file/line
info to be clear where the message comes from. This then replaces the usual
@cb{.ansi} [1;39mOK @ce, which isn't printed to avoid redundancy in the
output. The message can be formatted in the same way as in
@ref CORRADE_ASSERT(), including stream output operators:

@snippet TestSuite.cpp CORRADE_INFO

This macro is meant to be called in a test case in a
@relativeref{Corrade,TestSuite::Tester} subclass. It's possible to also call it
in a helper function or lambda called from inside a test case with some
caveats. See @ref CORRADE_VERIFY() for details.
@see @ref CORRADE_WARN(), @ref CORRADE_FAIL(), @ref CORRADE_FAIL_IF(),
    @ref CORRADE_SKIP()
*/
#define CORRADE_INFO(...)                                                   \
    Corrade::TestSuite::Tester::instance().infoOrWarn(Corrade::TestSuite::Tester::Printer{(Corrade::TestSuite::Tester::instance().registerTestCase(CORRADE_FUNCTION), [&](Corrade::Utility::Debug&& _CORRADE_HELPER_PASTE(infoDebug, __LINE__)) { \
        _CORRADE_HELPER_PASTE(infoDebug, __LINE__) << __VA_ARGS__;          \
    })}, __LINE__, false)

/** @hideinitializer
@brief Print a warning message
@param ...      Warning to print
@m_since_latest

Like @ref CORRADE_INFO(), but prefixes the output with @cb{.ansi} [1;33mWARN @ce
instead, replacing the usual @cb{.ansi} [1;39mOK @ce message as well. A
warning has no effect on the test result and doesn't end execution of the test
case either. The message can be formatted in the same way as in
@ref CORRADE_ASSERT(), including stream output operators:

@snippet TestSuite.cpp CORRADE_WARN

This macro is meant to be called in a test case in a
@relativeref{Corrade,TestSuite::Tester} subclass. It's possible to also call it
in a helper function or lambda called from inside a test case with some
caveats. See @ref CORRADE_VERIFY() for details.
@see @ref CORRADE_FAIL(), @ref CORRADE_FAIL_IF(), @ref CORRADE_SKIP()
*/
#define CORRADE_WARN(...)                                                   \
    Corrade::TestSuite::Tester::instance().infoOrWarn(Corrade::TestSuite::Tester::Printer{(Corrade::TestSuite::Tester::instance().registerTestCase(CORRADE_FUNCTION), [&](Corrade::Utility::Debug&& _CORRADE_HELPER_PASTE(infoDebug, __LINE__)) { \
        _CORRADE_HELPER_PASTE(infoDebug, __LINE__) << __VA_ARGS__;          \
    })}, __LINE__, true)

/** @hideinitializer
@brief Explicitly fail a test case
@param ...      Failure message to print
@m_since_latest

Useful for example to test a particular @cpp #define @ce, in which case there's
no expression or value to pass to @ref CORRADE_VERIFY() / @ref CORRADE_COMPARE();
or to check that given code path is never reached.

@snippet TestSuite.cpp CORRADE_FAIL

Even though the failure is unconditional, the test case can still continue
execution when combined with @ref CORRADE_EXPECT_FAIL() /
@ref CORRADE_EXPECT_FAIL_IF(). Such behavior is thus different from
@ref CORRADE_ASSERT_UNREACHABLE(), where the macro also tells the compiler the
code following it will never be executed, allowing to omit @cpp return @ce
statements among other things.

The message is prefixed with @cb{.ansi} [1;31mFAIL @ce including a file and
line where the failure happened and execution of given test case is terminated.
The message can be formatted in the same way as in @ref CORRADE_ASSERT(), including stream output operators. Note that, however, it isn't meant to
be used as the single verification macro in a test case:

@snippet TestSuite.cpp CORRADE_FAIL-wrong

In such a case, @ref CORRADE_FAIL_IF() should be used to ensure it's always
reached when running the test:

@snippet TestSuite.cpp CORRADE_FAIL_IF

This macro is meant to be called in a test case in a
@relativeref{Corrade,TestSuite::Tester} subclass. It's possible to also call it
in a helper function or lambda called from inside a test case with some
caveats. See @ref CORRADE_VERIFY() for details.
@see @ref CORRADE_INFO(), @ref CORRADE_WARN(), @ref CORRADE_SKIP()
*/
/* Not delegating to CORRADE_FAIL_IF() because I feel like the varargs are a
   useful property here, making it consistent with the CORRADE_INFO() and
   CORRADE_WARN() macros. */
#define CORRADE_FAIL(...)                                                   \
    Corrade::TestSuite::Tester::instance().failIf(Corrade::TestSuite::Tester::Printer{(Corrade::TestSuite::Tester::instance().registerTestCase(CORRADE_FUNCTION, __LINE__), [&](Corrade::Utility::Debug&& _CORRADE_HELPER_PASTE(infoDebug, __LINE__)) { \
        _CORRADE_HELPER_PASTE(infoDebug, __LINE__) << __VA_ARGS__;          \
    })}, true)

/** @hideinitializer
@brief Explicitly fail a test case if a condition is true
@param condition    Condition that's expected to evaluate to @cpp false @ce
@param message      Failure message which will be printed if the condition is
    @cpp true @ce
@m_since_latest

Useful when the implicit failure diagnostic from @ref CORRADE_VERIFY() or
@ref CORRADE_COMPARE() isn't descriptive enough. The message is prefixed with
@cb{.ansi} [1;31mFAIL @ce including a file and line where the failure happened
and execution of given test case is terminated. The message can be formatted in
the same way as in @ref CORRADE_ASSERT(), including stream output operators:

@snippet TestSuite.cpp CORRADE_FAIL_IF

This macro is meant to be called in a test case in a
@relativeref{Corrade,TestSuite::Tester} subclass. It's possible to also call it
in a helper function or lambda called from inside a test case with some
caveats. See @ref CORRADE_VERIFY() for details.
@see @ref CORRADE_INFO(), @ref CORRADE_WARN(), @ref CORRADE_FAIL(),
    @ref CORRADE_SKIP()
*/
#define CORRADE_FAIL_IF(condition, message)                                 \
    Corrade::TestSuite::Tester::instance().failIf(Corrade::TestSuite::Tester::Printer{(Corrade::TestSuite::Tester::instance().registerTestCase(CORRADE_FUNCTION, __LINE__), [&](Corrade::Utility::Debug&& _CORRADE_HELPER_PASTE(infoDebug, __LINE__)) { \
        _CORRADE_HELPER_PASTE(infoDebug, __LINE__) << message;              \
    })}, condition)

/** @hideinitializer
@brief Skip a test case
@param ...      Message which will be printed as an indication of a skipped
    test

Skips all following checks in given test case, printing a
@cb{.ansi} [1;39mSKIP @ce in the output. Useful for e.g. indicating that given
feature can't be tested on given platform:

@snippet TestSuite.cpp CORRADE_SKIP

The message can be formatted in the same way as in @ref CORRADE_ASSERT(),
including stream output operators.

This macro is meant to be called in a test case in a
@ref Corrade::TestSuite::Tester "TestSuite::Tester" subclass. It's possible to
also call it in a helper function or lambda called from inside a test case with
some caveats. See @ref CORRADE_VERIFY() for details.
@see @ref CORRADE_SKIP_IF_NO_ASSERT(), @ref CORRADE_SKIP_IF_NO_DEBUG_ASSERT(),
    @ref CORRADE_INFO(), @ref CORRADE_WARN(), @ref CORRADE_FAIL(),
    @ref CORRADE_FAIL_IF()
*/
#define CORRADE_SKIP(...)                                                   \
    Corrade::TestSuite::Tester::instance().skip(Corrade::TestSuite::Tester::Printer{(Corrade::TestSuite::Tester::instance().registerTestCase(CORRADE_FUNCTION), [&](Corrade::Utility::Debug&& _CORRADE_HELPER_PASTE(skipDebug, __LINE__)) { \
        _CORRADE_HELPER_PASTE(skipDebug, __LINE__) << __VA_ARGS__;          \
    })})

/** @hideinitializer
@brief Skip a test case if Corrade asserts are disabled
@m_since_latest

If @ref CORRADE_NO_ASSERT or @ref CORRADE_STANDARD_ASSERT is defined, expands
to a @ref CORRADE_SKIP() call. Otherwise expands to @cpp do {} while(false) @ce.
To be used in test cases that verify @ref CORRADE_ASSERT() and other assertion
macros and which would misbehave or crash if asserts are compiled out or
use the standard assertion macro which doesn't contain the custom message and
is unaffected by @ref CORRADE_GRACEFUL_ASSERT. Use
@ref CORRADE_SKIP_IF_NO_DEBUG_ASSERT() for testing @ref CORRADE_DEBUG_ASSERT()
and other debug assertion macros.

This macro is meant to be called in a test case in a
@ref Corrade::TestSuite::Tester "TestSuite::Tester" subclass. It's possible to
also call it in a helper function or lambda called from inside a test case with
some caveats. See @ref CORRADE_VERIFY() for details.
*/
#ifdef CORRADE_NO_ASSERT
#define CORRADE_SKIP_IF_NO_ASSERT() CORRADE_SKIP("CORRADE_NO_ASSERT defined, can't test assertions")
#elif defined(CORRADE_STANDARD_ASSERT)
#define CORRADE_SKIP_IF_NO_ASSERT() CORRADE_SKIP("CORRADE_STANDARD_ASSERT defined, can't test assertions")
#else
#define CORRADE_SKIP_IF_NO_ASSERT() do {} while(false)
#endif

/** @hideinitializer
@brief Skip a test case if Corrade debug asserts are disabled
@m_since_latest

If @ref CORRADE_NO_DEBUG_ASSERT or @ref CORRADE_STANDARD_ASSERT is defined,
expands to a @ref CORRADE_SKIP() call. Otherwise expands to
@cpp do {} while(false) @ce. To be used in test cases that verify
@ref CORRADE_DEBUG_ASSERT() and other assertion macros and which would
misbehave or crash if asserts are compiled out or use the standard assertion
macro which doesn't contain the custom message and is unaffected by
@ref CORRADE_GRACEFUL_ASSERT. Use @ref CORRADE_SKIP_IF_NO_ASSERT() for testing
@ref CORRADE_ASSERT() and other assertion macros.

This macro is meant to be called in a test case in a
@ref Corrade::TestSuite::Tester "TestSuite::Tester" subclass. It's possible to
also call it in a helper function or lambda called from inside a test case with
some caveats. See @ref CORRADE_VERIFY() for details.
*/
#ifdef CORRADE_NO_ASSERT
/* Not using CORRADE_NO_DEBUG_ASSERT in order to provide a clearer reason why
   the tests got skipped */
#define CORRADE_SKIP_IF_NO_DEBUG_ASSERT() CORRADE_SKIP("CORRADE_NO_ASSERT defined, can't test debug assertions")
#elif defined(CORRADE_STANDARD_ASSERT)
#define CORRADE_SKIP_IF_NO_DEBUG_ASSERT() CORRADE_SKIP("CORRADE_STANDARD_ASSERT defined, can't test assertions")
#elif !defined(CORRADE_IS_DEBUG_BUILD) && defined(NDEBUG)
#define CORRADE_SKIP_IF_NO_DEBUG_ASSERT() CORRADE_SKIP("CORRADE_IS_DEBUG_BUILD not defined and NDEBUG defined, can't test debug assertions")
#else
#define CORRADE_SKIP_IF_NO_DEBUG_ASSERT() do {} while(false)
#endif

/** @hideinitializer
@brief Annotate an iteration in a test case
@param ...      Value to print in a failure diagnostic
@m_since{2020,06}

Annotates loop iterations in order to provide clearer failure diagnostics next
to the file/line info. Doesn't print anything if there was no failure. Applies
to all following @ref CORRADE_VERIFY(), @ref CORRADE_COMPARE() etc. checks in
the same scope, multiple calls in the same scope (or nested scopes) are joined
together. See @ref TestSuite-Tester-iteration-annotations for an example.

The value can be formatted in the same way as in @ref CORRADE_ASSERT(),
including stream output operators.

This macro is meant to be called in a test case in a
@ref Corrade::TestSuite::Tester "TestSuite::Tester" subclass. It's possible to
also call it in a helper function or lambda called from inside a test case with
some caveats. See @ref CORRADE_VERIFY() for details.
*/
#define CORRADE_ITERATION(...)                                              \
    Corrade::TestSuite::Tester::IterationPrinter _CORRADE_HELPER_PASTE(iterationPrinter, __LINE__){(Corrade::TestSuite::Tester::instance().registerTestCase(CORRADE_FUNCTION), [&](Corrade::Utility::Debug&& _CORRADE_HELPER_PASTE(iterationPrinterDebug, __LINE__)) { \
        _CORRADE_HELPER_PASTE(iterationPrinterDebug, __LINE__) << __VA_ARGS__; \
    })}

/** @hideinitializer
@brief Run a benchmark in a test case

Benchmarks the following block or expression by measuring @p batchSize
iterations of given block. Desired use is in conjunction with
@ref Corrade::TestSuite::Tester::addBenchmarks() "TestSuite::Tester::addBenchmarks()"
and friends, see @ref TestSuite-Tester-benchmark for an introduction and an
example. Only one such loop can be in a function to achieve proper result.
Please note that there need to be additional measures in order to prevent the
optimizer from removing the benchmark code such as assigning to a
@cpp volatile @ce variable or combining all the results to a variable, which is
then being used outside of the loop.

@snippet TestSuite.cpp CORRADE_BENCHMARK

The resulting measured value is divided by @p batchSize to represent cost of
one iteration.

This macro is meant to be called in a test case in a
@ref Corrade::TestSuite::Tester "TestSuite::Tester" subclass. It's possible to
also call it in a helper function or lambda called from inside a test case with
some caveats. See @ref CORRADE_VERIFY() for details.
*/
#ifndef CORRADE_TARGET_MSVC
#define CORRADE_BENCHMARK(batchSize)                                        \
    Corrade::TestSuite::Tester::instance().registerTestCase(CORRADE_FUNCTION, __LINE__); \
    for(CORRADE_UNUSED auto&& _CORRADE_HELPER_PASTE(benchmarkIteration, __func__): Corrade::TestSuite::Tester::instance().createBenchmarkRunner(batchSize))
#else
/* MSVC warns about the benchmarkIteration variable being set but unused, no
   way around that except than disabling the warning */
#define CORRADE_BENCHMARK(batchSize)                                        \
    Corrade::TestSuite::Tester::instance().registerTestCase(CORRADE_FUNCTION, __LINE__); \
    for(                                                                    \
        __pragma(warning(push)) __pragma(warning(disable: 4189))            \
        CORRADE_UNUSED auto&& _CORRADE_HELPER_PASTE(benchmarkIteration, __func__): Corrade::TestSuite::Tester::instance().createBenchmarkRunner(batchSize) \
        __pragma(warning(pop))                                              \
    )
#endif

template<class T, class U, class V> void Tester::compareWith(Comparator<T>& comparator, const char* actual, const U& actualValue, const char* expected, const V& expectedValue) {
    /* Store (references to) possibly implicitly-converted values,
       otherwise the implicit conversion would when passing them to operator(),
       causing dead memory access later in printErrorMessage() */
    const typename Implementation::ComparatorTraits<T, typename std::decay<U>::type, typename std::decay<V>::type>::ActualType& actualValueInExpectedActualType = actualValue;
    const typename Implementation::ComparatorTraits<T, typename std::decay<U>::type, typename std::decay<V>::type>::ExpectedType& expectedValueInExpectedExpectedType = expectedValue;

    /* Compare and then print the message, if needed */
    ComparisonStatusFlags status =
        #ifdef CORRADE_BUILD_DEPRECATED
        Implementation::comparisonStatusFlags(
        #endif
            comparator(actualValueInExpectedActualType, expectedValueInExpectedExpectedType)
        #ifdef CORRADE_BUILD_DEPRECATED
        )
        #endif
        ;

    printComparisonMessageInternal(status, actual, expected,
        [](void* comparator, ComparisonStatusFlags flags, Debug& out, const char* actual, const char* expected) {
            #ifndef CORRADE_BUILD_DEPRECATED
            static_cast<Comparator<T>*>(comparator)->printMessage(flags, out, actual, expected);
            #else
            Implementation::printMessage<Comparator<T>>(*static_cast<Comparator<T>*>(comparator), flags, out, actual, expected);
            #endif
        }, Implementation::diagnosticSaver<T>(), &comparator);
}

template<class T> void Tester::verify(const char* expression, T&& value) {
    if(value) verifyInternal(expression, true);
    else verifyInternal(expression, false);
}

}}

#endif
