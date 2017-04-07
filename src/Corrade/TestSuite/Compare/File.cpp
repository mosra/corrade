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

#include "File.h"

#include <cstddef>
#include <utility>
#ifdef _MSC_VER
#include <algorithm> /* std::max() */
#endif

#include "Corrade/Utility/Debug.h"
#include "Corrade/Utility/Directory.h"

namespace Corrade { namespace TestSuite {

Comparator<Compare::File>::Comparator(std::string pathPrefix): _actualState{State::ReadError}, _expectedState{State::ReadError}, _pathPrefix{std::move(pathPrefix)} {}

bool Comparator<Compare::File>::operator()(const std::string& actualFilename, const std::string& expectedFilename) {
    _actualFilename = Utility::Directory::join(_pathPrefix, actualFilename);
    _expectedFilename = Utility::Directory::join(_pathPrefix, expectedFilename);

    if(!Utility::Directory::fileExists(_actualFilename))
        return false;

    _actualState = State::Success;

    if(!Utility::Directory::fileExists(_expectedFilename))
        return false;

    _actualContents = Utility::Directory::readString(_actualFilename);
    _expectedContents = Utility::Directory::readString(_expectedFilename);
    _expectedState = State::Success;

    return _actualContents == _expectedContents;
}

void Comparator<Compare::File>::printErrorMessage(Utility::Error& e, const std::string& actual, const std::string& expected) const {
    if(_actualState != State::Success) {
        e << "Actual file" << actual << "(" + _actualFilename + ")" << "cannot be read.";
        return;
    }

    if(_expectedState != State::Success) {
        e << "Expected file" << expected << "(" + _expectedFilename + ")" << "cannot be read.";
        return;
    }

    e << "Files" << actual << "and" << expected << "have different";
    if(_actualContents.size() != _expectedContents.size())
        e << "size, actual" << _actualContents.size() << "but" << _expectedContents.size() << "expected.";
    else
        e << "contents.";

    for(std::size_t i = 0, end = std::max(_actualContents.size(), _expectedContents.size()); i != end; ++i) {
        if(_actualContents.size() > i && _expectedContents.size() > i && _actualContents[i] == _expectedContents[i]) continue;

        if(_actualContents.size() <= i)
            e << "Expected has character" << std::string() + _expectedContents[i];
        else if(_expectedContents.size() <= i)
            e << "Actual has character" << std::string() + _actualContents[i];
        else
            e << "Actual character" << std::string() + _actualContents[i] << "but" << std::string() + _expectedContents[i] << "expected";

        e << "on position" << i << Utility::Debug::nospace << ".";
        break;
    }
}

namespace Compare {

File::File(const std::string& pathPrefix): _c{pathPrefix} {}

#ifndef DOXYGEN_GENERATING_OUTPUT
Comparator<File> File::comparator() { return _c; }
#endif

}

}}
