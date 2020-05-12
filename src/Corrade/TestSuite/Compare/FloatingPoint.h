#ifndef Corrade_TestSuite_Compare_FloatingPoint_h
#define Corrade_TestSuite_Compare_FloatingPoint_h
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
 * @brief Class @ref Corrade::TestSuite::Comparator specialized for floating-point values
 */

#include "Corrade/TestSuite/TestSuite.h"
#include "Corrade/TestSuite/visibility.h"
#include "Corrade/Utility/Utility.h"

namespace Corrade { namespace TestSuite {

namespace Implementation {
    template<class T> class CORRADE_TESTSUITE_EXPORT FloatComparator {
        public:
            ComparisonStatusFlags operator()(T actual, T expected);
            void printMessage(ComparisonStatusFlags, Utility::Debug& out, const char* actual, const char* expected) const;

        private:
            T _actualValue, _expectedValue;
    };
}

/**
@brief Fuzzy-compare for float values

Uses comparison algorithm from http://floating-point-gui.de/errors/comparison/
with epsilon equal to @cpp 1.0e-5f @ce (which is one digit less than how
@ref Utility::Debug or @ref Utility::format() prints them). Unlike the standard
floating-point comparison, comparing two NaN values gives a @cpp true @ce
result.
@see @ref Compare::Around
*/
template<> class Comparator<float>: public Implementation::FloatComparator<float> {};

/**
@brief Fuzzy-compare for double values

Uses comparison algorithm from http://floating-point-gui.de/errors/comparison/
with epsilon equal to @cpp 1.0e-14 @ce (which is one digit less than how
@ref Utility::Debug or @ref Utility::format() prints them). Unlike the standard
floating-point comparison, comparing two NaN values gives a @cpp true @ce
result.
@see @ref Compare::Around
*/
template<> class Comparator<double>: public Implementation::FloatComparator<double> {};

/**
@brief Fuzzy-compare for long double values

Uses comparison algorithm from http://floating-point-gui.de/errors/comparison/
with epsilon equal to @cpp 1.0e-17l @ce on platforms with 80-bit
@cpp long double @ce and @cpp 1.0e-14 @ce on platforms
@ref CORRADE_LONG_DOUBLE_SAME_AS_DOUBLE "where it is 64-bit". This is always
one digit less than how @ref Utility::Debug or @ref Utility::format() prints
them. Unlike the standard floating-point comparison, comparing two NaN values
gives a @cpp true @ce result.
@see @ref Compare::Around
*/
template<> class Comparator<long double>: public Implementation::FloatComparator<long double> {};

}}

#endif
