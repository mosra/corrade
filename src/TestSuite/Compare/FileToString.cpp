/*
    This file is part of Corrade.

    Copyright © 2007, 2008, 2009, 2010, 2011, 2012, 2013
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

#include "FileToString.h"

#include "Utility/Directory.h"

namespace Corrade { namespace TestSuite {

Comparator<Compare::FileToString>::Comparator(): state(State::ReadError) {}

bool Comparator<Compare::FileToString>::operator()(const std::string& filename, const std::string& expectedContents) {
    this->filename = filename;

    if(!Utility::Directory::fileExists(filename)) return false;

    actualContents = Utility::Directory::readString(filename);
    this->expectedContents = expectedContents;
    state = State::Success;

    return actualContents == expectedContents;
}

void Comparator<Compare::FileToString>::printErrorMessage(Utility::Error& e, const std::string& actual, const std::string& expected) const {
    if(state != State::Success) {
        e << "File" << actual << "(" + filename + ")" << "cannot be read.";
        return;
    }

    e << "Files" << actual << "and" << expected << "have different";
    if(actualContents.size() != expectedContents.size())
        e << "size, actual" << actualContents.size() << "but" << expectedContents.size() << "expected.";
    else
        e << "contents.";

    for(std::size_t i = 0, end = std::max(actualContents.size(), expectedContents.size()); i != end; ++i) {
        if(actualContents.size() > i && expectedContents.size() > i && actualContents[i] == expectedContents[i]) continue;

        if(actualContents.size() <= i)
            e << "Expected has character" << std::string() + expectedContents[i];
        else if(expectedContents.size() <= i)
            e << "Actual has character" << std::string() + actualContents[i];
        else
            e << "Actual character" << std::string() + actualContents[i] << "but" << std::string() + expectedContents[i] << "expected";

        e << "on position" << i;
        e.setFlag(Utility::Debug::SpaceAfterEachValue, false);
        e << ".";
        e.setFlag(Utility::Debug::SpaceAfterEachValue, true);

        break;
    }

}

}}
