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

#include "Container.h"

namespace Corrade { namespace TestSuite { namespace Implementation {

void ContainerComparatorBase::printMessage(ComparisonStatusFlags, Utility::Debug& out, const char* const actual, const char* const expected, void(*printer)(Utility::Debug&, const void*), void(*itemPrinter)(Utility::Debug&, const void*, std::size_t)) const {
    CORRADE_INTERNAL_ASSERT(_actualContents && _expectedContents);

    out << "Containers" << actual << "and" << expected << "have different";
    if(_actualContentsSize != _expectedContentsSize)
        out << "size, actual" << _actualContentsSize << "but" << _expectedContentsSize << "expected. Actual contents:\n       ";
    else
        out << "contents, actual:\n       ";

    printer(out, _actualContents);
    out << Utility::Debug::newline << "        but expected\n       ";
    printer(out, _expectedContents);
    out << Utility::Debug::newline << "       ";

    if(_actualContentsSize <= _firstDifferent) {
        out << "Expected has";
        itemPrinter(out, _expectedContents, _firstDifferent);
    } else if(_expectedContentsSize <= _firstDifferent) {
        out << "Actual has";
        itemPrinter(out, _actualContents, _firstDifferent);
    } else {
        out << "Actual";
        itemPrinter(out, _actualContents, _firstDifferent);
        out << "but";
        itemPrinter(out, _expectedContents, _firstDifferent);
        out << "expected";
    }

    out << "on position" << _firstDifferent << Utility::Debug::nospace << ".";
}

}}}
