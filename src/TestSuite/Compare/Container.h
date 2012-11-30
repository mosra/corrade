#ifndef Corrade_TestSuite_Compare_Container_h
#define Corrade_TestSuite_Compare_Container_h
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
 * @brief Class Corrade::TestSuite::Compare::Container
 */

#include <algorithm>

#include "TestSuite/Comparator.h"

namespace Corrade { namespace TestSuite {

namespace Compare {
    /**
     * @brief %Container comparison method
     *
     * @see Container
     */
    enum class ContainerMethod {
        Unsorted,   /**< Don't sort the containers before comparison */
        Sorted      /**< Sort the containers before comparison */
    };

    template<class> class Container;
}

#ifndef DOXYGEN_GENERATING_OUTPUT
template<class T> class Comparator<Compare::Container<T>> {
    public:
        inline Comparator(Compare::ContainerMethod method = Compare::ContainerMethod::Unsorted): method(method) {}

        inline bool operator()(const T& actual, const T& expected) {
            actualContents = actual;
            expectedContents = expected;

            if(method == Compare::ContainerMethod::Sorted) {
                std::sort(actualContents.begin(), actualContents.end());
                std::sort(expectedContents.begin(), expectedContents.end());
            }

            return actualContents == expectedContents;
        }

        void printErrorMessage(Utility::Error& e, const std::string& actual, const std::string& expected) const {
            e << "Containers" << actual << "and" << expected << "have different";
            if(actualContents.size() != expectedContents.size())
                e << "size, actual" << actualContents.size() << "but" << expectedContents.size() << "expected.";
            else
                e << "contents.";

            for(std::size_t i = 0, end = std::max(actualContents.size(), expectedContents.size()); i != end; ++i) {
                if(actualContents.size() > i && expectedContents.size() > i && actualContents[i] == expectedContents[i]) continue;

                if(actualContents.size() <= i)
                    e << "Expected has" << expectedContents[i];
                else if(expectedContents.size() <= i)
                    e << "Actual has" << actualContents[i];
                else
                    e << "Actual" << actualContents[i] << "but" << expectedContents[i] << "expected";

                e << "on position" << i;
                e.setFlag(Utility::Debug::SpaceAfterEachValue, false);
                e << '.';
                e.setFlag(Utility::Debug::SpaceAfterEachValue, true);

                break;
            }
        }

    private:
        Compare::ContainerMethod method;
        T actualContents, expectedContents;
};
#endif

namespace Compare {

/**
@brief Pseudo-type for comparing container contents

Prints the length of both containers (if they are different) and prints value
of first different item in both containers. Example usage:
@code
std::vector<int> a, b;
CORRADE_COMPARE_AS(a, b, Compare::Container);
@endcode

It is also possible to sort the containers before comparison:
@code
CORRADE_COMPARE_WITH(a, b, Compare::Container<std::vector<int>>(Compare::Container::Method::Sorted));
@endcode

See @ref Comparator-pseudo-types for more information.
@todo Is it possible to make sorted comparison more convenient?
*/
template<class T> class Container {
    public:
        /**
         * @brief Constructor
         * @param method    Comparison method
         *
         * See class documentation for more information.
         */
        Container(ContainerMethod method = ContainerMethod::Unsorted): c(method) {}

        #ifndef DOXYGEN_GENERATING_OUTPUT
        inline Comparator<Compare::Container<T>>& comparator() { return c; }
        #endif

    private:
        Comparator<Compare::Container<T>> c;
};

}

}}

#endif
