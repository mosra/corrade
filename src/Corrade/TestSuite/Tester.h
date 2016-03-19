#ifndef Corrade_TestSuite_Tester_h
#define Corrade_TestSuite_Tester_h
/*
    This file is part of Corrade.

    Copyright © 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016
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
 * @brief Class @ref Corrade::TestSuite::Tester, macros @ref CORRADE_TEST_MAIN(), @ref CORRADE_VERIFY(), @ref CORRADE_COMPARE(), @ref CORRADE_COMPARE_AS(), @ref CORRADE_COMPARE_WITH(), @ref CORRADE_SKIP()
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
@brief Base class for unit tests

See @ref unit-testing for introduction.

@see @ref CORRADE_TEST_MAIN(), @ref CORRADE_VERIFY(), @ref CORRADE_COMPARE(),
    @ref CORRADE_COMPARE_AS(), @ref CORRADE_COMPARE_WITH(), @ref CORRADE_SKIP(),
    @ref CORRADE_EXPECT_FAIL(), @ref CORRADE_EXPECT_FAIL_IF()
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
         * @brief Constructor
         * @param configuration     Optional configuration
         */
        explicit Tester(const TesterConfiguration& configuration = TesterConfiguration{});

        /**
         * @brief Execute the tester
         * @param argc          Main function argument count
         * @param argv          Main function argument values
         * @param logOutput     Output stream for log messages
         * @param errorOutput   Output stream for error messages
         * @return Non-zero if there are no test cases, if any test case fails
         *      or doesn't contain any checking macros, zero otherwise.
         */
        int exec(int argc, const char** argv, std::ostream* logOutput, std::ostream* errorOutput);

        /** @overload */
        int exec(int argc, char** argv, std::ostream* logOutput, std::ostream* errorOutput) {
            return exec(argc, const_cast<const char**>(argv), logOutput, errorOutput);
        }

        /** @overload */
        int exec(int argc, std::nullptr_t argv, std::ostream* logOutput, std::ostream* errorOutput) {
            return exec(argc, static_cast<const char**>(argv), logOutput, errorOutput);
        }

        /**
         * @brief Execute the tester
         * @param argc          Main function argument count
         * @param argv          Main function argument values
         * @return Non-zero if there are no test cases, if any test case fails
         *      or doesn't contain any checking macros, zero otherwise.
         *
         * The same as above, redirects log output to `std::cout` and error
         * output to `std::cerr`.
         */
        int exec(int argc, const char** argv);

        /** @overload */
        int exec(int argc, char** argv) {
            return exec(argc, const_cast<const char**>(argv));
        }

        /** @overload */
        int exec(int argc, std::nullptr_t argv) {
            return exec(argc, static_cast<const char**>(argv));
        }

        /**
         * @brief Add test cases
         *
         * Adds one or more test cases to be executed when calling @ref exec().
         * @see @ref addInstancedTests()
         */
        template<class Derived> void addTests(std::initializer_list<void(Derived::*)()> tests) {
            addRepeatedTests<Derived>(tests, 1);
        }

        /**
         * @brief Add repeated test cases
         *
         * Unlike the above function repeats each of the test cases until it
         * fails or @ref repeatCount is reached. Useful for stability or
         * resource leak checking. Each test case appears in the output log
         * only once.
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
         * Adds one or more test cases to be executed when calling @ref exec().
         * The @p setup function is called before every test case in the list,
         * the @p teardown function is called after every test case in the
         * list, regardless whether it passed, failed or was skipped. Using
         * verification macros in @p setup or @p teardown function is not
         * allowed.
         * @see @ref addInstancedTests()
         */
        template<class Derived> void addTests(std::initializer_list<void(Derived::*)()> tests, void(Derived::*setup)(), void(Derived::*teardown)()) {
            addRepeatedTests<Derived>(tests, 1, setup, teardown);
        }

        /**
         * @brief Add repeated test cases with explicit setup and teardown functions
         *
         * Unlike the above function repeats each of the test cases until it
         * fails or @ref repeatCount is reached. Useful for stability or
         * resource leak checking. The @p setup and @p teardown functions are
         * called again for each repeat of each test case. Each test case
         * appears in the output log only once.
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
         * appears in the output once for each instance.
         * @see @ref testCaseInstanceId(), @ref setTestCaseDescription()
         */
        template<class Derived> void addInstancedTests(std::initializer_list<void(Derived::*)()> tests, std::size_t instanceCount) {
            addRepeatedInstancedTests<Derived>(tests, 1, instanceCount);
        }

        /**
         * @brief Add repeated instanced test cases
         *
         * Unlike the above function repeats each of the test case instances
         * until it fails or @ref repeatCount is reached. Useful for stability
         * or resource leak checking. Each test case appears in the output once
         * for each instance.
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
         * Unlike @ref addTests(), this function runs each of the test cases
         * @p instanceCount times. Useful for data-driven tests. The @p setup
         * function is called before every instance of every test case in the
         * list, the @p teardown function is called after every instance of
         * every test case in the list, regardless whether it passed, failed or
         * was skipped. Using verification macros in @p setup or @p teardown
         * function is not allowed.
         */
        template<class Derived> void addInstancedTests(std::initializer_list<void(Derived::*)()> tests, std::size_t instanceCount, void(Derived::*setup)(), void(Derived::*teardown)()) {
            addRepeatedInstancedTests<Derived>(tests, 1, instanceCount, setup, teardown);
        }

        /**
         * @brief Add repeated instanced test cases with explicit setup and teardown functions
         *
         * Unlike the above function repeats each of the test case instances
         * until it fails or @ref repeatCount is reached. Useful for stability
         * or resource leak checking. The @p setup and @p teardown functions
         * are called again for each repeat of each instance of each test case.
         * The test case appears in the output once for each instance.
         * @see @ref addInstancedTests(), @ref addRepeatedInstancedTests()
         */
        template<class Derived> void addRepeatedInstancedTests(std::initializer_list<void(Derived::*)()> tests, std::size_t repeatCount, std::size_t instanceCount, void(Derived::*setup)(), void(Derived::*teardown)()) {
            _testCases.reserve(_testCases.size() + tests.size());
            for(auto test: tests) for(std::size_t i = 0; i != instanceCount; ++i)
                _testCases.emplace_back(i, repeatCount, static_cast<TestCase::Function>(test), static_cast<TestCase::Function>(setup), static_cast<TestCase::Function>(teardown));
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

    #ifdef DOXYGEN_GENERATING_OUTPUT
    private:
    #endif
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

        void registerTest(std::string filename, std::string name);

        void skip(const std::string& message);

    #ifndef DOXYGEN_GENERATING_OUTPUT
    protected:
    #endif
        class CORRADE_TESTSUITE_EXPORT ExpectedFailure {
            public:
                explicit ExpectedFailure(Tester* instance, std::string message, bool enabled = true);

                /* For types with explicit bool conversion */
                template<class T> explicit ExpectedFailure(Tester* instance, std::string message, T&& enabled): ExpectedFailure{instance, message, enabled ? true : false} {}

                ~ExpectedFailure();

                std::string message() const;

            private:
                Tester* _instance;
                std::string _message;
        };

        void registerTestCase(std::string&& name, int line);

    private:
        class Exception {};
        class SkipException {};

        struct TestCase {
            typedef void (Tester::*Function)();

            explicit TestCase(std::size_t instanceId, std::size_t repeatCount, Function test, Function setup, Function teardown): instanceId{instanceId}, repeatCount{repeatCount}, test{test}, setup{setup}, teardown{teardown} {}

            std::size_t instanceId, repeatCount;
            Function test, setup, teardown;
        };

        void verifyInternal(const std::string& expression, bool value);
        void printTestCaseLabel(Debug& out, const char* status, Debug::Color statusColor, Debug::Color labelColor);

        Debug::Flags _useColor;
        std::ostream *_logOutput, *_errorOutput;
        std::vector<TestCase> _testCases;
        std::string _testFilename, _testName, _testCaseName,
            _testCaseDescription, _expectFailMessage;
        std::size_t _testCaseId, _testCaseInstanceId, _testCaseRepeatId,
            _testCaseLine, _checkCount;
        ExpectedFailure* _expectedFailure;
        TesterConfiguration _configuration;
};

/** @hideinitializer
@brief Create `main()` function for given Tester subclass
*/
#ifdef CORRADE_TARGET_EMSCRIPTEN
/* In Emscripten, returning from main() with non-zero exit code won't
   affect Node.js exit code, causing all tests to look like they passed.
   Calling std::abort() causes Node.js to exit with non-zero code. The lambda
   voodoo is done to have `t` properly destructed before aborting. */
/** @todo Remove workaround when Emscripten can properly propagate exit codes */
#define CORRADE_TEST_MAIN(Class)                                            \
    int main(int argc, const char** argv) {                                 \
        if([&argc, argv]() {                                                \
            Class t;                                                        \
            t.registerTest(__FILE__, #Class);                               \
            return t.exec(argc, argv);                                      \
        }() != 0) std::abort();                                             \
        return 0;                                                           \
    }
#elif defined(CORRADE_TESTSUITE_TARGET_XCTEST)
#define CORRADE_TEST_MAIN(Class)                                            \
    int CORRADE_VISIBILITY_EXPORT corradeTestMain(int argc, const char** argv) { \
        Class t;                                                            \
        t.registerTest(__FILE__, #Class);                                   \
        return t.exec(argc, argv);                                          \
    }
#else
#define CORRADE_TEST_MAIN(Class)                                            \
    int main(int argc, const char** argv) {                                 \
        Class t;                                                            \
        t.registerTest(__FILE__, #Class);                                   \
        return t.exec(argc, argv);                                          \
    }
#endif

#ifndef DOXYGEN_GENERATING_OUTPUT
#define _CORRADE_REGISTER_TEST_CASE()                                       \
    Tester::registerTestCase(__func__, __LINE__);
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
    ExpectedFailure _CORRADE_HELPER_PASTE(expectedFailure, __LINE__)(this, message)

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
    ExpectedFailure _CORRADE_HELPER_PASTE(expectedFailure, __LINE__)(this, message, condition)

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
            << actual << "and" << expected << "are not equal.";
        return;
    }

    /* Otherwise print message to error output and throw exception */
    Error out{_errorOutput, _useColor};
    printTestCaseLabel(out, _expectedFailure ? " XPASS" : "  FAIL", Debug::Color::Red, Debug::Color::Default);
    out << "at" << _testFilename << "on line"
        << _testCaseLine << Debug::newline << "       ";
    if(!_expectedFailure) comparator.printErrorMessage(out, actual, expected);
    else out << actual << "and" << expected << "are not expected to be equal.";
    throw Exception();
}

template<class T> void Tester::verify(const std::string& expression, T&& value) {
    if(value) verifyInternal(expression, true);
    else verifyInternal(expression, false);
}

}}

#endif
