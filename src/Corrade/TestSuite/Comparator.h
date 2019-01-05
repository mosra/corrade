#ifndef Corrade_TestSuite_Comparator_h
#define Corrade_TestSuite_Comparator_h
/*
    This file is part of Corrade.

    Copyright © 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016,
                2017, 2018, 2019 Vladimír Vondruš <mosra@centrum.cz>

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
 * @brief Class @ref Corrade::TestSuite::Comparator
 */

#include <string>

#include "Corrade/Utility/Assert.h"
#include "Corrade/Utility/Debug.h"

namespace Corrade { namespace TestSuite {

/**
@brief Default comparator implementation

See @ref CORRADE_COMPARE_AS(), @ref CORRADE_COMPARE_WITH() for more information
and @ref Compare namespace for pseudo-type comparator implementations.

@section TestSuite-Comparator-subclassing Subclassing

You can reimplement this class for your own data types and even pseudo types
to provide different ways to compare the same type.

You have to implement @cpp operator()() @ce for comparison of two values of
arbitrary types and @cpp printErrorMessage() @ce for printing error message
when the comparison failed. The reason for having those two separated is
allowing the user to use colored output --- due to a limitation of Windows
console API, where it's only possible to specify text color when writing
directly to the output without any intermediate buffer.

@section TestSuite-Comparator-pseudo-types Comparing with pseudo-types

Imagine you have two filenames and you want to compare their contents instead
of comparing the filename strings. Because you want to also compare strings
elsewhere, you cannot override the default behavior. The solution is to have
some "pseudo type", for which you create the @ref Comparator template
specialization, but the actual comparison operator will still take strings as
parameters:

@snippet TestSuite.cpp Comparator-pseudotypes

The actual use in the unit test would be like this:

@snippet TestSuite.cpp Comparator-pseudotypes-usage

@attention Due to implementation limitations, it's not possible to have
    multiple overloads for @cpp operator()() @ce in one class (for example to
    compare file contents with both a filename and a @ref std::istream), you
    have to create a different pseudo-type for that.

@section TestSuite-Comparator-parameters Passing parameters to comparators

Sometimes you need to pass additional parameters to the comparator class so you
can then use it in the @ref CORRADE_COMPARE_WITH() macro. In that case you need
to implement the constructor and a @cpp comparator() @ce function in your
pseudo-type. The @cpp comparator() @ce function returns reference to existing
configured @ref Comparator instance. Example:

@snippet TestSuite2.cpp Comparator-parameters

Don't forget to allow default construction of @ref Comparator, if you want to
be able to use it also with @ref CORRADE_COMPARE_AS().

The actual use in a test would be like this:

@snippet TestSuite2.cpp Comparator-parameters-usage
*/
template<class T> class Comparator {
    public:
        explicit Comparator();

        /** @brief Compare two values */
        bool operator()(const T& actual, const T& expected);

        /** @brief Print error message, assuming the two values are inequal */
        void printErrorMessage(Utility::Error& e, const std::string& actual, const std::string& expected) const;

    private:
        const T* actualValue;
        const T* expectedValue;
};

template<class T> Comparator<T>::Comparator(): actualValue(), expectedValue() {}

template<class T> bool Comparator<T>::operator()(const T& actual, const T& expected) {
    if(actual == expected) return true;

    actualValue = &actual;
    expectedValue = &expected;
    return false;
}

template<class T> void Comparator<T>::printErrorMessage(Utility::Error& e, const std::string& actual, const std::string& expected) const {
    CORRADE_INTERNAL_ASSERT(actualValue && expectedValue);
    e << "Values" << actual << "and" << expected << "are not the same, actual is\n       "
      << *actualValue << Utility::Debug::newline << "        but expected\n       " << *expectedValue;
}

namespace Implementation {

template<class T> struct ComparatorOperatorTraits;

template<class T, class U, class V> struct ComparatorOperatorTraits<bool(T::*)(U, V)> {
    typedef typename std::decay<U>::type ActualType;
    typedef typename std::decay<V>::type ExpectedType;
};

template<class T> struct ComparatorTraits: ComparatorOperatorTraits<decltype(&Comparator<T>::operator())> {};

}

}}

#endif
