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

#include <string>

#include "Corrade/Containers/String.h"
#include "Corrade/TestSuite/Comparator.h"
#include "Corrade/Utility/DebugStl.h"
#include "Corrade/Utility/Path.h"

class FileContents;

namespace Corrade { namespace TestSuite {

#ifndef CORRADE_NO_ASSERT
/* [Comparator-save-diagnostic] */
template<> class Comparator<FileContents> {
    public:
        ComparisonStatusFlags operator()(Containers::StringView actual, Containers::StringView expected);

        // ...

        void saveDiagnostic(ComparisonStatusFlags flags, Utility::Debug& out, Containers::StringView path) {
            CORRADE_INTERNAL_ASSERT(flags & ComparisonStatusFlag::Diagnostic);
            Containers::String filename = Utility::Path::join(path, _expectedFilename);
            if(Utility::Path::write(filename, _actualContents))
                out << "->" << filename;
        }

    private:
        Containers::String _actualContents;
        // ...
        Containers::String _expectedFilename;
};
/* [Comparator-save-diagnostic] */
static_assert(Implementation::CanSaveDiagnostic<Comparator<FileContents>>::value,
    "this snippet is broken");
#endif

}}

/* To prevent macOS ranlib complaining that there are no symbols */
int main() {}
