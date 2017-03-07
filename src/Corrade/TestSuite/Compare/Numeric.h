#ifndef Corrade_TestSuite_Compare_Numeric_h
#define Corrade_TestSuite_Compare_Numeric_h
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
 * @brief Class @ref Corrade::TestSuite::Compare::Less, @ref Corrade::TestSuite::Compare::LessOrEqual, @ref Corrade::TestSuite::Compare::GreaterOrEqual, @ref Corrade::TestSuite::Compare::Greater
 */

#include "Corrade/TestSuite/TestSuite.h"
#include "Corrade/Utility/Debug.h"

namespace Corrade { namespace TestSuite {

namespace Compare {

/**
@brief Pseudo-type for verifying that value is less than expected

Prints both values if the first is not less than the second. Compared type
needs to implement at least an `operator<` (explicitly) convertible to `bool`.
Example usage:
@code
float a;
CORRADE_COMPARE_AS(a, 9.28f, Compare::Less);
@endcode

See @ref TestSuite-Comparator-pseudo-types for more information.
@see @ref LessOrEqual, @ref GreaterOrEqual, @ref Greater
*/
template<class T> class Less {};

/**
@brief Pseudo-type for verifying that value is less than expected

Prints both values if the first is not less than the second. Compared type
needs to implement at least an `operator<=` (explicitly) convertible to `bool`.
Example usage:
@code
float a;
CORRADE_COMPARE_AS(a, 9.28f, Compare::LessOrEqual);
@endcode

See @ref TestSuite-Comparator-pseudo-types for more information.
@see @ref LessOrEqual, @ref GreaterOrEqual, @ref Greater
*/
template<class T> class LessOrEqual {};

/**
@brief Pseudo-type for verifying that value is less than expected

Prints both values if the first is not less than the second.  Compared type
needs to implement at least an `operator>=` (explicitly) convertible to `bool`.
Example usage:
@code
float a;
CORRADE_COMPARE_AS(a, 9.28f, Compare::GreaterOrEqual);
@endcode

See @ref TestSuite-Comparator-pseudo-types for more information.
@see @ref LessOrEqual, @ref GreaterOrEqual, @ref Greater
*/
template<class T> class GreaterOrEqual {};

/**
@brief Pseudo-type for verifying that value is less than expected

Prints both values if the first is not less than the second. Compared type
needs to implement at least an `operator>` (explicitly) convertible to `bool`.
Example usage:
@code
float a;
CORRADE_COMPARE_AS(a, 9.28f, Compare::Greater);
@endcode

See @ref TestSuite-Comparator-pseudo-types for more information.
@see @ref LessOrEqual, @ref GreaterOrEqual, @ref Greater
*/
template<class T> class Greater {};

/**
@brief Pseudo-type for verifying that value is in given bounds

Prints both values if `actual < expected - epsilon` or
`expected + epsilon < actual`. Compared type needs to implement at least an
`operator-`, `operator+` and `operator<`. Example usage:
@code
float a;
CORRADE_COMPARE_WITH(a, 9.28f, Compare::Around<float>{0.1f});
@endcode
@see @ref around()
*/
template<class T> class Around {
    public:
        /**
         * @brief Constructor
         * @param epsilon   Epsilon value for comparison
         */
        explicit Around(T epsilon): _c{epsilon} {}

        #ifndef DOXYGEN_GENERATING_OUTPUT
        Comparator<Compare::Around<T>> comparator() { return _c; }
        #endif

    private:
        Comparator<Compare::Around<T>> _c;
};

/**
@brief Make a pseudo-type for verifying that value is in given bounds

Convenience wrapper around @ref Around::Around(T). These two lines are
equivalent:
@code
CORRADE_COMPARE_WITH(a, 9.28f, Compare::Around<float>{0.1f});
CORRADE_COMPARE_WITH(a, 9.28f, Compare::around(0.1f));
@endcode
*/
template<class T> inline Around<T> around(T epsilon) { return Around<T>{epsilon}; }

}

#ifndef DOXYGEN_GENERATING_OUTPUT
template<class T> class Comparator<Compare::Less<T>> {
    public:
        bool operator()(const T& actual, const T& expected) {
            _actualValue = &actual;
            _expectedValue = &expected;
            return bool(*_actualValue < *_expectedValue);
        }

        void printErrorMessage(Utility::Error& e, const std::string& actual, const std::string& expected) const {
            e << "Value" << actual << "is not less than" << expected
              << Utility::Debug::nospace << ", actual is" << *_actualValue
              << "but expected <" << *_expectedValue;
        }

    private:
        const T* _actualValue;
        const T* _expectedValue;
};

template<class T> class Comparator<Compare::LessOrEqual<T>> {
    public:
        bool operator()(const T& actual, const T& expected) {
            _actualValue = &actual;
            _expectedValue = &expected;
            return bool(*_actualValue <= *_expectedValue);
        }

        void printErrorMessage(Utility::Error& e, const std::string& actual, const std::string& expected) const {
            e << "Value" << actual << "is not less than or equal to" << expected
              << Utility::Debug::nospace << ", actual is" << *_actualValue
              << "but expected <=" << *_expectedValue;
        }

    private:
        const T* _actualValue;
        const T* _expectedValue;
};

template<class T> class Comparator<Compare::GreaterOrEqual<T>> {
    public:
        bool operator()(const T& actual, const T& expected) {
            _actualValue = &actual;
            _expectedValue = &expected;
            return bool(*_actualValue >= *_expectedValue);
        }

        void printErrorMessage(Utility::Error& e, const std::string& actual, const std::string& expected) const {
            e << "Value" << actual << "is not greater than or equal to" << expected
              << Utility::Debug::nospace << ", actual is" << *_actualValue
              << "but expected >=" << *_expectedValue;
        }

    private:
        const T* _actualValue;
        const T* _expectedValue;
};

template<class T> class Comparator<Compare::Greater<T>> {
    public:
        bool operator()(const T& actual, const T& expected) {
            _actualValue = &actual;
            _expectedValue = &expected;
            return bool(*_actualValue > *_expectedValue);
        }

        void printErrorMessage(Utility::Error& e, const std::string& actual, const std::string& expected) const {
            e << "Value" << actual << "is not greater than" << expected
              << Utility::Debug::nospace << ", actual is" << *_actualValue
              << "but expected >" << *_expectedValue;
        }

    private:
        const T* _actualValue;
        const T* _expectedValue;
};

template<class T> class Comparator<Compare::Around<T>> {
    public:
        /* GCC 4.7 and 4.9 (Travis CI Ubuntu and Travis CI Android) complain
           about too many intiializers for _epsilon{epsilon} in the test that
           uses an empty Bar structure. So can't use {} here. */
        explicit Comparator(T epsilon): _epsilon(epsilon) {}

        bool operator()(const T& actual, const T& expected) {
            _actualValue = &actual;
            _expectedValue = &expected;
            return !(*_actualValue < *_expectedValue - _epsilon ||
                     *_expectedValue + _epsilon < *_actualValue);
        }

        void printErrorMessage(Utility::Error& e, const std::string& actual, const std::string& expected) const {
            e << "Value" << actual << "is not around" << expected
              << Utility::Debug::nospace << ", actual is" << *_actualValue
              << "but" << *_expectedValue - _epsilon << "<= expected <="
              << *_expectedValue + _epsilon;
        }

    private:
        T _epsilon;
        const T* _actualValue;
        const T* _expectedValue;
};
#endif

}}

#endif
