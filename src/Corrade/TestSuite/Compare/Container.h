#ifndef Corrade_TestSuite_Compare_Container_h
#define Corrade_TestSuite_Compare_Container_h
/*
    This file is part of Corrade.

    Copyright © 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016,
                2017, 2018, 2019, 2020, 2021, 2022
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
 * @brief Class @ref Corrade::TestSuite::Compare::Container
 */

#include "Corrade/TestSuite/Comparator.h"
#include "Corrade/Utility/Math.h"

namespace Corrade { namespace TestSuite {

namespace Compare {

/**
@brief Pseudo-type for comparing container contents

Prints the length of both containers (if they are different) and then prints
value of the first different item in both containers. Example usage:

@snippet TestSuite.cpp Compare-Container

Comparison of containers of floating-point types is by default done as a
fuzzy-compare, delegated to @ref Comparator<float> and @ref Comparator<double>.

This comparator can only compare containers that have random access (i.e.,
implementing @cpp operator[]() @ce). For comparing non-randomly-accessible
containers (such as @ref std::list or @ref std::map) and unordered containers
(such as  @ref std::unordered_map) use @ref SortedContainer instead.

See @ref TestSuite-Comparator-pseudo-types for more information.
*/
template<class> class Container {};

}

#ifndef DOXYGEN_GENERATING_OUTPUT
template<class T> class Comparator<Compare::Container<T>> {
    public:
        ComparisonStatusFlags operator()(const T& actual, const T& expected);

        void printMessage(ComparisonStatusFlags, Utility::Debug& out, const char* actual, const char* expected) const;

    private:
        const T* _actualContents;
        const T* _expectedContents;
        std::size_t _firstDifferent;
};

template<class T> ComparisonStatusFlags Comparator<Compare::Container<T>>::operator()(const T& actual, const T& expected) {
    _actualContents = &actual;
    _expectedContents = &expected;

    ComparisonStatusFlags status;
    if(_actualContents->size() != _expectedContents->size())
        status = ComparisonStatusFlag::Failed;

    /* Recursively use the comparator on the values, find the first different
       item in the common prefix. If there's none, then the first different
       item is right after the common prefix, and if both have the same size
       then it means the containers are the same. */
    Comparator<typename std::decay<decltype((*_actualContents)[0])>::type> comparator;
    const std::size_t commonPrefixSize = Utility::min(_actualContents->size(), _expectedContents->size());
    _firstDifferent = commonPrefixSize;
    for(std::size_t i = 0; i != commonPrefixSize; ++i) {
        if(comparator((*_actualContents)[i], (*_expectedContents)[i]) & ComparisonStatusFlag::Failed) {
            _firstDifferent = i;
            status = ComparisonStatusFlag::Failed;
            break;
        }
    }

    return status;
}

template<class T> void Comparator<Compare::Container<T>>::printMessage(ComparisonStatusFlags, Utility::Debug& out, const char* actual, const char* expected) const {
    out << "Containers" << actual << "and" << expected << "have different";
    if(_actualContents->size() != _expectedContents->size())
        out << "size, actual" << _actualContents->size() << "but" << _expectedContents->size() << "expected. Actual contents:\n       ";
    else
        out << "contents, actual:\n       ";

    out << *_actualContents << Utility::Debug::newline << "        but expected\n       " << *_expectedContents << Utility::Debug::newline << "       ";

    if(_actualContents->size() <= _firstDifferent)
        out << "Expected has" << (*_expectedContents)[_firstDifferent];
    else if(_expectedContents->size() <= _firstDifferent)
        out << "Actual has" << (*_actualContents)[_firstDifferent];
    else
        out << "Actual" << (*_actualContents)[_firstDifferent] << "but" << (*_expectedContents)[_firstDifferent] << "expected";

    out << "on position" << _firstDifferent << Utility::Debug::nospace << ".";
}
#endif

}}

#endif
