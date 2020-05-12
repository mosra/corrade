#ifndef Corrade_TestSuite_Compare_StringToFile_h
#define Corrade_TestSuite_Compare_StringToFile_h
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

Prints the length of both files (if they are different) and prints the value
and position of the first different character in both files. Filename is
expected to be in UTF-8. Example usage:

@snippet TestSuite.cpp Compare-StringToFile

See @ref TestSuite-Comparator-pseudo-types for more information.

@section TestSuite-Compare-StringToFile-save-diagnostic Saving files for failed comparisons

The comparator supports the @ref TestSuite-Tester-save-diagnostic "--save-diagnostic option"
--- if the comparison fails, it saves actual file contents to given directory
with a filename matching the expected file. You can use it to perform a manual
data comparison with an external tool or for example to quickly update expected
test data --- point the option to the directory with expected test files and
let the test overwrite them with actual results. The @ref StringToFile variant
supports the same.

@see @ref Compare::File, @ref Compare::FileToString
*/
class StringToFile {};

}

#ifndef DOXYGEN_GENERATING_OUTPUT
template<> class CORRADE_TESTSUITE_EXPORT Comparator<Compare::StringToFile> {
    public:
        Comparator();

        ComparisonStatusFlags operator()(const std::string& actualContents, const std::string& filename);

        void printMessage(ComparisonStatusFlags flags, Utility::Debug& out, const char* actual, const char* expected) const;

        void saveDiagnostic(ComparisonStatusFlags flags, Utility::Debug& out, const std::string& path);

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
