#ifndef Corrade_TestSuite_Compare_Container_h
#define Corrade_TestSuite_Compare_Container_h
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
 * @brief Class @ref Corrade::TestSuite::Compare::Container, @ref Corrade::TestSuite::Compare::SortedContainer
 */

#include <algorithm>

#include "Corrade/TestSuite/Comparator.h"

namespace Corrade { namespace TestSuite {

namespace Compare {

/**
@brief Pseudo-type for comparing container contents

Prints the length of both containers (if they are different) and prints value
of first different item in both containers. Example usage:
@code
std::vector<int> a, b;
CORRADE_COMPARE_AS(a, b, Compare::Container);
@endcode

It is also possible to sort the containers before comparison using
@ref SortedContainer :
@code
CORRADE_COMPARE_AS(a, b, Compare::SortedContainer);
@endcode

Comparison of containers of floating-point types is by default done as a
fuzzy-compare, see @ref Comparator<float> and @ref Comparator<double> for
details.

See @ref TestSuite-Comparator-pseudo-types for more information.
*/
template<class> class Container {};

/**
@brief Pseudo-type for comparing sorted container contents

See @ref Container for more information.
*/
template<class T> class SortedContainer: public Container<T> {};

}

#ifndef DOXYGEN_GENERATING_OUTPUT
template<class T> class Comparator<Compare::Container<T>> {
    public:
        bool operator()(const T& actual, const T& expected);

        void printErrorMessage(Utility::Error& e, const std::string& actual, const std::string& expected) const;

    private:
        const T* _actualContents;
        const T* _expectedContents;
};

template<class T> bool Comparator<Compare::Container<T>>::operator()(const T& actual, const T& expected) {
    _actualContents = &actual;
    _expectedContents = &expected;

    if(_actualContents->size() != _expectedContents->size()) return false;

    /* Recursively use comparator on the values */
    Comparator<typename std::decay<decltype((*_actualContents)[0])>::type> comparator;
    for(std::size_t i = 0; i != _actualContents->size(); ++i)
        if(!comparator((*_actualContents)[i], (*_expectedContents)[i])) return false;

    return true;
}

template<class T> void Comparator<Compare::Container<T>>::printErrorMessage(Utility::Error& e, const std::string& actual, const std::string& expected) const {
    e << "Containers" << actual << "and" << expected << "have different";
    if(_actualContents->size() != _expectedContents->size())
        e << "size, actual" << _actualContents->size() << "but" << _expectedContents->size() << "expected. Actual contents:\n       ";
    else
        e << "contents, actual:\n       ";

    e << *_actualContents << Utility::Debug::newline << "        but expected\n       " << *_expectedContents << Utility::Debug::newline << "       ";

    Comparator<typename std::decay<decltype((*_actualContents)[0])>::type> comparator;
    for(std::size_t i = 0, end = std::max(_actualContents->size(), _expectedContents->size()); i != end; ++i) {
        if(_actualContents->size() > i && _expectedContents->size() > i &&
            comparator((*_actualContents)[i], (*_expectedContents)[i])) continue;

        if(_actualContents->size() <= i)
            e << "Expected has" << (*_expectedContents)[i];
        else if(_expectedContents->size() <= i)
            e << "Actual has" << (*_actualContents)[i];
        else
            e << "Actual" << (*_actualContents)[i] << "but" << (*_expectedContents)[i] << "expected";

        e << "on position" << i << Utility::Debug::nospace << ".";
        break;
    }
}

template<class T> class Comparator<Compare::SortedContainer<T>>: public Comparator<Compare::Container<T>> {
    public:
        bool operator()(const T& actual, const T& expected);

    private:
        T _actualSorted, _expectedSorted;
};

template<class T> bool Comparator<Compare::SortedContainer<T>>::operator()(const T& actual, const T& expected) {
    _actualSorted = actual;
    _expectedSorted = expected;

    std::sort(_actualSorted.begin(), _actualSorted.end());
    std::sort(_expectedSorted.begin(), _expectedSorted.end());

    return Comparator<Compare::Container<T>>::operator()(_actualSorted, _expectedSorted);
}
#endif

}}

#endif
