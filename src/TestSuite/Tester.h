#ifndef Corrade_TestSuite_Tester_h
#define Corrade_TestSuite_Tester_h
/*
    Copyright © 2007, 2008, 2009, 2010, 2011, 2012
              Vladimír Vondruš <mosra@centrum.cz>

    This file is part of Corrade.

    Corrade is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License version 3
    only, as published by the Free Software Foundation.

    Corrade is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU Lesser General Public License version 3 for more details.
*/

/** @file
 * @brief Class Corrade::TestSuite::Tester, macros CORRADE_TEST_MAIN(), CORRADE_VERIFY(), CORRADE_COMPARE(), CORRADE_COMPARE_AS().
 */

#include <functional>

#include "Utility/Debug.h"
#include "Compare.h"

namespace Corrade { namespace TestSuite {

/**
@brief Base class for unit tests

See @ref UnitTesting for introduction.

@see CORRADE_TEST_MAIN(), CORRADE_VERIFY(), CORRADE_COMPARE(), CORRADE_COMPARE_AS()
*/
template<class Derived> class Tester {
    public:
        /** @brief Pointer to test case function */
        typedef void (Derived::*TestCase)();

        inline constexpr Tester(): testCaseLine(0) {}

        /**
         * @brief Execute the tester
         * @return Non-zero code if any test case fails or doesn't contain any
         *      checking macros.
         */
        int exec() {
            Utility::Debug() << "Starting" << testName << "with" << testCases.size() << "test cases...";

            unsigned int errorCount = 0,
                noCheckCount = 0;

            for(auto i: testCases) {
                try {
                    testCaseName.clear();
                    i(*static_cast<Derived*>(this));
                } catch(Exception e) {
                    ++errorCount;
                    continue;
                }

                /* No testing macros called, don't print function name to output */
                if(testCaseName.empty()) {
                    ++noCheckCount;
                    continue;
                }

                Utility::Debug() << "    OK:" << testCaseName;
            }

            Utility::Debug d;
            d << "Finished" << testName << "with" << errorCount << "errors.";
            if(noCheckCount)
                d << noCheckCount << "test cases didn't contain any checks!";

            return errorCount != 0 || noCheckCount != 0;
        }

        /**
         * @brief Add test cases
         *
         * Adds one or more test cases to be executed when calling exec().
         */
        template<class ...T> void addTests(TestCase first, T... next) {
            testCases.push_back(std::mem_fn(first));

            addTests(next...);
        }

        #ifndef DOXYGEN_GENERATING_OUTPUT
        template<class T> inline void compare(const std::string& actual, const T& actualValue, const std::string& expected, const T& expectedValue) {
            return compare<T, T, T>(actual, actualValue, expected, expectedValue);
        }
        template<class T, class U, class V> void compare(const std::string& actual, const U& actualValue, const std::string& expected, const V& expectedValue) {
            Compare<T> compare;
            if(compare(actualValue, expectedValue)) return;

            Utility::Error e;
            e << "  FAIL:" << testCaseName << "at" << testFilename << "on line" << testCaseLine << "\n       ";
            compare.printErrorMessage(e, actual, expected);
            throw Exception();
        }

        void verify(const std::string& expression, bool expressionValue) {
            if(expressionValue) return;

            Utility::Error() << "  FAIL:" << testCaseName << "at" << testFilename << "on line" << testCaseLine << "\n        Expression" << expression << "failed.";
            throw Exception();
        }

        inline void registerTest(const std::string& filename, const std::string& name) {
            testFilename = filename;
            testName = name;
        }

    protected:
        inline void registerTestCase(const std::string& name, int line) {
            if(testCaseName.empty()) testCaseName = name + "()";
            this->testCaseLine = line;
        }
        #endif

    private:
        class Exception {};

        void addTests() {} /* Terminator function for addTests() */

        std::vector<std::function<void(Derived&)>> testCases;
        std::string testFilename, testName, testCaseName;
        size_t testCaseLine;
};

/** @hideinitializer
@brief Create `main()` function for given Tester subclass
*/
#define CORRADE_TEST_MAIN(Class)                                            \
    int main(int, char**) {                                                 \
        Class t;                                                            \
        t.registerTest(__FILE__, #Class);                                   \
        return t.exec();                                                    \
    }

#ifndef DOXYGEN_GENERATING_OUTPUT
#define _CORRADE_REGISTER_TEST_CASE()                                       \
    registerTestCase(__func__, __LINE__);
#endif

/** @hideinitializer
@brief Verify an expression in Tester subclass

If the expression is not true, the expression is printed and execution of given
test case is terminated. Example usage:
@code
string s("hello");
CORRADE_VERIFY(!s.empty());
@endcode

@see CORRADE_COMPARE(), CORRADE_COMPARE_AS()
*/
#define CORRADE_VERIFY(expression)                                          \
    do {                                                                    \
        _CORRADE_REGISTER_TEST_CASE();                                      \
        verify(#expression, expression);                                    \
    } while(false)

/** @hideinitializer
@brief Compare two values in Tester subclass

If the values are not the same, they are printed for comparison and execution
of given test case is terminated. Example usage:
@code
int a = 5 + 3;
CORRADE_COMPARE(a, 8);
@endcode

@see CORRADE_VERIFY(), CORRADE_COMPARE_AS()
*/
#define CORRADE_COMPARE(actual, expected)                                   \
    do {                                                                    \
        _CORRADE_REGISTER_TEST_CASE();                                      \
        compare(#actual, actual, #expected, expected);                      \
    } while(false)

/** @hideinitializer
@brief Compare two values in Tester subclass with explicitly specified type

If the values are not the same, they are printed for comparison and execution
of given test case is terminated. Example usage:
@code
CORRADE_COMPARE_AS(sin(0.0f), 0.0f, float);
@endcode
See also @ref Corrade::TestSuite::Compare "Compare" class documentation for
example of more involved comparisons.

@see CORRADE_VERIFY(), CORRADE_COMPARE()
*/
#define CORRADE_COMPARE_AS(actual, expected, Type)                          \
    do {                                                                    \
        _CORRADE_REGISTER_TEST_CASE();                                      \
        compare<Type>(#actual, actual, #expected, expected);                \
    } while(false)

}}

#endif
