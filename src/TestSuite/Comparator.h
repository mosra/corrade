#ifndef Corrade_TestSuite_Comparator_h
#define Corrade_TestSuite_Comparator_h
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
 * @brief Class Corrade::TestSuite::Comparator
 */

#include <string>

#include "Utility/Debug.h"

namespace Corrade { namespace TestSuite {

/**
@brief Default comparator implementation

See CORRADE_COMPARE_AS(), CORRADE_COMPARE_WITH() for more information and
Compare namespace for pseudo-type comparator implementations.

@section Comparator-subclassing Subclassing

You can reimplement this class for your own data types and even pseudo types
for providing different ways to compare the same type.

You have to implement operator()() for comparison of two values with arbitrary
type and printErrorMessage() for printing error message when the comparison
failed.

@subsection Comparator-pseudo-types Comparing with pseudo-types

Imagine you have two filenames and you want to compare their contents instead
of comparing the filename strings. Because you want to also compare strings
elsewhere, you cannot override its behavior. The solution is to have some
"pseudo type", for which you create the Comparator template specialization, but
the actual comparison operator will still take strings as parameters:
@code
class FileContents {};

namespace Corrade { namespace TestSuite { // the namespace is important

template<> class Comparator<FileContents> {
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

}}
@endcode

You can add more overloads for operator()() in one class, e.g. for comparing
file contents with string or input stream etc. The actual use in unit test
would be like this:
@code
CORRADE_COMPARE_AS("/path/to/actual.dat", "/path/to/expected.dat", FileContents);
@endcode

@subsection Comparator-parameters Passing parameters to comparators

Sometimes you need to pass additional parameters to comparator class so you
can then use it in CORRADE_COMPARE_WITH() macro. In that case you need to
implement constructor and `comparator()` function in your pseudo-type. Function
`comparator()` returns reference to existing configured Comparator instance.
Example:
@code
class FileContents;

namespace Corrade { namespace TestSuite { // the namespace is important

template<> class Comparator<FileContents> {
    public:
        Comparator(const std::string& pathPrefix = "");
        bool operator()(const std::string& actual, const std::string& expected);
        void printErrorMessage(Utility::Error& e, const std::string& actual, const std::string& expected) const;

        // ...
};

}}

class FileContents {
    public:
        inline FileContents(const std::string& pathPrefix = ""): c(pathPrefix) {}

        inline Comparator<FileContents>& comparator() {
            return c;
        }

    private:
        Comparator<FileContents> c;
};
@endcode

Don't forget to allow default construction of Comparator, if you want to be
able to use it also with CORRADE_COMPARE_AS().

The actual use in unit test would be like this:
@code
CORRADE_COMPARE_WITH("actual.dat", "expected.dat", FileContents("/common/path/prefix"));
@endcode
*/
template<class T> class Comparator {
    public:
        inline constexpr Comparator(): actualValue(), expectedValue() {}

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

}}

#endif
