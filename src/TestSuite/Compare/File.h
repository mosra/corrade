#ifndef Corrade_TestSuite_Compare_File_h
#define Corrade_TestSuite_Compare_File_h
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
 * @brief Class Corrade::TestSuite::Compare::File
 */

#include "TestSuite/Comparator.h"

#include "TestSuite/corradeTestSuiteVisibility.h"

namespace Corrade { namespace TestSuite {

namespace Compare {
    class File;
}

#ifndef DOXYGEN_GENERATING_OUTPUT
template<> class CORRADE_TESTSUITE_EXPORT Comparator<Compare::File> {
    public:
        inline Comparator(const std::string& pathPrefix = ""): actualState(State::ReadError), expectedState(State::ReadError), pathPrefix(pathPrefix) {}

        bool operator()(const std::string& actualFilename, const std::string& expectedFilename);

        void printErrorMessage(Utility::Error& e, const std::string& actual, const std::string& expected) const;

    private:
        enum class State {
            Success,
            ReadError
        };

        State actualState, expectedState;
        std::string pathPrefix, actualFilename, expectedFilename,
            actualContents, expectedContents;
};
#endif

namespace Compare {

/**
@brief Pseudo-type for comparing file contents

Prints the length of both files (if they are different) and prints value
and position of first different character in both files. Example usage:
@code
CORRADE_COMPARE_AS("actual.txt", "expected.txt", Compare::File);
@endcode

If the files have the same path prefix, you can use CORRADE_COMPARE_WITH()
macro and pass the prefix to the constructor:
@code
CORRADE_COMPARE_WITH("actual.txt", "expected.txt", Compare::File("/common/path"));
@endcode

See @ref Comparator-pseudo-types and @ref Comparator-parameters for more
information.
*/
class File {
    public:
        /**
         * @brief Constructor
         * @param pathPrefix    Path prefix common for both files
         *
         * See class documentation for more information.
         */
        File(const std::string& pathPrefix = ""): c(pathPrefix) {}

        #ifndef DOXYGEN_GENERATING_OUTPUT
        inline Comparator<Compare::File> comparator() { return c; }
        #endif

    private:
        Comparator<Compare::File> c;
};

}

}}

#endif
