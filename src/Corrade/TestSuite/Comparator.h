#ifndef Corrade_TestSuite_Comparator_h
#define Corrade_TestSuite_Comparator_h
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

## Subclassing

You can reimplement this class for your own data types and even pseudo types
for providing different ways to compare the same type.

You have to implement `operator()()` for comparison of two values with
arbitrary type and `printErrorMessage()` for printing error message when the
comparison failed. The reason for having those two separated is allowing the
user to use colored output -- due to a limitation of Windows console API, where
it's only possible to specify text color when writing directly to the output
without any intermediate buffer.

@anchor TestSuite-Comparator-pseudo-types
### Comparing with pseudo-types

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
            _actualContents = ...;
            _expectedContents = ...;
            return _actualContents == _expectedContents;
        }

        void printErrorMessage(Utility::Error& e, const std::string& actual, const std::string& expected) const {
            e << "Files" << actual << "and" << expected << "are not the same, actual" << _actualContents << "but expected" << _expectedContents;
        }

    private:
        std::string _actualContents, _expectedContents;
};

}}
@endcode

You can add more overloads for `operator()()` in one class, e.g. for comparing
file contents with string or input stream etc. The actual use in unit test
would be like this:
@code
CORRADE_COMPARE_AS("/path/to/actual.dat", "/path/to/expected.dat", FileContents);
@endcode

@anchor TestSuite-Comparator-parameters
### Passing parameters to comparators

Sometimes you need to pass additional parameters to comparator class so you
can then use it in @ref CORRADE_COMPARE_WITH() macro. In that case you need to
implement constructor and `comparator()` function in your pseudo-type. Function
`comparator()` returns reference to existing configured @ref Comparator
instance. Example:
@code
class FileContents;

namespace Corrade { namespace TestSuite { // the namespace is important

template<> class Comparator<FileContents> {
    public:
        Comparator(const std::string& pathPrefix = {});
        bool operator()(const std::string& actual, const std::string& expected);
        void printErrorMessage(Utility::Error& e, const std::string& actual, const std::string& expected) const;

        // ...
};

}}

class FileContents {
    public:
        explicit FileContents(const std::string& pathPrefix = {}): _c{pathPrefix} {}

        Comparator<FileContents>& comparator() {
            return _c;
        }

    private:
        Comparator<FileContents> _c;
};
@endcode

Don't forget to allow default construction of @ref Comparator, if you want to
be able to use it also with @ref CORRADE_COMPARE_AS().

The actual use in unit test would be like this:
@code
CORRADE_COMPARE_WITH("actual.dat", "expected.dat", FileContents("/common/path/prefix"));
@endcode
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
