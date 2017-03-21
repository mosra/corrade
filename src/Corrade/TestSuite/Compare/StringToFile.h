#ifndef Corrade_TestSuite_Compare_StringToFile_h
#define Corrade_TestSuite_Compare_StringToFile_h
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
 * @brief Class @ref Corrade::TestSuite::Compare::StringToFile
 */

#include <string>

#include "Corrade/TestSuite/TestSuite.h"
#include "Corrade/TestSuite/visibility.h"
#include "Corrade/Utility/Utility.h"

namespace Corrade { namespace TestSuite {

namespace Compare {

/**
@brief Pseudo-type for comparing string to file contents

Prints the length of both files (if they are different) and prints value
and position of first different character in both files. Filename is expected
to be in UTF-8. Example usage:
@code
CORRADE_COMPARE_AS("actual file contents", "expected.txt", Compare::StringToFile);
@endcode

See @ref TestSuite-Comparator-pseudo-types for more information.
@see @ref Compare::File, @ref Compare::FileToString
*/
class StringToFile {};

}

#ifndef DOXYGEN_GENERATING_OUTPUT
template<> class CORRADE_TESTSUITE_EXPORT Comparator<Compare::StringToFile> {
    public:
        Comparator();

        bool operator()(const std::string& actualContents, const std::string& filename);

        void printErrorMessage(Utility::Error& e, const std::string& actual, const std::string& expected) const;

    private:
        enum class State {
            Success,
            ReadError
        };

        State _state;
        std::string _filename,
            _actualContents, _expectedContents;
};
#endif

}}

#endif
