#ifndef Corrade_TestSuite_Tester_h
#define Corrade_TestSuite_Tester_h
/*
    This file is part of Corrade.

    Copyright © 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016,
                2017, 2018, 2019, 2020 Vladimír Vondruš <mosra@centrum.cz>

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
 * @brief Class @ref Corrade::TestSuite::Tester, macros @ref CORRADE_TEST_MAIN(), @ref CORRADE_VERIFY(), @ref CORRADE_COMPARE(), @ref CORRADE_COMPARE_AS(), @ref CORRADE_COMPARE_WITH(), @ref CORRADE_EXPECT_FAIL(), @ref CORRADE_EXPECT_FAIL_IF(), @ref CORRADE_SKIP(), @ref CORRADE_ITERATION(), @ref CORRADE_BENCHMARK()
 */

#include <initializer_list>

#include "Corrade/Containers/Pointer.h"
#include "Corrade/TestSuite/Comparator.h"
#include "Corrade/TestSuite/Compare/FloatingPoint.h"
#include "Corrade/TestSuite/visibility.h"
#include "Corrade/Utility/Debug.h"
#include "Corrade/Utility/Macros.h"
#include "Corrade/Utility/StlForwardString.h"

#ifdef CORRADE_TARGET_EMSCRIPTEN
#include <cstdlib>
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
given platform (printing a @cb{.ansi} [1;39mSKIP @ce on output and exiting the
test case right after the statement) or documenting that some algorithm
produces incorrect result due to a bug, printing an @cb{.ansi} [1;33mXFAIL @ce.
Passing a test while failure is expected is treated as an error
(@cb{.ansi} [1;31mXPASS @ce), which can be helpful to ensure the assumptions
in the tests don't get stale. Expected failures can be also disabled globally
via a command-line option `--no-xfail` or via environment variable,
@ref TestSuite-Tester-command-line "see below".

The only reason why those are macros and not member functions is the ability to
gather class/function/file/line/expression information via the preprocessor for
printing the test output and exact location of possible test failure. If none
of these macros is encountered when running the test case, the test case is
reported as invalid, with @cb{.ansi} [1;33m? @ce on output.

The test cases are numbered on the output and those numbers can be used on the
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
executed. The test output contains number of executed repeats after the test
case name, prefixed by @cb{.ansi} [1;39m@ @ce. Example of testing race
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
@ref Compare::Container and @ref Compare::SortedContainer can be used for
convenient comparison of large containers. Compared to plain
@ref CORRADE_COMPARE() Its diagnostic will show where exactly the containers
differ, which becomes useful with large data sizes:

@snippet TestSuite.cpp Compare-Container
</li>
<li>
@ref Compare::File, @ref Compare::FileToString and @ref Compare::StringToFile
allow you to compare files without having to manually read them:

@snippet TestSuite.cpp Compare-File
</li>
<li>
@ref Compare::Less, @ref Compare::Around and others from the
@ref Corrade/TestSuite/Compare/Numeric.h header allow you to do numeric
comparisons that again provide better failure reporting compared to checking an
expression result with @ref CORRADE_VERIFY():

@snippet TestSuite.cpp Compare-around-just-one
</li>
<li>
Finally, [Magnum](https://magnum.graphics) has a
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
benchmarks. The default benchmark type can be also overriden
@ref TestSuite-Tester-command-line "on the command-line" via `--benchmark`.

It's possible to use all @ref CORRADE_VERIFY(), @ref CORRADE_COMPARE() etc.
verification macros inside the benchmark to check pre/post-conditions. If one
of them fails, the benchmark is treated on output just like failing test, with
no benchmark results being printed out. Keep in mind, however, that those
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

Different benchmark type have different units, time values are displayed in
`ns`, `µs`, `ms` and `s`, dimensionless count is suffixed by `k`, `M` or `G`
indicating thousands, millions and billions, instructions with `I`, `kI`, `MI`
and `GI`, cycles with `C`, `kC`, `MC` and `GC` and memory in `B`, `kB`, `MB`
and `GB`. In case of memory the prefixes are multiples of 1024 instead of 1000.
For easier visual recognition of the values, by default the sample standard
deviation is colored yellow if it is larger than 5% of the absolute value of
the mean and red if it is larger than 25% of the absolute value of the mean.
This can be overriden @ref TestSuite-Tester-command-line "on the command-line"
via `--benchmark-yellow` and `--benchmark-red`.

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
caught to avoid interferring with serious issues such as memory access errors.
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
./my-test [-h|--help] [-c|--color on|off|auto] [--skip "N1 N2..."]
    [--skip-tests] [--skip-benchmarks] [--only "N1 N2..."] [--shuffle]
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
-   `--skip "N1 N2..."` --- skip test cases with given numbers
-   `--skip-tests` --- skip all tests (environment:
    `CORRADE_TEST_SKIP_TESTS=ON|OFF`)
-   `--skip-benchmarks` --- skip all benchmarks (environment:
    `CORRADE_TEST_SKIP_BENCHMARKS=ON|OFF`)
-   `--only "N1 N2..."` --- run only test cases with given numbers
-   `--shuffle` --- randomly shuffle test case order (environment:
    `CORRADE_TEST_SHUFFLE=ON|OFF`)
-   `--repeat-every N` --- repeat every test case N times (environment:
    `CORRADE_TEST_REPEAT_EVERY`, default: `1`)
-   `--repeat-all N` --- repeat all test cases N times (environment:
    `CORRADE_TEST_REPEAT_ALL`, default: `1`)
-   `--abort-on-fail` --- abort after first failure (environment:
    `CORRADE_TEST_ABORT_ON_FAIL=ON|OFF`)
-   `--no-xfail` --- disallow expected failures (environment:
    `CORRADE_TEST_NO_XFAIL=ON|OFF`)
-   `--no-catch` --- don't catch standard exceptions (environment:
    `CORRADE_TEST_NO_CATCH=ON|OFF`)
-   `--save-diagnostic PATH` --- save diagnostic files to given path
    (environment: `CORRADE_TEST_SAVE_DIAGNOSTIC`)
-   `-v`, `--verbose` --- enable verbose output (environment:
    `CORRADE_TEST_VERBOSE`)
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
    being silently ignored. See [Android Issue 3254](http://web.archive.org/web/20160806094132/https://code.google.com/p/android/issues/detail?id=3254)
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
*/
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
                Containers::ArrayView<const std::string> skippedArgumentPrefixes() const;

                /**
                 * @brief Set skipped argument prefixes
                 *
                 * Useful to allow passing command-line arguments elsewhere
                 * without having the tester complaining about them.
                 * @see @ref arguments()
                 */
                TesterConfiguration& setSkippedArgumentPrefixes(std::initializer_list<std::string> prefixes);

                #if defined(__linux__) || defined(DOXYGEN_GENERATING_OUTPUT)
                /** @brief Where to check for active CPU scaling governor */
                std::string cpuScalingGovernorFile() const;

                /**
                 * @brief Set where to check for active CPU scaling governor
                 *
                 * Running benchmarks on a system with dynamic CPU scaling
                 * makes the measurements very noisy. If that's detected, a
                 * warning is printed on output. Defaults to
                 * `/sys/devices/system/cpu/cpu{}/cpufreq/scaling_governor`,
                 * where `{}` is replaced with CPU ID; if the file doesn't
                 * exist, no check is done.
                 * @partialsupport Available only on Linux.
                 */
                TesterConfiguration& setCpuScalingGovernorFile(const std::string& filename);
                #endif

            private:
                friend Tester;

                /* Don't want to include any vector or array here because we
                   don't need it in public APIs anyway. */
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
             * overriden on command-line using the `--benchmark` option.
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
            /* Values should not overlap with BenchmarkType */

            Nanoseconds = 100,      /**< Time in nanoseconds */
            Cycles = 101,           /**< Processor cycle count */
            Instructions = 102,     /**< Processor instruction count */
            Bytes = 103,            /**< Memory (in bytes) */
            Count = 104             /**< Generic count */
        };

        /**
         * @brief Constructor
         * @param configuration     Optional configuration
         */
        explicit Tester(const TesterConfiguration& configuration = TesterConfiguration{});

        /**
         * @brief Command-line arguments
         *
         * Populated by @ref CORRADE_TEST_MAIN().
         */
        std::pair<int&, char**> arguments() { return {*_argc, _argv}; }

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
         * in the output log once for each occurence in the list.
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
         * that case it will appear in the output log once for each occurence
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
         * occurence in the list.
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
         * appear once for each instance of each occurence in the list.
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
         * in that case it will appear once for each instance of each occurence
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
         * each occurence in the list.
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
         * each instance of each occurence in the list.
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
         * occurence in the list.
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
         * it will appear once for each instance of each occurence in the list.
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
         * it will appear once for each instance of each occurence in the list.
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
         * @see @ref setTestCaseName(), @ref setTestCaseTemplateName(),
         *      @ref setTestCaseDescription()
         */
        void setTestName(const std::string& name);
        void setTestName(std::string&& name); /**< @overload */
        void setTestName(const char* name); /**< @overload */

        /**
         * @brief Set custom test case name
         *
         * By default the test case name is gathered in the check macros and is
         * equivalent to the following:
         *
         * @snippet TestSuite.cpp Tester-setTestCaseName
         *
         * @see @ref setTestCaseTemplateName(), @ref setTestName(),
         *      @ref setTestCaseDescription(), @ref CORRADE_FUNCTION
         */
        void setTestCaseName(const std::string& name);
        void setTestCaseName(std::string&& name); /**< @overload */
        void setTestCaseName(const char* name); /**< @overload */

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
         * @see @ref setTestCaseName(), @ref setTestName(),
         *      @ref setTestCaseDescription(), @ref CORRADE_FUNCTION
         */
        void setTestCaseTemplateName(const std::string& name);

        /**
         * @overload
         * @m_since{2019,10}
         */
        void setTestCaseTemplateName(std::string&& name);

        /**
         * @overload
         * @m_since{2019,10}
         */
        void setTestCaseTemplateName(const char* name);

        /**
         * @overload
         * @m_since_latest
         *
         * Useful for test cases that are templated with more than one
         * parameter. Names are joined with `,`.
         */
        void setTestCaseTemplateName(std::initializer_list<Containers::StringView> names);
        /**
         * @overload
         * @m_since_latest
         *
         * Has to be present in order to avoid @cpp {"abc", "def"} @ce being
         * interpreted as a begin/end @ref std::string constructor causing all
         * sorts of nasty memory issues. Sigh.
         * @todo remove once we get rid of @ref std::string
         */
        void setTestCaseTemplateName(std::initializer_list<const char*> names);

        /**
         * @brief Set test case description
         *
         * Additional text displayed after the test case name. By default
         * the description is empty for non-instanced test cases and instance
         * ID for instanced test cases.
         * @see @ref setTestName(), @ref setTestCaseName(),
         *      @ref setTestCaseTemplateName()
         */
        void setTestCaseDescription(const std::string& description);
        void setTestCaseDescription(std::string&& description); /**< @overload */
        void setTestCaseDescription(const char* description); /**< @overload */

        /**
         * @brief Set benchmark name
         *
         * In case of @ref addCustomBenchmarks() and @ref addCustomInstancedBenchmarks()
         * provides the name for the unit measured, for example @cpp "wall time" @ce.
         */
        void setBenchmarkName(const std::string& name);
        void setBenchmarkName(std::string&& name); /**< @overload */
        void setBenchmarkName(const char* name); /**< @overload */

    protected:
        ~Tester();

    #ifdef DOXYGEN_GENERATING_OUTPUT
    private:
    #else
    public:
    #endif
        /* Called from CORRADE_TEST_MAIN(). argc is grabbed via a mutable
           reference and argv is grabbed as non-const in order to allow the
           users modifying the argument list (and GLUT requires that) */
        static void registerArguments(int& argc, char** argv);

        /* Overload needed for testing */
        static void registerArguments(int& argc, const char** argv) {
            registerArguments(argc, const_cast<char**>(argv));
        }

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

        /* Called from CORRADE_TEST_MAIN() */
        void registerTest(const char* filename, const char* name, bool isDebugBuild = false);

        /* Called from CORRADE_SKIP() */
        CORRADE_NORETURN void skip(const std::string& message);
        CORRADE_NORETURN void skip(const char* message);

        class CORRADE_TESTSUITE_EXPORT ExpectedFailure {
            public:
                explicit ExpectedFailure(const std::string& message, bool enabled = true);
                explicit ExpectedFailure(std::string&& message, bool enabled = true);
                explicit ExpectedFailure(const char* message, bool enabled = true);

                /* For types with explicit bool conversion */
                template<class T> explicit ExpectedFailure(const std::string& message, T&& enabled): ExpectedFailure{message, enabled ? true : false} {}
                template<class T> explicit ExpectedFailure(std::string&& message, T&& enabled): ExpectedFailure{message, enabled ? true : false} {}
                template<class T> explicit ExpectedFailure(const char* message, T&& enabled): ExpectedFailure{message, enabled ? true : false} {}

                ~ExpectedFailure();
        };

        class CORRADE_TESTSUITE_EXPORT IterationPrinter {
            public:
                IterationPrinter();
                ~IterationPrinter();

                Debug debug();

            private:
                friend Tester;
                struct Data;

                /* There's a std::ostringstream inside (yes, ew); don't want
                   that in a header */
                Containers::Pointer<Data> _data;
        };

        /* Called from all CORRADE_*() verification/skip/xfail macros through
           _CORRADE_REGISTER_TEST_CASE() */
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
            CustomCountBenchmark = int(BenchmarkUnits::Count)
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

                const char* begin() const { return nullptr; }
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
        static char** _argv;

        CORRADE_TESTSUITE_LOCAL void printTestCaseLabel(Debug& out, const char* status, Debug::Color statusColor, Debug::Color labelColor);
        CORRADE_TESTSUITE_LOCAL void printFileLineInfo(Debug& out);
        void verifyInternal(const char* expression, bool value);
        void printComparisonMessageInternal(ComparisonStatusFlags flags, const char* actual, const char* expected, void(*printer)(void*, ComparisonStatusFlags, Debug&, const char*, const char*), void(*saver)(void*, ComparisonStatusFlags, Debug&, const std::string&), void* comparator);

        void wallTimeBenchmarkBegin();
        std::uint64_t wallTimeBenchmarkEnd();

        void cpuTimeBenchmarkBegin();
        std::uint64_t cpuTimeBenchmarkEnd();

        void cpuCyclesBenchmarkBegin();
        std::uint64_t cpuCyclesBenchmarkEnd();

        void addTestCaseInternal(const TestCase& testCase);

        Containers::Pointer<TesterState> _state;
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
/* Needs to have a separate definiton to silence the -Wmissing-prototypes
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
@see @ref CORRADE_COMPARE(), @ref CORRADE_COMPARE_AS()
*/
#define CORRADE_VERIFY(expression)                                          \
    do {                                                                    \
        Corrade::TestSuite::Tester::instance().registerTestCase(CORRADE_FUNCTION, __LINE__); \
        Corrade::TestSuite::Tester::instance().verify(#expression, expression); \
    } while(false)

/** @hideinitializer
@brief Compare two values in a test case

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
@see @ref CORRADE_COMPARE_AS()
*/
#define CORRADE_COMPARE(actual, expected)                                   \
    do {                                                                    \
        Corrade::TestSuite::Tester::instance().registerTestCase(CORRADE_FUNCTION, __LINE__); \
        Corrade::TestSuite::Tester::instance().compare(#actual, actual, #expected, expected); \
    } while(false)

/** @hideinitializer
@brief Compare two values in a test case with explicitly specified type

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
@see @ref CORRADE_VERIFY(),
    @ref Corrade::TestSuite::Comparator "TestSuite::Comparator"
*/
#ifdef DOXYGEN_GENERATING_OUTPUT
#define CORRADE_COMPARE_AS(actual, expected, Type...)
#else
#define CORRADE_COMPARE_AS(actual, expected, ...)                           \
    do {                                                                    \
        Corrade::TestSuite::Tester::instance().registerTestCase(CORRADE_FUNCTION, __LINE__); \
        Corrade::TestSuite::Tester::instance().compareAs<__VA_ARGS__>(#actual, actual, #expected, expected); \
    } while(false)
#endif

/** @hideinitializer
@brief Compare two values in a test case with explicitly specified comparator

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
@see @ref CORRADE_VERIFY(), @ref CORRADE_COMPARE(), ,
    @ref Corrade::TestSuite::Comparator "TestSuite::Comparator"
*/
#define CORRADE_COMPARE_WITH(actual, expected, comparatorInstance)          \
    do {                                                                    \
        Corrade::TestSuite::Tester::instance().registerTestCase(CORRADE_FUNCTION, __LINE__); \
        Corrade::TestSuite::Tester::instance().compareWith((comparatorInstance).comparator(), #actual, actual, #expected, expected); \
    } while(false)

/** @hideinitializer
@brief Expect failure in a test case in all following checks in the same scope
@param message Message which will be printed into output as indication of
    expected failure

Expects failure in all following @ref CORRADE_VERIFY(), @ref CORRADE_COMPARE()
and @ref CORRADE_COMPARE_AS() checks in the same scope. In most cases it will
be until the end of the function, but you can limit the scope by placing
relevant checks in a separate block. If any check following the macro in the
same scope passes, an error will be printed to output.

@snippet TestSuite.cpp CORRADE_EXPECT_FAIL

This macro is meant to be called in a test case in a
@ref Corrade::TestSuite::Tester "TestSuite::Tester" subclass. It's possible to
also call it in a helper function or lambda called from inside a test case with
some caveats. See @ref CORRADE_VERIFY() for details.
@see @ref CORRADE_EXPECT_FAIL_IF()
*/
#define CORRADE_EXPECT_FAIL(message)                                        \
    Corrade::TestSuite::Tester::ExpectedFailure _CORRADE_HELPER_PASTE(expectedFailure, __LINE__){message}

/** @hideinitializer
@brief Conditionally expect failure in a test case in all following checks in the same scope
@param message      Message which will be printed into output as indication of
    expected failure
@param condition    The failure is expected only if the condition evaluates to
    @cpp true @ce

With @ref CORRADE_EXPECT_FAIL() it's not possible to write code such as this,
because the scope of expected failure will end at the end of the @cpp if @ce
block:

@snippet TestSuite.cpp CORRADE_EXPECT_FAIL_IF-wrong

The solution is to use @cpp CORRADE_EXPECT_FAIL_IF() @ce:

@snippet TestSuite.cpp CORRADE_EXPECT_FAIL_IF

Similarly to @ref CORRADE_VERIFY(), it is possible to use
@ref CORRADE_EXPECT_FAIL_IF() also on objects with @cpp explicit operator bool @ce
without doing explicit conversion (e.g. using @cpp !! @ce).

This macro is meant to be called in a test case in a
@ref Corrade::TestSuite::Tester "TestSuite::Tester" subclass. It's possible to
also call it in a helper function or lambda called from inside a test case with
some caveats. See @ref CORRADE_VERIFY() for details.
*/
#define CORRADE_EXPECT_FAIL_IF(condition, message)                          \
    Corrade::TestSuite::Tester::ExpectedFailure _CORRADE_HELPER_PASTE(expectedFailure, __LINE__)(message, condition)

/** @hideinitializer
@brief Skip a test case
@param message Message which will be printed into output as indication of
    skipped test

Skips all following checks in given test case. Useful for e.g. indicating that
given feature can't be tested on given platform:

@snippet TestSuite.cpp CORRADE_SKIP

This macro is meant to be called in a test case in a
@ref Corrade::TestSuite::Tester "TestSuite::Tester" subclass. It's possible to
also call it in a helper function or lambda called from inside a test case with
some caveats. See @ref CORRADE_VERIFY() for details.
*/
#define CORRADE_SKIP(message)                                               \
    do {                                                                    \
        Corrade::TestSuite::Tester::instance().registerTestCase(CORRADE_FUNCTION, __LINE__); \
        Corrade::TestSuite::Tester::instance().skip(message); \
    } while(false)

/** @hideinitializer
@brief Annotate an iteration in a test case
@param ...      Value to print in a failure diagnostic
@m_since{2020,06}

Annotates loop iterations in order to provide clearer failure diagnostics next
to the file/line info. Doesn't print anything if there was no failure. Applies
to all following @ref CORRADE_VERIFY(), @ref CORRADE_COMPARE() etc. checks in
the same scope, multiple calls in the same scope (or nested scopes) are joined
together. See @ref TestSuite-Tester-iteration-annotations for an example.

This macro is meant to be called in a test case in a
@ref Corrade::TestSuite::Tester "TestSuite::Tester" subclass. It's possible to
also call it in a helper function or lambda called from inside a test case with
some caveats. See @ref CORRADE_VERIFY() for details.
*/
#define CORRADE_ITERATION(...)                                              \
    Corrade::TestSuite::Tester::IterationPrinter _CORRADE_HELPER_PASTE(iterationPrinter, __LINE__); \
    do {                                                                    \
        Corrade::TestSuite::Tester::instance().registerTestCase(CORRADE_FUNCTION, __LINE__); \
        _CORRADE_HELPER_PASTE(iterationPrinter, __LINE__).debug() << __VA_ARGS__; \
    } while(false)

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
#ifndef _MSC_VER
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
