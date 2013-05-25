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

#include "File.h"

#include <fstream>

#include "Utility/Directory.h"

using Corrade::Utility::Directory;

namespace Corrade { namespace TestSuite {

Comparator<Compare::File>::Comparator(const std::string& pathPrefix): actualState(State::ReadError), expectedState(State::ReadError), pathPrefix(pathPrefix) {}

bool Comparator<Compare::File>::operator()(const std::string& actualFilename, const std::string& expectedFilename) {
    this->actualFilename = Directory::join(pathPrefix, actualFilename);
    this->expectedFilename = Directory::join(pathPrefix, expectedFilename);

    std::ifstream actualIn(this->actualFilename);
    std::ifstream expectedIn(this->expectedFilename);

    if(!actualIn.good())
        return false;
    if(!expectedIn.good()) {
        actualState = State::Success;
        return false;
    }

    actualIn.seekg(0, std::ios::end);
    actualContents.reserve(actualIn.tellg());
    actualIn.seekg(0, std::ios::beg);

    expectedIn.seekg(0, std::ios::end);
    actualContents.reserve(expectedIn.tellg());
    expectedIn.seekg(0, std::ios::beg);

    actualContents.assign((std::istreambuf_iterator<char>(actualIn)), std::istreambuf_iterator<char>());
    expectedContents.assign((std::istreambuf_iterator<char>(expectedIn)), std::istreambuf_iterator<char>());

    actualState = State::Success;
    expectedState = State::Success;

    return actualContents == expectedContents;
}

void Comparator<Compare::File>::printErrorMessage(Utility::Error& e, const std::string& actual, const std::string& expected) const {
    if(actualState != State::Success) {
        e << "Actual file" << actual << "(" + actualFilename + ")" << "cannot be read.";
        return;
    }

    if(expectedState != State::Success) {
        e << "Expected file" << expected << "(" + expectedFilename + ")" << "cannot be read.";
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

namespace Compare {

File::File(const std::string& pathPrefix): c(pathPrefix) {}

Comparator<File> File::comparator() { return c; }

}

}}
