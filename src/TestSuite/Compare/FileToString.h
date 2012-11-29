#ifndef Corrade_TestSuite_Compare_FileToString_h
#define Corrade_TestSuite_Compare_FileToString_h
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
 * @brief Class Corrade::TestSuite::Compare::FileToString
 */

#include "TestSuite/Comparator.h"

#include "TestSuite/corradeTestSuiteVisibility.h"

namespace Corrade { namespace TestSuite {

namespace Compare {

/**
@brief Pseudo-type for comparing file contents to string

Prints the length of both files (if they are different) and prints value
and position of first different character in both files. Example usage:
@code
CORRADE_COMPARE_AS("actual.txt", "expected file contents", Compare::FileToString);
@endcode

See @ref Comparator-pseudo-types for more information.
*/
class FileToString {};

}

#ifndef DOXYGEN_GENERATING_OUTPUT
template<> class CORRADE_TESTSUITE_EXPORT Comparator<Compare::FileToString> {
    public:
        inline Comparator(): state(State::ReadError) {}

        bool operator()(const std::string& filename, const std::string& expectedContents);

        void printErrorMessage(Utility::Error& e, const std::string& actual, const std::string& expected) const;

    private:
        enum class State {
            Success,
            ReadError
        };

        State state;
        std::string filename,
            actualContents, expectedContents;
};
#endif

}}

#endif
