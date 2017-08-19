#ifndef Corrade_TestSuite_Tester_h
#define Corrade_TestSuite_Tester_h
/*
    This file is part of Corrade.

    Copyright © 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016,
                2017 Vladimír Vondruš <mosra@centrum.cz>

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
 * @brief Class @ref Corrade::TestSuite::Tester, macros @ref CORRADE_TEST_MAIN(), @ref CORRADE_VERIFY(), @ref CORRADE_COMPARE(), @ref CORRADE_COMPARE_AS(), @ref CORRADE_COMPARE_WITH(), @ref CORRADE_EXPECT_FAIL(), @ref CORRADE_EXPECT_FAIL_IF(), @ref CORRADE_SKIP(), @ref CORRADE_BENCHMARK()
 */

#include <initializer_list>
#include <iosfwd>
#include <string>
#include <type_traits>
#include <vector>

#include "Corrade/TestSuite/Comparator.h"
#include "Corrade/TestSuite/Compare/FloatingPoint.h"
#include "Corrade/TestSuite/visibility.h"
#include "Corrade/Utility/Debug.h"
#include "Corrade/Utility/Macros.h"

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

## Basic testing workflow

A test starts with deriving the @ref Tester class. The test cases are
parameter-less `void` member functions that are added using @ref addTests() in
the constructor and the `main()` function is created using
@ref CORRADE_TEST_MAIN(). The goal is to have as little boilerplate as
possible, thus the test usually  consists of only one `*.cpp` file with no
header and the derived type is a `struct` to avoid having to write `public`
keywords.

@snippet testsuite-basic.cpp 0

The above gives the following output:

@image html testsuite-basic.png

Actual testing is done via various @ref CORRADE_VERIFY(), @ref CORRADE_COMPARE(),
@ref CORRADE_COMPARE_AS() and @ref CORRADE_COMPARE_WITH() macros. If some
comparison in given test case fails, a `FAIL` with concrete file, line and
additional diagnostic is printed to the output and the test case is exited
without executing the remaining statements. Otherwise, if all comparisons in
given test case pass, an `OK` is printed. The main difference between these
macros is the kind of diagnostic output they print when comparison fails --
for example a simple expression failure reported by @ref CORRADE_VERIFY() is
enough when checking for non-`nullptr` value, but for comparing a 1000-element
array you might want to use @ref CORRADE_COMPARE_AS() with @ref Compare::Container
instead.

Additionally there are @ref CORRADE_SKIP(), @ref CORRADE_EXPECT_FAIL() and
@ref CORRADE_EXPECT_FAIL_IF() control flow helpers that allow you to say for
example that a particular test was skipped due to missing functionality on
given platform (printing a `SKIP` on output and exiting the test case right
after the statement) or documenting that some algorithm produces incorrect
result due to a bug, printing an `XFAIL`. Passing a test while failure is
expected is treated as an error (`XPASS`), which can be helpful to ensure the
assumptions in the tests don't get stale. Expected failures can be also
disabled globally via a command-line option `--no-xfail` or via environment
variable, @ref TestSuite-Tester-command-line "see below".

The only reason why those are macros and not member functions is the ability to
gather class/function/file/line/expression information via the preprocessor for
printing the test output and exact location of possible test failure. If none
of these macros is encountered when running the test case, the test case is
reported as invalid, with `?` on output.

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

@image html testsuite-templated.png

This works with all `add*()` functions though please note that current C++11
compilers (at least GCC and Clang) are not able to properly detect class type
when passing only templated functions to it, so you may need to specify the
type explicitly. Also, there is no easy and portable way to get function name
with template parameters so by default it will be just the function name, but
you can call @ref setTestCaseName() to specify a full name.

## Instanced tests

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

@image html testsuite-instanced.png

The tester class just gives you an instance index via @ref testCaseInstanceId()
and it's up to you whether you use it as an offset to some data array or
generate an input using it, the above example is just a hint how one might use
it. Each instance is printed to the output separately and if one instance
fails, it doesn't stop the other instances from being executed. Similarly to
the templated tests, @ref setTestCaseDescription() allows you to set a
human-readable description of given instance. If not called, the instances are
just numbered in the output.

## Repeated tests

A complementary feature to instanced tests are repeated tests using
@ref addRepeatedTests(), useful for example to repeatedly call one function
10000 times to increase probability of potential race conditions. The
difference from instanced tests is that all repeats are treated as executing
the same code and thus only the overall result is reported in the output. Also
unlike instanced tests, if a particular repeat fails, no further repeats are
executed. The test output contains number of executed repeats after the test
case name, prefixed by `@`. Example of testing race conditions with multiple
threads accessing the same variable:

@snippet testsuite-repeated.cpp 0

Depending on various factors, here is one possible output:

@image html testsuite-repeated.png

Similarly to @ref testCaseInstanceId() there is @ref testCaseRepeatId() which
gives repeat index. Use with care, however, as the repeated tests are assumed
to execute the same code every time. On
@ref TestSuite-Tester-command-line "the command-line" it is possible to
increase repeat count via `--repeat-every`. In addition there is `--repeat-all`
which behaves as like all `add*()` functions in the constructor were called
multiple times in a loop. Combined with `--shuffle` this can be used to run the
test cases multiple times in a random order to uncover potential unwanted
interactions and order-dependent bugs.

It's also possible to combine instanced and repeated tests using
@ref addRepeatedInstancedTests().

## Benchmarks

Besides verifying code correctness, it's possible to measure code performance.
Unlike correctness tests, the benchmark results are hard to reason about using
only automated means, so there are no macros for verifying benchmark results
and instead the measured values are just printed to the output for users to
see. Benchmarks can be added using @ref addBenchmarks(), the actual benchmark
loop is marked by @ref CORRADE_BENCHMARK() and the results are printed to
output with `BENCH` identifier. Example benchmark comparing performance of
inverse square root implementations:

@snippet testsuite-benchmark.cpp 0

Note that it's not an error to add one test/benchmark multiple times -- here it
is used to have the same code benchmarked with different timers. Possible
output:

@image html testsuite-benchmark.png

The number passed to @ref addBenchmarks() is equivalent to repeat count passed
to @ref addRepeatedTests() and specifies measurement sample count. The number
passed to @ref CORRADE_BENCHMARK() is number of iterations of the inner loop in
one sample measurement to amortize the overhead and error caused by clock
precision -- the faster the measured code is the more iterations it needs. The
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

## Custom benchmarks

It's possible to specify a custom pair of functions for intiating the benchmark
and returning the result using @ref addCustomBenchmarks(). The benchmark end
function returns an unsigned 64-bit integer indicating measured amount in units
given by @ref BenchmarkUnits. To further describe the value being measured you
can call @ref setBenchmarkName() in the benchmark begin function. Contrived
example of benchmarking number of copies when using `std::vector::push_back()`:

@snippet testsuite-benchmark-custom.cpp 0

Running the benchmark shows that adding calling `push_back()` for 10 thousand
elements actually causes the copy constructor to be called 26 thousand times:

@image html testsuite-benchmark-custom.png

## Specifying setup/teardown routines

While the common practice in C++ is to use RAII for resource lifetime
management, sometimes you may need to execute arbitrary code at the beginning
and end of each test case. For this, all @ref addTests(),
@ref addInstancedTests(), @ref addRepeatedTests(), @ref addRepeatedInstancedTests(),
@ref addBenchmarks(), @ref addInstancedBenchmarks(), @ref addCustomBenchmarks()
and @ref addCustomInstancedBenchmarks() have an overload that is additionally
taking a pair of parameter-less `void` functions for setup and teardown. Both
functions are called before and after each test case run, independently on
whether the test case passed or failed.

@anchor TestSuite-Tester-command-line
## Command-line options

Command-line options that make sense to be set globally for multiple test cases
are also configurable via environment variables for greater flexibility when
for example running the tests in a batch via `ctest`.

Usage:

    ./my-test [-h|--help] [-c|--color on|off|auto] [--skip "N1 N2..."]
        [--skip-tests] [--skip-benchmarks] [--only "N1 N2..."] [--shuffle]
        [--repeat-every N] [--repeat-all N] [--abort-on-fail] [--no-xfail]
        [--benchmark TYPE] [--benchmark-discard N] [--benchmark-yellow N]
        [--benchmark-red N]

Arguments:

-   `-h`, `--help` -- display this help message and exit
-   `-c`, `--color on|off|auto` -- colored output (environment:
    `CORRADE_TEST_COLOR`, default: `auto`). The `auto` option enables color
    output in case an interactive terminal is detected. Note that on Windows it
    is possible to output colors only directly to an interactive terminal
    unless @ref CORRADE_UTILITY_USE_ANSI_COLORS is defined.
-   `--skip "N1 N2..."` -- skip test cases with given numbers
-   `--skip-tests` -- skip all tests (environment:
    `CORRADE_TEST_SKIP_TESTS=ON|OFF`)
-   `--skip-benchmarks` -- skip all benchmarks (environment:
    `CORRADE_TEST_SKIP_BENCHMARKS=ON|OFF`)
-   `--only "N1 N2..."` -- run only test cases with given numbers
-   `--shuffle` -- randomly shuffle test case order (environment:
    `CORRADE_TEST_SHUFFLE=ON|OFF`)
-   `--repeat-every N` -- repeat every test case N times (environment:
    `CORRADE_TEST_REPEAT_EVERY`, default: `1`)
-   `--repeat-all N` -- repeat all test cases N times (environment:
    `CORRADE_TEST_REPEAT_ALL`, default: `1`)
-   `--abort-on-fail` -- abort after first failure (environment:
    `CORRADE_TEST_ABORT_ON_FAIL=ON|OFF`)
-   `--no-xfail` -- disallow expected failures (environment:
    `CORRADE_TEST_NO_XFAIL=ON|OFF`)
-   `--benchmark TYPE` -- default benchmark type (environment:
    `CORRADE_BENCHMARK`). Supported benchmark types:
    -   `wall-time` -- wall time spent
    -   `cpu-time` -- CPU time spent
    -   `cpu-cycles` -- CPU cycles spent (x86 only, gives zero result
        elsewhere)
-   `--benchmark-discard N` -- discard first N measurements of each benchmark
    (environment: `CORRADE_BENCHMARK_DISCARD`, default: `1`)
-   `--benchmark-yellow N` -- deviation threshold for marking benchmark yellow
    (environment: `CORRADE_BENCHMARK_YELLOW`, default: `0.05`)
-   `--benchmark-red N` -- deviation threshold for marking benchmark red
    (environment: `CORRADE_BENCHMARK_RED`, default: `0.25`)

## CMake support and platform-specific goodies

While the test executables can be created in any way you want, there's also an
@ref corrade-cmake-add-test "corrade_add_test()" CMake macro that creates the
executable, links `Corrade::TestSuite` library to it and adds it to CTest.
Besides that it is able to link other arbitrary libraries to the executable
and specify a list of files that the tests used. It provides additional useful
features on various platforms:

-   If compiling for Emscripten, using @ref corrade-cmake-add-test "corrade_add_test()"
    makes CTest run the resulting `*.js` file via Node.js. Also it is able to
    bundle all files specified in `FILES` into the virtual Emscripten
    filesystem, making it easy to run file-based tests on this platform; all
    environment options are passed through as well.
-   If Xcode projects are generated via CMake and @ref CORRADE_TESTSUITE_TARGET_XCTEST
    is enabled, @ref corrade-cmake-add-test "corrade_add_test()" makes the test
    executables in a way compatible with XCTest, making it easy to run them
    directly from Xcode. Running the tests via `ctest` will also use XCTest.
-   If building for Android, using @ref corrade-cmake-add-test "corrade_add_test()"
    will make CTest upload the test executables and all files specified in `FILES`
    onto the device or emulator via `adb`, run it there with all environment
    options passed through as well and transfers test results back to the host.

Example of using the @ref corrade-cmake-add-test "corrade_add_test()" macro is
below. The test executable will get built from the specified source with the
libJPEG library linked and the `*.jpg` files will be available on desktop,
Emscripten and Android in path specified in `JPEG_TEST_DIR` that was saved into
the `configure.h` file inside current build directory:

@snippet testsuite.cmake 0
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
                explicit TesterConfiguration();

                /** @brief Skipped argument prefixes */
                const std::vector<std::string>& skippedArgumentPrefixes() const {
                    return _skippedArgumentPrefixes;
                }

                /**
                 * @brief Set skipped argument prefixes
                 *
                 * Useful to allow passing command-line arguments elsewhere
                 * without having the tester complaining about them.
                 * @see @ref arguments()
                 */
                TesterConfiguration& setSkippedArgumentPrefixes(std::initializer_list<std::string> prefixes) {
                    _skippedArgumentPrefixes.insert(_skippedArgumentPrefixes.end(), prefixes);
                    return *this;
                }

            private:
                std::vector<std::string> _skippedArgumentPrefixes;
        };

        /**
         * @brief Alias for debug output
         *
         * For convenient debug output inside test cases (instead of using
         * fully qualified name):
         * @code
         * void myTestCase() {
         *     int a = 4;
         *     Debug() << a;
         *     CORRADE_COMPARE(a + a, 8);
         * }
         * @endcode
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
         * @see @ref addBenchmarks(), @ref addInstancedBenchmarks()
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

            #ifdef CORRADE_BUILD_DEPRECATED
            /** @copybrief BenchmarkType::WallTime
             * @deprecated Use @ref BenchmarkType::WallTime instead.
             */
            WallClock CORRADE_DEPRECATED_ENUM("use BenchmarkType::WallTime instead") = int(WallTime),
            #endif

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
         * @see @ref addCustomBenchmarks(), @ref addCustomInstancedBenchmarks()
         */
        enum class BenchmarkUnits {
            /* Values should not overlap with BenchmarkType */

            Nanoseconds = 100,      /**< Time in nanoseconds */

            #ifdef CORRADE_BUILD_DEPRECATED
            /** @copybrief BenchmarkUnits::Nanoseconds
             * @deprecated Use @ref BenchmarkUnits::Nanoseconds instead.
             */
            Time CORRADE_DEPRECATED_ENUM("use Nanoseconds instead") = int(Nanoseconds),
            #endif

            Cycles = 101,           /**< Processor cycle count */
            Instructions = 102,     /**< Processor instruction count */
            Bytes = 103,            /**< Memory (in bytes) */

            #ifdef CORRADE_BUILD_DEPRECATED
            /** @copybrief BenchmarkUnits::Bytes
             * @deprecated Use @ref BenchmarkUnits::Bytes instead.
             */
            Memory CORRADE_DEPRECATED_ENUM("use Bytes instead") = int(Bytes),
            #endif

            Count = 104             /**< Generic count */
        };

        /**
         * @brief Command-line arguments
         *
         * Populated by @ref CORRADE_TEST_MAIN().
         */
        std::pair<int&, char**> arguments() { return {*_argc, _argv}; }

        /**
         * @brief Constructor
         * @param configuration     Optional configuration
         */
        explicit Tester(TesterConfiguration configuration = TesterConfiguration{});

        /**
         * @brief Add test cases
         *
         * Adds one or more test cases to be executed. It's not an error to
         * call this function multiple times or add one test case more than
         * once.
         * @see @ref addInstancedTests()
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
         * particular test case more than once -- in that case it will appear
         * in the output log once for each occurence in the list.
         * @see @ref addInstancedTests(), @ref addRepeatedInstancedTests()
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
         * @see @ref addInstancedTests()
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
         * multiple times or add a particular test case more than once -- in
         * that case it will appear in the output log once for each occurence
         * in the list.
         * @see @ref addInstancedTests(), @ref addRepeatedInstancedTests()
         */
        template<class Derived> void addRepeatedTests(std::initializer_list<void(Derived::*)()> tests, std::size_t repeatCount, void(Derived::*setup)(), void(Derived::*teardown)()) {
            _testCases.reserve(_testCases.size() + tests.size());
            for(auto test: tests)
                _testCases.emplace_back(~std::size_t{}, repeatCount, static_cast<TestCase::Function>(test), static_cast<TestCase::Function>(setup), static_cast<TestCase::Function>(teardown));
        }

        /**
         * @brief Add instanced test cases
         *
         * Unlike @ref addTests(), this function runs each of the test cases
         * @p instanceCount times. Useful for data-driven tests. Each test case
         * appears in the output once for each instance. It's not an error to
         * call this function multiple times or add one test case more than
         * once -- in that case it will appear once for each instance of each
         * occurence in the list.
         * @see @ref testCaseInstanceId(), @ref setTestCaseDescription()
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
         * times or add one test case more than once -- in that case it will
         * appear once for each instance of each occurence in the list.
         * @see @ref addInstancedTests(), @ref addRepeatedInstancedTests()
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
         * this function multiple times or add one test case more than once --
         * in that case it will appear once for each instance of each occurence
         * in the list.
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
         * than once -- in that case it will appear once for each instance of
         * each occurence in the list.
         * @see @ref addInstancedTests(), @ref addRepeatedInstancedTests()
         */
        template<class Derived> void addRepeatedInstancedTests(std::initializer_list<void(Derived::*)()> tests, std::size_t repeatCount, std::size_t instanceCount, void(Derived::*setup)(), void(Derived::*teardown)()) {
            _testCases.reserve(_testCases.size() + tests.size());
            for(auto test: tests) for(std::size_t i = 0; i != instanceCount; ++i)
                _testCases.emplace_back(i, repeatCount, static_cast<TestCase::Function>(test), static_cast<TestCase::Function>(setup), static_cast<TestCase::Function>(teardown));
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
         * @see @ref addInstancedBenchmarks()
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
         * @see @ref addInstancedBenchmarks()
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
         * @see @ref addCustomInstancedBenchmarks()
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
         * @see @ref addCustomInstancedBenchmarks()
         */
        template<class Derived> void addCustomBenchmarks(std::initializer_list<void(Derived::*)()> benchmarks, std::size_t batchCount, void(Derived::*setup)(), void(Derived::*teardown)(), void(Derived::*benchmarkBegin)(), std::uint64_t(Derived::*benchmarkEnd)(), BenchmarkUnits benchmarkUnits) {
            _testCases.reserve(_testCases.size() + benchmarks.size());
            for(auto benchmark: benchmarks)
                _testCases.emplace_back(~std::size_t{}, batchCount, static_cast<TestCase::Function>(benchmark), static_cast<TestCase::Function>(setup), static_cast<TestCase::Function>(teardown), static_cast<TestCase::BenchmarkBegin>(benchmarkBegin), static_cast<TestCase::BenchmarkEnd>(benchmarkEnd), TestCaseType(int(benchmarkUnits)));
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
         * benchmark more than once -- in that case it will appear once for
         * each instance of each occurence in the list.
         * @see @ref testCaseInstanceId(), @ref setTestCaseDescription()
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
         * once -- in that case it will appear once for each instance of each
         * occurence in the list.
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
         * multiple times or add one benchmark more than once -- in that case
         * it will appear once for each instance of each occurence in the list.
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
         * multiple times or add one benchmark more than once -- in that case
         * it will appear once for each instance of each occurence in the list.
         */
        template<class Derived> void addCustomInstancedBenchmarks(std::initializer_list<void(Derived::*)()> benchmarks, std::size_t batchCount, std::size_t instanceCount, void(Derived::*setup)(), void(Derived::*teardown)(), void(Derived::*benchmarkBegin)(), std::uint64_t(Derived::*benchmarkEnd)(), BenchmarkUnits benchmarkUnits) {
            _testCases.reserve(_testCases.size() + benchmarks.size());
            for(auto benchmark: benchmarks) for(std::size_t i = 0; i != instanceCount; ++i)
                _testCases.emplace_back(i, batchCount, static_cast<TestCase::Function>(benchmark), static_cast<TestCase::Function>(setup), static_cast<TestCase::Function>(teardown), static_cast<TestCase::BenchmarkBegin>(benchmarkBegin), static_cast<TestCase::BenchmarkEnd>(benchmarkEnd), TestCaseType(int(benchmarkUnits)));
        }

        /**
         * @brief Test case ID
         *
         * Returns ID of the test case that is currently executing, starting
         * from `1`. Value is undefined if called  outside of test cases and
         * setup/teardown functions.
         */
        std::size_t testCaseId() const { return _testCaseId; }

        /**
         * @brief Test case instance ID
         *
         * Returns instance ID of the instanced test case that is currently
         * executing, starting from `0`. Value is undefined if called outside
         * of *instanced* test cases and setup/teardown functions.
         * @see @ref addInstancedTests()
         */
        std::size_t testCaseInstanceId() const { return _testCaseInstanceId; }

        /**
         * @brief Test case repeat ID
         *
         * Returns repeat ID of the repeated test case that is currently
         * executing, starting from `0`. Value is undefined if called outside
         * of *repeated* test cases and setup/teardown functions.
         */
        std::size_t testCaseRepeatId() const { return _testCaseRepeatId; }

        /**
         * @brief Set custom test case name
         *
         * By default the test case name is gathered in the check macros and is
         * equivalent to the following:
         * @code
         * setTestCaseName(__func__);
         * @endcode
         */
        void setTestCaseName(const std::string& name);
        void setTestCaseName(std::string&& name); /**< @overload */

        /**
         * @brief Set test case description
         *
         * Additional text displayed after the test case name. By default
         * the description is empty for non-instanced test cases and instance
         * ID for instanced test cases.
         */
        void setTestCaseDescription(const std::string& description);
        void setTestCaseDescription(std::string&& description); /**< @overload */

        /**
         * @brief Set benchmark name
         *
         * In case of @ref addCustomBenchmarks() and @ref addCustomInstancedBenchmarks()
         * provides the name for the unit measured, for example `"wall time"`.
         */
        void setBenchmarkName(const std::string& name);
        void setBenchmarkName(std::string&& name); /**< @overload */

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

        /* Called from CORRADE_TEST_MAIN() */
        int exec();

        /* Overload needed for testing */
        int exec(std::ostream* logOutput, std::ostream* errorOutput);

        /* Compare two identical types without explicit type specification */
        template<class T> void compare(const std::string& actual, const T& actualValue, const std::string& expected, const T& expectedValue) {
            compareAs<T, T, T>(actual, actualValue, expected, expectedValue);
        }

        /* Compare two different types without explicit type specification */
        template<class Actual, class Expected> void compare(const std::string& actual, const Actual& actualValue, const std::string& expected, const Expected& expectedValue) {
            compareAs<typename Implementation::CommonType<Actual, Expected>::Type, Actual, Expected>(actual, actualValue, expected, expectedValue);
        }

        /* Compare two different types with explicit templated type
           specification (e.g. Compare::Containers). This allows the user to
           call only `CORRADE_COMPARE_AS(a, b, Compare::Containers)` without
           explicitly specifying the type, e.g.
           `CORRADE_COMPARE_AS(a, b, Compare::Containers<std::vector<int>>)` */
        template<template<class> class T, class Actual, class Expected> void compareAs(const std::string& actual, const Actual& actualValue, const std::string& expected, const Expected& expectedValue) {
            compareAs<T<typename Implementation::CommonType<Actual, Expected>::Type>, Actual, Expected>(actual, actualValue, expected, expectedValue);
        }

        /* Compare two different types with explicit type specification */
        template<class T, class U, class V> void compareAs(const std::string& actual, const U& actualValue, const std::string& expected, const V& expectedValue) {
            compareWith(Comparator<T>(), actual, actualValue, expected, expectedValue);
        }

        /* Compare two different types with explicit comparator specification */
        template<class T, class U, class V> void compareWith(Comparator<T> comparator, const std::string& actual, const U& actualValue, const std::string& expected, const V& expectedValue);

        template<class T> void verify(const std::string& expression, T&& value);

        /* Called from CORRADE_TEST_MAIN() */
        void registerTest(std::string filename, std::string name);

        /* Called from CORRADE_SKIP() */
        void skip(const std::string& message);

    #ifndef DOXYGEN_GENERATING_OUTPUT
    protected:
    #endif
        class CORRADE_TESTSUITE_EXPORT ExpectedFailure {
            public:
                explicit ExpectedFailure(Tester& instance, std::string message, bool enabled = true);

                /* For types with explicit bool conversion */
                template<class T> explicit ExpectedFailure(Tester& instance, std::string message, T&& enabled): ExpectedFailure{instance, message, enabled ? true : false} {}

                ~ExpectedFailure();

                std::string message() const;

            private:
                Tester& _instance;
                std::string _message;
        };

        /* Called from all CORRADE_*() verification/skip/xfail macros through
           _CORRADE_REGISTER_TEST_CASE() */
        void registerTestCase(std::string&& name, int line);

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

            explicit TestCase(std::size_t instanceId, std::size_t repeatCount, Function test, Function setup, Function teardown): instanceId{instanceId}, repeatCount{repeatCount}, test{test}, setup{setup}, teardown{teardown}, benchmarkBegin{}, benchmarkEnd{}, type{TestCaseType::Test} {}

            explicit TestCase(std::size_t instanceId, std::size_t repeatCount, Function test, Function setup, Function teardown, BenchmarkBegin benchmarkBegin, BenchmarkEnd benchmarkEnd, TestCaseType type): instanceId{instanceId}, repeatCount{repeatCount}, test{test}, setup{setup}, teardown{teardown}, benchmarkBegin{benchmarkBegin}, benchmarkEnd{benchmarkEnd}, type{type} {}

            std::size_t instanceId, repeatCount;
            Function test, setup, teardown;
            BenchmarkBegin benchmarkBegin;
            BenchmarkEnd benchmarkEnd;
            TestCaseType type;
        };

    #ifndef DOXYGEN_GENERATING_OUTPUT
    protected:
    #endif
        class CORRADE_TESTSUITE_EXPORT BenchmarkRunner {
            public:
                explicit BenchmarkRunner(Tester& instance, TestCase::BenchmarkBegin begin, TestCase::BenchmarkEnd end): _instance(instance), _end{end} {
                    (_instance.*begin)();
                }

                ~BenchmarkRunner() {
                    _instance._benchmarkResult = (_instance.*_end)();
                }

                const char* begin() const { return nullptr; }
                const char* end() const { return reinterpret_cast<char*>(_instance._benchmarkBatchSize); }

            private:
                Tester& _instance;
                TestCase::BenchmarkEnd _end;
        };

        /* Called from CORRADE_BENCHMARK() */
        BenchmarkRunner createBenchmarkRunner(std::size_t batchSize);

    private:
        static int* _argc;
        static char** _argv;

        void verifyInternal(const std::string& expression, bool value);
        void printTestCaseLabel(Debug& out, const char* status, Debug::Color statusColor, Debug::Color labelColor);

        void wallTimeBenchmarkBegin();
        std::uint64_t wallTimeBenchmarkEnd();

        void cpuTimeBenchmarkBegin();
        std::uint64_t cpuTimeBenchmarkEnd();

        void cpuCyclesBenchmarkBegin();
        std::uint64_t cpuCyclesBenchmarkEnd();

        Debug::Flags _useColor;
        std::ostream *_logOutput, *_errorOutput;
        std::vector<TestCase> _testCases;
        std::string _testFilename, _testName, _testCaseName,
            _testCaseDescription, _benchmarkName, _expectFailMessage;
        std::size_t _testCaseId, _testCaseInstanceId, _testCaseRepeatId,
            _benchmarkBatchSize, _testCaseLine, _checkCount;

        std::uint64_t _benchmarkBegin;
        std::uint64_t _benchmarkResult;
        TestCase* _testCase = nullptr;
        bool _expectedFailuresDisabled;
        ExpectedFailure* _expectedFailure;
        TesterConfiguration _configuration;
};

/** @hideinitializer
@brief Create `main()` function for given Tester subclass

Populates @ref Corrade::TestSuite::Tester::arguments() "Tester::arguments()",
instantiates @p Class, executes the test cases and returns from `main()` with
code based on the test results. This macro has to be used outside of any
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
        t.registerTest(__FILE__, #Class);                                   \
        return t.exec();                                                    \
    }
#else
#define CORRADE_TEST_MAIN(Class)                                            \
    int main(int argc, char** argv) {                                       \
        Corrade::TestSuite::Tester::registerArguments(argc, argv);          \
        Class t;                                                            \
        t.registerTest(__FILE__, #Class);                                   \
        return t.exec();                                                    \
    }
#endif

#ifndef DOXYGEN_GENERATING_OUTPUT
#ifndef CORRADE_TARGET_ANDROID
#define _CORRADE_REGISTER_TEST_CASE()                                       \
    Tester::registerTestCase(__func__, __LINE__);
#else
/* C++11 standard __func__ on Android behaves like GCC's __PRETTY_FUNCTION__,
   while GCC's __FUNCTION__ does the right thing.. I wonder -- do they have
   *any* tests for libc at all?! */
#define _CORRADE_REGISTER_TEST_CASE()                                       \
    Tester::registerTestCase(__FUNCTION__, __LINE__);
#endif
#endif

/** @hideinitializer
@brief Verify an expression in @ref Corrade::TestSuite::Tester "Tester" subclass

If the expression is not true, the expression is printed and execution of given
test case is terminated. Example usage:
@code
string s("hello");
CORRADE_VERIFY(!s.empty());
@endcode

It is possible to use @ref CORRADE_VERIFY() also on objects with *explicit*
`operator bool` without doing explicit conversion (e.g. using `!!`), for
example:
@code
std::unique_ptr<T> t(new T);
CORRADE_VERIFY(t);
@endcode

@see @ref CORRADE_COMPARE(), @ref CORRADE_COMPARE_AS()
*/
#define CORRADE_VERIFY(expression)                                          \
    do {                                                                    \
        _CORRADE_REGISTER_TEST_CASE();                                      \
        Tester::verify(#expression, expression);                            \
    } while(false)

/** @hideinitializer
@brief Compare two values in @ref Corrade::TestSuite::Tester "Tester" subclass

If the values are not the same, they are printed for comparison and execution
of given test case is terminated. Example usage:
@code
int a = 5 + 3;
CORRADE_COMPARE(a, 8);
@endcode

Comparison of floating-point types is by default done as a fuzzy-compare, see
@ref Corrade::TestSuite::Comparator<float> "Comparator<float>" and
@ref Corrade::TestSuite::Comparator<double> "Comparator<double>" for details.

Note that this macro is usable only if given type implements equality
comparison operators and is printable via @ref Corrade::Utility::Debug "Utility::Debug".
@see @ref CORRADE_VERIFY(), @ref CORRADE_COMPARE_AS()
*/
#define CORRADE_COMPARE(actual, expected)                                   \
    do {                                                                    \
        _CORRADE_REGISTER_TEST_CASE();                                      \
        Tester::compare(#actual, actual, #expected, expected);              \
    } while(false)

/** @hideinitializer
@brief Compare two values in @ref Corrade::TestSuite::Tester "Tester" subclass with explicitly specified type

If the values are not the same, they are printed for comparison and execution
of given test case is terminated. Example usage:
@code
CORRADE_COMPARE_AS(std::sin(0.0), 0.0f, float);
@endcode
See also @ref Corrade::TestSuite::Comparator "Comparator" class documentation
for example of more involved comparisons.

Note that this macro is usable only if the type passed to it is printable via
@ref Corrade::Utility::Debug "Utility::Debug" and is convertible to / usable
with given comparator type.
@see @ref CORRADE_VERIFY(), @ref CORRADE_COMPARE(), @ref CORRADE_COMPARE_WITH()
*/
#ifdef DOXYGEN_GENERATING_OUTPUT
#define CORRADE_COMPARE_AS(actual, expected, Type...)
#else
#define CORRADE_COMPARE_AS(actual, expected, ...)                           \
    do {                                                                    \
        _CORRADE_REGISTER_TEST_CASE();                                      \
        Tester::compareAs<__VA_ARGS__>(#actual, actual, #expected, expected); \
    } while(false)
#endif

/** @hideinitializer
@brief Compare two values in @ref Corrade::TestSuite::Tester "Tester" subclass with explicitly specified comparator

If the values are not the same, they are printed for comparison and execution
of given test case is terminated. Example usage:
@code
CORRADE_COMPARE_WITH("actual.txt", "expected.txt", Compare::File("/common/path/prefix"));
@endcode
See @ref Corrade::TestSuite::Comparator "Comparator" class documentation for
more information.

Note that this macro is usable only if the type passed to it is printable via
@ref Corrade::Utility::Debug "Utility::Debug" and is usable with given
comparator type.
@see @ref CORRADE_VERIFY(), @ref CORRADE_COMPARE(), @ref CORRADE_COMPARE_AS()
*/
#define CORRADE_COMPARE_WITH(actual, expected, comparatorInstance)          \
    do {                                                                    \
        _CORRADE_REGISTER_TEST_CASE();                                      \
        Tester::compareWith(comparatorInstance.comparator(), #actual, actual, #expected, expected); \
    } while(false)

/** @hideinitializer
@brief Expect failure in all following checks in the same scope
@param message Message which will be printed into output as indication of
    expected failure

Expects failure in all following @ref CORRADE_VERIFY(), @ref CORRADE_COMPARE()
and @ref CORRADE_COMPARE_AS() checks in the same scope. In most cases it will
be until the end of the function, but you can limit the scope by placing
relevant checks in a separate block:
@code
{
    CORRADE_EXPECT_FAIL("Not implemented.");
    CORRADE_VERIFY(isFutureClear());
}

int i = 6*7;
CORRADE_COMPARE(i, 42);
@endcode
If any of the following checks passes, an error will be printed to output.
@see @ref CORRADE_EXPECT_FAIL_IF()
*/
#define CORRADE_EXPECT_FAIL(message)                                        \
    Tester::ExpectedFailure _CORRADE_HELPER_PASTE(expectedFailure, __LINE__)(*this, message)

/** @hideinitializer
@brief Conditionally expect failure in all following checks in the same scope
@param message      Message which will be printed into output as indication of
    expected failure
@param condition    The failure is expected only if the condition evaluates to
    `true`

With @ref CORRADE_EXPECT_FAIL() it's not possible to write code such as this,
because the scope of expected failure will end at the end of the `if` block:
@code
{
    if(answer != 42)
        CORRADE_EXPECT_FAIL("This is not our universe.");

    CORRADE_VERIFY(6*7, 49); // always fails
}
@endcode

The solution is to use `CORRADE_EXPECT_FAIL_IF()`:
@code
{
    CORRADE_EXPECT_FAIL_IF(answer != 42, "This is not our universe.");

    CORRADE_VERIFY(6*7, 49); // expect the failure if answer is not 42
}
@endcode

Similarly to @ref CORRADE_VERIFY(), it is possible to use
@ref CORRADE_EXPECT_FAIL_IF() also on objects with *explicit* `operator bool`
without doing explicit conversion (e.g. using `!!`).
*/
#define CORRADE_EXPECT_FAIL_IF(condition, message)                          \
    Tester::ExpectedFailure _CORRADE_HELPER_PASTE(expectedFailure, __LINE__)(*this, message, condition)

/** @hideinitializer
@brief Skip test case
@param message Message which will be printed into output as indication of
    skipped test

Skips all following checks in given test case. Useful for e.g. indicating that
given feature can't be tested on given platform:
@code
if(!bigEndian) {
    CORRADE_SKIP("Big endian compatibility can't be tested on this system.");
}
@endcode
*/
#define CORRADE_SKIP(message)                                               \
    do {                                                                    \
        _CORRADE_REGISTER_TEST_CASE();                                      \
        Tester::skip(message);                                              \
    } while(false)

/** @hideinitializer
@brief Run a benchmark

Benchmarks the following block or expression by measuring @p batchSize
iterations of given block. Use in conjunction with
@ref Corrade::TestSuite::Tester::addBenchmarks() "addBenchmarks()" and others.
Only one such loop can be in a function to achieve proper result. Please note
that there need to be additional measures in order to prevent the optimizer
from removing the benchmark code such as assigning to a `volatile` variable or
combining all the results to a variable, which is then being used outside of
the loop.
@code
void benchmark() {
    std::string a = "hello", b = "world";
    CORRADE_BENCHMARK(1000) {
        volatile std::string c = a + b;
    }
}
@endcode
The resulting measured value is divided by @p batchSize to represent cost of
one iteration.
*/
#define CORRADE_BENCHMARK(batchSize)                                        \
    _CORRADE_REGISTER_TEST_CASE();                                          \
    for(CORRADE_UNUSED auto&& _CORRADE_HELPER_PASTE(benchmarkIteration, __func__): Tester::createBenchmarkRunner(batchSize))

template<class T, class U, class V> void Tester::compareWith(Comparator<T> comparator, const std::string& actual, const U& actualValue, const std::string& expected, const V& expectedValue) {
    ++_checkCount;

    /* Store (references to) possibly implicitly-converted values,
       otherwise the implicit conversion would when passing them to operator(),
       causing dead memory access later in printErrorMessage() */
    const typename Implementation::ComparatorTraits<T>::ActualType& actualValueInExpectedActualType = actualValue;
    const typename Implementation::ComparatorTraits<T>::ExpectedType& expectedValueInExpectedExpectedType = expectedValue;

    /* If the comparison succeeded or the failure is expected, done */
    bool equal = comparator(actualValueInExpectedActualType, expectedValueInExpectedExpectedType);
    if(!_expectedFailure) {
        if(equal) return;
    } else if(!equal) {
        Debug out{_logOutput, _useColor};
        printTestCaseLabel(out, " XFAIL", Debug::Color::Yellow, Debug::Color::Default);
        out << "at" << _testFilename << "on line"
            << _testCaseLine << Debug::newline << "       " << _expectedFailure->message()
            << actual << "and" << expected << "failed the comparison.";
        return;
    }

    /* Otherwise print message to error output and throw exception */
    Error out{_errorOutput, _useColor};
    printTestCaseLabel(out, _expectedFailure ? " XPASS" : "  FAIL", Debug::Color::Red, Debug::Color::Default);
    out << "at" << _testFilename << "on line"
        << _testCaseLine << Debug::newline << "       ";
    if(!_expectedFailure) comparator.printErrorMessage(out, actual, expected);
    else out << actual << "and" << expected << "were expected to fail the comparison.";
    throw Exception();
}

template<class T> void Tester::verify(const std::string& expression, T&& value) {
    if(value) verifyInternal(expression, true);
    else verifyInternal(expression, false);
}

}}

#endif
