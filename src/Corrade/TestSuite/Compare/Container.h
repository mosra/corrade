#ifndef Corrade_TestSuite_Compare_Container_h
#define Corrade_TestSuite_Compare_Container_h
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
 * @brief Class @ref Corrade::TestSuite::Compare::Container
 */

#include "Corrade/TestSuite/Comparator.h"

namespace Corrade { namespace TestSuite {

namespace Compare {

/**
@brief Pseudo-type for comparing container contents

Prints the length of both containers (if they are different) and then prints
value of the first different item in both containers. Example usage:

@snippet TestSuite.cpp Compare-Container

It is also possible to sort the containers before comparison using
@link SortedContainer @endlink:

@snippet TestSuite.cpp Compare-SortedContainer

Comparison of containers of floating-point types is by default done as a
fuzzy-compare, see @ref Comparator<float> and @ref Comparator<double> for
details.

See @ref TestSuite-Comparator-pseudo-types for more information.
*/
template<class> class Container {};

}

namespace Implementation {
    /* Copied from Magnum/Math/Vector.h, to avoid #include <algorithm> */
    inline std::size_t max(std::size_t a, std::size_t b) { return a < b ? b : a; }
}

#ifndef DOXYGEN_GENERATING_OUTPUT
template<class T> class Comparator<Compare::Container<T>> {
    public:
        ComparisonStatusFlags operator()(const T& actual, const T& expected);

        void printMessage(ComparisonStatusFlags, Utility::Debug& out, const char* actual, const char* expected) const;

    private:
        const T* _actualContents;
        const T* _expectedContents;
};

template<class T> ComparisonStatusFlags Comparator<Compare::Container<T>>::operator()(const T& actual, const T& expected) {
    _actualContents = &actual;
    _expectedContents = &expected;

    if(_actualContents->size() != _expectedContents->size())
        return ComparisonStatusFlag::Failed;

    /* Recursively use comparator on the values */
    Comparator<typename std::decay<decltype((*_actualContents)[0])>::type> comparator;
    for(std::size_t i = 0; i != _actualContents->size(); ++i)
        if(comparator((*_actualContents)[i], (*_expectedContents)[i]) & ComparisonStatusFlag::Failed)
            return ComparisonStatusFlag::Failed;

    return {};
}

template<class T> void Comparator<Compare::Container<T>>::printMessage(ComparisonStatusFlags, Utility::Debug& out, const char* actual, const char* expected) const {
    out << "Containers" << actual << "and" << expected << "have different";
    if(_actualContents->size() != _expectedContents->size())
        out << "size, actual" << _actualContents->size() << "but" << _expectedContents->size() << "expected. Actual contents:\n       ";
    else
        out << "contents, actual:\n       ";

    out << *_actualContents << Utility::Debug::newline << "        but expected\n       " << *_expectedContents << Utility::Debug::newline << "       ";

    Comparator<typename std::decay<decltype((*_actualContents)[0])>::type> comparator;
    for(std::size_t i = 0, end = Implementation::max(_actualContents->size(), _expectedContents->size()); i != end; ++i) {
        if(_actualContents->size() > i && _expectedContents->size() > i &&
            !(comparator((*_actualContents)[i], (*_expectedContents)[i]) & ComparisonStatusFlag::Failed)) continue;

        if(_actualContents->size() <= i)
            out << "Expected has" << (*_expectedContents)[i];
        else if(_expectedContents->size() <= i)
            out << "Actual has" << (*_actualContents)[i];
        else
            out << "Actual" << (*_actualContents)[i] << "but" << (*_expectedContents)[i] << "expected";

        out << "on position" << i << Utility::Debug::nospace << ".";
        break;
    }
}
#endif

}}

#endif
