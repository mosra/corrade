#ifndef Corrade_TestSuite_Compare_SortedContainer_h
#define Corrade_TestSuite_Compare_SortedContainer_h
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
 * @brief Class @ref Corrade::TestSuite::Compare::SortedContainer
 * @m_since{2019,10}
 */

#include <algorithm> /* std::sort() */

/* Given there's <algorithm> already, it won't really help much if we'd PIMPL
   the Array include */
#include "Corrade/Containers/Array.h"
#include "Corrade/Containers/constructHelpers.h"
#include "Corrade/TestSuite/Compare/Container.h"

namespace Corrade { namespace TestSuite {

namespace Compare {

/**
@brief Pseudo-type for comparing sorted container contents

Compared to @ref Container the containers are sorted before performing the
comparison, making it possible to compare against expected contents even when
any side may be in a random order (such as when listing filesystem directory
contents). Can be also used to compare contents of containers that don't
provide random access (such as @ref std::list or @ref std::map) or have
an unspecified order (such as @ref std::unordered_map). Example usage:

@snippet TestSuite.cpp Compare-SortedContainer

The operation is performed by first copying contents of both containers to new
@ref Containers::Array instances, performing @ref std::sort() on them and then
passing them to @ref Container for doing actual comparison. The stored types
are expected to implement @cpp operator<() @ce and be copyable. The container
itself doesn't need to be copyable.

See @ref TestSuite-Comparator-pseudo-types for more information.
*/
template<class T> class SortedContainer: public Container<T> {};

namespace Implementation {
    template<class T> using UnderlyingContainerType = typename std::decay<decltype(*std::declval<T>().begin())>::type;
}

}

#ifndef DOXYGEN_GENERATING_OUTPUT
template<class T> class Comparator<Compare::SortedContainer<T>>: public Comparator<Compare::Container<Containers::Array<Compare::Implementation::UnderlyingContainerType<T>>>> {
    public:
        ComparisonStatusFlags operator()(const T& actual, const T& expected);

    private:
        Containers::Array<Compare::Implementation::UnderlyingContainerType<T>> _actualSorted, _expectedSorted;
};

template<class T> ComparisonStatusFlags Comparator<Compare::SortedContainer<T>>::operator()(const T& actual, const T& expected) {
    /* Copy the container contents to an Array first, as T itself might not be
       copyable (such as Array itself) or sortable (such as std::unordered_map).
       Using a NoInit'd array to avoid issues with types that don't have a
       default constructor. */
    _actualSorted = Containers::Array<Compare::Implementation::UnderlyingContainerType<T>>{NoInit, actual.size()};
    _expectedSorted = Containers::Array<Compare::Implementation::UnderlyingContainerType<T>>{NoInit, expected.size()};

    /* Using a range-for as T itself might not have index-based access (such as
       std::map); using the construct() helper instead of a raw new() to avoid
       issues with plain structs on certain compilers. See its docs for
       details. */
    std::size_t actualOffset = 0;
    for(const Compare::Implementation::UnderlyingContainerType<T>& i: actual)
        Containers::Implementation::construct(_actualSorted.data()[actualOffset++], i);
    std::size_t expectedOffset = 0;
    for(const Compare::Implementation::UnderlyingContainerType<T>& i: expected)
        Containers::Implementation::construct(_expectedSorted.data()[expectedOffset++], i);

    std::sort(_actualSorted.begin(), _actualSorted.end());
    std::sort(_expectedSorted.begin(), _expectedSorted.end());

    return Comparator<Compare::Container<Containers::Array<Compare::Implementation::UnderlyingContainerType<T>>>>::operator()(_actualSorted, _expectedSorted);
}
#endif

}}

#endif
