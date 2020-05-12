#ifndef Corrade_TestSuite_Compare_File_h
#define Corrade_TestSuite_Compare_File_h
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
 * @brief Class @ref Corrade::TestSuite::Compare::File
 */

#include <string>

#include "Corrade/TestSuite/TestSuite.h"
#include "Corrade/TestSuite/visibility.h"
#include "Corrade/Utility/Utility.h"

namespace Corrade { namespace TestSuite {

namespace Compare { class File; }

#ifndef DOXYGEN_GENERATING_OUTPUT
template<> class CORRADE_TESTSUITE_EXPORT Comparator<Compare::File> {
    public:
        explicit Comparator(std::string pathPrefix = {});

        ComparisonStatusFlags operator()(const std::string& actualFilename, const std::string& expectedFilename);

        void printMessage(ComparisonStatusFlags flags, Utility::Debug& out, const char* actual, const char* expected) const;

        void saveDiagnostic(ComparisonStatusFlags flags, Utility::Debug& out, const std::string& path);

    private:
        enum class State {
            Success,
            ReadError
        };

        State _actualState, _expectedState;
        std::string _pathPrefix, _actualFilename, _expectedFilename,
            _actualContents, _expectedContents;
};
#endif

namespace Compare {

/**
@brief Pseudo-type for comparing file contents

Prints the length of both files (if they are different) and prints the value
and position of the first different character in both files. Filenames are
expected to be in UTF-8. Example usage:

@snippet TestSuite.cpp Compare-File

If the files have the same path prefix, you can use @ref CORRADE_COMPARE_WITH()
and pass the prefix to the constructor:

@snippet TestSuite.cpp Compare-File-prefix

See @ref TestSuite-Comparator-pseudo-types and @ref TestSuite-Comparator-parameters
for more information.

@section TestSuite-Compare-File-save-diagnostic Saving files for failed comparisons

The comparator supports the @ref TestSuite-Tester-save-diagnostic "--save-diagnostic option"
--- if the comparison fails, it saves actual file contents to given directory
with a filename matching the expected file. You can use it to perform a manual
data comparison with an external tool or for example to quickly update expected
test data --- point the option to the directory with expected test files and
let the test overwrite them with actual results. The @ref StringToFile variant
supports the same.

@see @ref FileToString, @ref StringToFile
*/
class CORRADE_TESTSUITE_EXPORT File {
    public:
        /**
         * @brief Constructor
         * @param pathPrefix    Path prefix common for both files
         *
         * See class documentation for more information.
         */
        explicit File(const std::string& pathPrefix = {});

        #ifndef DOXYGEN_GENERATING_OUTPUT
        Comparator<Compare::File> comparator();
        #endif

    private:
        Comparator<Compare::File> _c;
};

}

}}

#endif
