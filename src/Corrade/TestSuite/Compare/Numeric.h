#ifndef Corrade_TestSuite_Compare_Numeric_h
#define Corrade_TestSuite_Compare_Numeric_h
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
 * @brief Class @ref Corrade::TestSuite::Compare::Less, @ref Corrade::TestSuite::Compare::LessOrEqual, @ref Corrade::TestSuite::Compare::GreaterOrEqual, @ref Corrade::TestSuite::Compare::Greater
 */

#include "Corrade/TestSuite/Tester.h"
#include "Corrade/TestSuite/TestSuite.h"
#include "Corrade/Utility/Debug.h"

namespace Corrade { namespace TestSuite {

namespace Compare {

/**
@brief Pseudo-type for verifying that value is less than expected

Prints both values if the first is not less than the second. Compared type
needs to implement at least an @cpp operator<() @ce (explicitly) convertible to
@cpp bool @ce. Example usage:

@snippet TestSuite.cpp Compare-Less

See @ref TestSuite-Comparator-pseudo-types for more information.
@see @ref LessOrEqual, @ref GreaterOrEqual, @ref Greater
*/
template<class T> class Less {};

/**
@brief Pseudo-type for verifying that value is less than expected

Prints both values if the first is not less than the second. Compared type
needs to implement at least an @cpp operator<=() @ce (explicitly) convertible
to @cpp bool @ce. Example usage:

@snippet TestSuite.cpp Compare-LessOrEqual

See @ref TestSuite-Comparator-pseudo-types for more information.
@see @ref LessOrEqual, @ref GreaterOrEqual, @ref Greater
*/
template<class T> class LessOrEqual {};

/**
@brief Pseudo-type for verifying that value is less than expected

Prints both values if the first is not less than the second.  Compared type
needs to implement at least an @cpp operator>=() @ce (explicitly) convertible
to @cpp bool @ce. Example usage:

@snippet TestSuite.cpp Compare-GreaterOrEqual

See @ref TestSuite-Comparator-pseudo-types for more information.
@see @ref LessOrEqual, @ref GreaterOrEqual, @ref Greater
*/
template<class T> class GreaterOrEqual {};

/**
@brief Pseudo-type for verifying that value is less than expected

Prints both values if the first is not less than the second. Compared type
needs to implement at least an @cpp operator>() @ce (explicitly) convertible to
@cpp bool @ce. Example usage:

@snippet TestSuite.cpp Compare-Greater

See @ref TestSuite-Comparator-pseudo-types for more information.
@see @ref LessOrEqual, @ref GreaterOrEqual, @ref Greater
*/
template<class T> class Greater {};

/**
@brief Pseudo-type for verifying that value is in given bounds

Prints both values if
@cpp !(actual >= expected - epsilon && expected + epsilon >= actual) @ce.
Compared type needs to implement at least an @cpp operator-() @ce,
@cpp operator+() @ce and @cpp operator>=() @ce. Example usage:

@snippet TestSuite.cpp Compare-Around

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

@snippet TestSuite.cpp Compare-around
*/
template<class T> inline Around<T> around(T epsilon) { return Around<T>{epsilon}; }

/**
@brief Pseudo-type for verifying that value is divisible by
@m_since{2020,06}

Prints both values if the first *is not* divisible by the second. Compared type
needs to implement at least an @cpp operator%() @ce returning the same type.
Example usage:

@snippet TestSuite.cpp Compare-Divisible

See @ref TestSuite-Comparator-pseudo-types for more information.
@see @ref NotDivisible
*/
template<class T> class Divisible {};

/**
@brief Pseudo-type for verifying that value is not divisible by
@m_since{2020,06}

Prints both values if the first *is* divisible by the second. Compared type
needs to implement at least an @cpp operator%() @ce returning the same type.
Example usage:

@snippet TestSuite.cpp Compare-NotDivisible

See @ref TestSuite-Comparator-pseudo-types for more information.
@see @ref Divisible
*/
template<class T> class NotDivisible {};

}

#ifndef DOXYGEN_GENERATING_OUTPUT
template<class T> class Comparator<Compare::Less<T>> {
    public:
        ComparisonStatusFlags operator()(const T& actual, const T& expected) {
            _actualValue = &actual;
            _expectedValue = &expected;
            return *_actualValue < *_expectedValue ?
                ComparisonStatusFlags{} : ComparisonStatusFlag::Failed;
        }

        void printMessage(ComparisonStatusFlags, Utility::Debug& out, const char* actual, const char* expected) const {
            out << "Value" << actual << "is not less than" << expected
                << Utility::Debug::nospace << ", actual is" << *_actualValue
                << "but expected <" << *_expectedValue;
        }

    private:
        const T* _actualValue;
        const T* _expectedValue;
};

template<class T> class Comparator<Compare::LessOrEqual<T>> {
    public:
        ComparisonStatusFlags operator()(const T& actual, const T& expected) {
            _actualValue = &actual;
            _expectedValue = &expected;
            return *_actualValue <= *_expectedValue ?
                ComparisonStatusFlags{} : ComparisonStatusFlag::Failed;
        }

        void printMessage(ComparisonStatusFlags, Utility::Debug& out, const char* actual, const char* expected) const {
            out << "Value" << actual << "is not less than or equal to" << expected
                << Utility::Debug::nospace << ", actual is" << *_actualValue
                << "but expected <=" << *_expectedValue;
        }

    private:
        const T* _actualValue;
        const T* _expectedValue;
};

template<class T> class Comparator<Compare::GreaterOrEqual<T>> {
    public:
        ComparisonStatusFlags operator()(const T& actual, const T& expected) {
            _actualValue = &actual;
            _expectedValue = &expected;
            return *_actualValue >= *_expectedValue ?
                ComparisonStatusFlags{} : ComparisonStatusFlag::Failed;
        }

        void printMessage(ComparisonStatusFlags, Utility::Debug& out, const char* actual, const char* expected) const {
            out << "Value" << actual << "is not greater than or equal to" << expected
                << Utility::Debug::nospace << ", actual is" << *_actualValue
                << "but expected >=" << *_expectedValue;
        }

    private:
        const T* _actualValue;
        const T* _expectedValue;
};

template<class T> class Comparator<Compare::Greater<T>> {
    public:
        ComparisonStatusFlags operator()(const T& actual, const T& expected) {
            _actualValue = &actual;
            _expectedValue = &expected;
            return *_actualValue > *_expectedValue ?
                ComparisonStatusFlags{} : ComparisonStatusFlag::Failed;
        }

        void printMessage(ComparisonStatusFlags, Utility::Debug& out, const char* actual, const char* expected) const {
            out << "Value" << actual << "is not greater than" << expected
                << Utility::Debug::nospace << ", actual is" << *_actualValue
                << "but expected >" << *_expectedValue;
        }

    private:
        const T* _actualValue;
        const T* _expectedValue;
};

template<class T> class Comparator<Compare::Around<T>> {
    public:
        /* Has to be () and not {} otherwise GCC 4.8 complains that "too many
           initializers for Bar" in NumericTest.cpp: */
        explicit Comparator(T epsilon): _epsilon(epsilon) {}

        ComparisonStatusFlags operator()(const T& actual, const T& expected) {
            _actualValue = &actual;
            _expectedValue = &expected;
            return *_actualValue >= *_expectedValue - _epsilon &&
                   *_expectedValue + _epsilon >= *_actualValue ?
                ComparisonStatusFlags{} : ComparisonStatusFlag::Failed;
        }

        void printMessage(ComparisonStatusFlags, Utility::Debug& out, const char* actual, const char* expected) const {
            out << "Value" << actual << "is not around" << expected
                << Utility::Debug::nospace << ", actual is" << *_actualValue
                << "but" << *_expectedValue - _epsilon << "<= expected <="
                << *_expectedValue + _epsilon;
        }

    private:
        T _epsilon;
        const T* _actualValue;
        const T* _expectedValue;
};

template<class T> class Comparator<Compare::Divisible<T>> {
    public:
        ComparisonStatusFlags operator()(const T& actual, const T& expected) {
            _actualValue = &actual;
            _expectedValue = &expected;
            return *_actualValue % *_expectedValue == T() ?
                ComparisonStatusFlags{} : ComparisonStatusFlag::Failed;
        }

        void printMessage(ComparisonStatusFlags, Utility::Debug& out, const char* actual, const char* expected) const {
            out << "Value" << actual << "is not divisible by" << expected
                << Utility::Debug::nospace << "," << *_actualValue << "%"
                << *_expectedValue << "was not expected to be" << (*_actualValue % *_expectedValue);
        }

    private:
        const T* _actualValue;
        const T* _expectedValue;
};

template<class T> class Comparator<Compare::NotDivisible<T>> {
    public:
        ComparisonStatusFlags operator()(const T& actual, const T& expected) {
            _actualValue = &actual;
            _expectedValue = &expected;
            return *_actualValue % *_expectedValue != T() ?
                ComparisonStatusFlags{} : ComparisonStatusFlag::Failed;
        }

        void printMessage(ComparisonStatusFlags, Utility::Debug& out, const char* actual, const char* expected) const {
            out << "Value" << actual << "is divisible by" << expected
                << Utility::Debug::nospace << "," << *_actualValue << "%"
                << *_expectedValue << "was not expected to be 0";
        }

    private:
        const T* _actualValue;
        const T* _expectedValue;
};

#endif

}}

#endif
