#ifndef Corrade_TestSuite_Compare_FloatingPoint_h
#define Corrade_TestSuite_Compare_FloatingPoint_h
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
 * @brief Class Corrade::TestSuite::Comparator specialized for floating-point values
 */

#include "TestSuite/Comparator.h"

#include "TestSuite/corradeTestSuiteVisibility.h"

namespace Corrade { namespace TestSuite {

#ifndef DOXYGEN_GENERATING_OUTPUT
namespace Implementation {
    template<class T> class FloatComparatorEpsilon {};

    template<> class FloatComparatorEpsilon<float> {
        public:
            inline constexpr static float epsilon() {
                return 1.0e-6f;
            }
    };

    template<> class FloatComparatorEpsilon<double> {
        public:
            inline constexpr static double epsilon() {
                return 1.0e-12;
            }
    };

    template<class T> class CORRADE_TESTSUITE_EXPORT FloatComparator {
        public:
            bool operator()(T actual, T expected);
            void printErrorMessage(Utility::Error& e, const std::string& actual, const std::string& expected) const;

        private:
            T actualValue, expectedValue;
    };
}
#endif

/** @brief Fuzzy-compare for float values */
template<> class Comparator<float>: public Implementation::FloatComparator<float> {};

/** @brief Fuzzy-compare for double values */
template<> class Comparator<double>: public Implementation::FloatComparator<double> {};

}}

#endif
