#ifndef Corrade_TestSuite_Compare_h
#define Corrade_TestSuite_Compare_h
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
 * @brief Class Corrade::TestSuite::Compare
 */

#include "Utility/Debug.h"

#include <cmath>

namespace Corrade { namespace TestSuite {

/**
@brief Default comparison implementation

You can reimplement this class for your own data types and even pseudo types
for providing different ways to compare the same type.

You have to implement operator()() for comparison of two values with arbitrary
type and printErrorMessage() for printing error message when the comparison
failed.

@section ComparePseudoTypes Comparing with pseudo types
Imagine you have two filenames and you want to compare their contents instead
of comparing the filename strings. Because you want to also compare strings
elsewhere, you cannot override its behavior. The solution is to have some
"pseudo type", for which you create the Compare template specialization, but
the actual comparison operator will still take strings as parameters:
@code
class FileContents {};

template<> class Compare<FileContents> {
    public:
        bool operator()(const std::string& actual, const std::string& expected) {
            actualContents = ...;
            expectedContents = ...;
            return actualContents == expectedContents;
        }

        void printErrorMessage(Utility::Error& e, const std::string& actual, const std::string& expected) const {
            e << "Files" << actual << "and" << expected << "are not the same, actual:" << actualContents << "vs. expected:" << expectedContents;
        }

    private:
        std::string actualContents, expectedContents;
};
@endcode
You can add more overloads for operator()() in one class, e.g. for comparing
file contents with string or input stream etc. The actual use in unit test is
for example as following:
@code
CORRADE_COMPARE_AS("actual.dat", "expected.dat", FileContents);
@endcode
*/
template<class T> class Compare {
    public:
        /** @brief %Compare two values */
        bool operator()(const T& actual, const T& expected) {
            if(actual == expected) return true;

            actualValue = actual;
            expectedValue = expected;
            return false;
        }

        /** @brief Print error message, assuming the two values are inequal */
        void printErrorMessage(Utility::Error& e, const std::string& actual, const std::string& expected) const {
            e << "Values" << actual << "and" << expected << "are not the same, actual" << actualValue << "but" << expectedValue << "expected.";
        }

    private:
        T actualValue, expectedValue;
};

#ifndef DOXYGEN_GENERATING_OUTPUT
namespace Implementation {
    template<class T> class FloatCompareEpsilon {};

    template<> class FloatCompareEpsilon<float> {
        public:
            inline constexpr static float epsilon() {
                return 1.0e-6f;
            }
    };

    template<> class FloatCompareEpsilon<double> {
        public:
            inline constexpr static double epsilon() {
                return 1.0e-12;
            }
    };

    template<class T> class FloatCompare {
        public:
            /** @brief %Compare two values */
            bool operator()(T actual, T expected) {
                if(actual == expected || (actual != actual && expected != expected) ||
                    std::abs(actual - expected) < FloatCompareEpsilon<T>::epsilon()) return true;

                actualValue = actual;
                expectedValue = expected;
                return false;
            }

            /** @brief Print error message, assuming the two values are inequal */
            void printErrorMessage(Utility::Error& e, const std::string& actual, const std::string& expected) const {
                e << "Floating-point values" << actual << "and" << expected << "are not the same, actual" << actualValue << "but" << expectedValue << "expected";
                e.setFlag(Utility::Debug::SpaceAfterEachValue, false);
                e << " (delta " << std::abs(actualValue-expectedValue) << ").";
                e.setFlag(Utility::Debug::SpaceAfterEachValue, true);
            }

        private:
            T actualValue, expectedValue;
    };
}
#endif

/** @brief Fuzzy-compare for float values */
template<> class Compare<float>: public Implementation::FloatCompare<float> {};

/** @brief Fuzzy-compare for double values */
template<> class Compare<double>: public Implementation::FloatCompare<double> {};

}}

#endif
