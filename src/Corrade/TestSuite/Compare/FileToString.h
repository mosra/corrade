#ifndef Corrade_TestSuite_Compare_FileToString_h
#define Corrade_TestSuite_Compare_FileToString_h
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
 * @brief Class @ref Corrade::TestSuite::Compare::FileToString
 */

#include "Corrade/Containers/Pointer.h"
/* The include is not strictly needed, but it would only mean the users would
   then have to include it on their own -- as there's no way to use this
   comparator without a StringView */
#include "Corrade/Containers/StringView.h"
#include "Corrade/TestSuite/TestSuite.h"
#include "Corrade/TestSuite/visibility.h"
#include "Corrade/Utility/Utility.h"

#ifdef CORRADE_BUILD_DEPRECATED
/* The arguments used to be a std::string, so provide implicit conversion to a
   StringView */
#include "Corrade/Containers/StringStl.h"
#endif

namespace Corrade { namespace TestSuite {

namespace Compare {

/**
@brief Pseudo-type for comparing file contents to string

Prints the length of both files (if they are different) and prints the value
and position of the first different character in both files. Filename is
expected to be in UTF-8. Example usage:

@snippet TestSuite.cpp Compare-FileToString

See @ref TestSuite-Comparator-pseudo-types for more information.

Unlike @ref File and @ref StringToFile, this comparator *doesn't* support the
@ref TestSuite-Tester-save-diagnostic "--save-diagnostic option", due to the
fact that the comparison is done against a string and so producing a file isn't
that helpful as in the other two variants.

@see @ref File, @ref StringToFile
*/
class FileToString {};

}

#ifndef DOXYGEN_GENERATING_OUTPUT
template<> class CORRADE_TESTSUITE_EXPORT Comparator<Compare::FileToString> {
    public:
        explicit Comparator();

        ~Comparator();

        ComparisonStatusFlags operator()(Containers::StringView filename, Containers::StringView expectedContents);

        void printMessage(ComparisonStatusFlags flags, Utility::Debug& out, const char* actual, const char* expected) const;

    private:
        struct CORRADE_TESTSUITE_LOCAL State;
        Containers::Pointer<State> _state;
};
#endif

}}

#endif
