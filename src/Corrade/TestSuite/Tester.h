#ifndef Corrade_TestSuite_Tester_h
#define Corrade_TestSuite_Tester_h
/*
    This file is part of Corrade.

    Copyright © 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015
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

/**
@brief Base class for unit tests

See @ref unit-testing for introduction.

@see @ref CORRADE_TEST_MAIN(), @ref CORRADE_VERIFY(), @ref CORRADE_COMPARE(),
    @ref CORRADE_COMPARE_AS(), @ref CORRADE_COMPARE_WITH(), @ref CORRADE_SKIP(),
    @ref CORRADE_EXPECT_FAIL(), @ref CORRADE_EXPECT_FAIL_IF()
@todo Data-driven tests
*/
class CORRADE_TESTSUITE_EXPORT Tester {
    public:
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

        explicit Tester();

        /**
         * @brief Execute the tester
         * @param logOutput     Output stream for log messages
         * @param errorOutput   Output stream for error messages
         * @return Non-zero if there are no test cases, if any test case fails
         *      or doesn't contain any checking macros, zero otherwise.
         */
        int exec(std::ostream* logOutput, std::ostream* errorOutput);

        /**
         * @brief Execute the tester
         * @return Non-zero if there are no test cases, if any test case fails
         *      or doesn't contain any checking macros, zero otherwise.
         *
         * The same as above, redirects log output to `std::cout` and error
         * output to `std::cerr`.
         */
        int exec();

        /**
         * @brief Add test cases
         *
         * Adds one or more test cases to be executed when calling @ref exec().
         */
        template<class Derived> void addTests(std::initializer_list<void(Derived::*)()> tests) {
            _testCases.reserve(_testCases.size() + tests.size());
            for(auto test: tests)
                _testCases.push_back(static_cast<TestCase>(test));
        }

    #ifdef DOXYGEN_GENERATING_OUTPUT
    private:
    #endif
        /* Compare two identical types without explicit type specification */
        template<class T> void compare(const std::string& actual, const T& actualValue, const std::string& expected, const T& expectedValue) {
            compareAs<T, T, T>(actual, actualValue, expected, expectedValue);
        }

        /* Compare two different types without explicit type specification */
        template<class T, class U> void compare(const std::string& actual, const T& actualValue, const std::string& expected, const U& expectedValue) {
            compareAs<typename std::common_type<T, U>::type, T, U>(actual, actualValue, expected, expectedValue);
        }

        /* Compare two different types with explicit templated type
           specification (e.g. Compare::Containers). This allows the user to
           call only `CORRADE_COMPARE_AS(a, b, Compare::Containers)` without
           explicitly specifying the type, e.g.
           `CORRADE_COMPARE_AS(a, b, Compare::Containers<std::vector<int>>)` */
        template<template<class> class T, class U, class V> void compareAs(const std::string& actual, const U& actualValue, const std::string& expected, const V& expectedValue) {
            compareAs<T<typename std::common_type<U, V>::type>, U, V>(actual, actualValue, expected, expectedValue);
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

                ~ExpectedFailure();

                std::string message() const;

            private:
                Tester* _instance;
                std::string _message;
        };

        void registerTestCase(const std::string& name, int line);

    private:
        class Exception {};
        class SkipException {};

        typedef void (Tester::*TestCase)();

        void addTests() {} /* Terminator function for addTests() */

        void verifyInternal(const std::string& expression, bool value);

        std::ostream *_logOutput, *_errorOutput;
        std::vector<TestCase> _testCases;
        std::string _testFilename, _testName, _testCaseName, _expectFailMessage;
        std::size_t _testCaseLine, _checkCount;
        ExpectedFailure* _expectedFailure;
};

/** @hideinitializer
@brief Create `main()` function for given Tester subclass
*/
#ifndef CORRADE_TARGET_EMSCRIPTEN
#define CORRADE_TEST_MAIN(Class)                                            \
    int main(int, char**) {                                                 \
        Class t;                                                            \
        t.registerTest(__FILE__, #Class);                                   \
        return t.exec();                                                    \
    }
#else
/* In Emscripten, returning from main() with non-zero exit code won't
   affect Node.js exit code, causing all tests to look like they passed.
   Calling std::abort() causes Node.js to exit with non-zero code. The lambda
   voodoo is done to have `t` properly destructed before aborting. */
/** @todo Remove workaround when Emscripten can properly propagate exit codes */
#define CORRADE_TEST_MAIN(Class)                                            \
    int main(int, char**) {                                                 \
        if([]() {                                                           \
            Class t;                                                        \
            t.registerTest(__FILE__, #Class);                               \
            return t.exec();                                                \
        }() != 0) std::abort();                                             \
        return 0;                                                           \
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
    CORRADE_EXPECT_FAIL_IF("This is not our universe.", answer != 42);

    CORRADE_VERIFY(6*7, 49); // expect the failure if answer is not 42
}
@endcode
*/
#define CORRADE_EXPECT_FAIL_IF(message, condition)                          \
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
        Utility::Debug(_logOutput) << " XFAIL:" << _testCaseName << "at" << _testFilename << "on line" << _testCaseLine << "\n       " << _expectedFailure->message() << actual << "and" << expected << "are not equal.";
        return;
    }

    /* Otherwise print message to error output and throw exception */
    Utility::Error e(_errorOutput);
    e << (_expectedFailure ? " XPASS:" : "  FAIL:") << _testCaseName << "at" << _testFilename << "on line" << _testCaseLine << "\n       ";
    if(!_expectedFailure) comparator.printErrorMessage(e, actual, expected);
    else e << actual << "and" << expected << "are not expected to be equal.";
    throw Exception();
}

template<class T> void Tester::verify(const std::string& expression, T&& value) {
    if(value) verifyInternal(expression, true);
    else verifyInternal(expression, false);
}

}}

#endif
