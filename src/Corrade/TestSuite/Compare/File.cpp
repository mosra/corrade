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

#include "File.h"

#include <cstddef>
#include <utility>
#ifdef _MSC_VER
#include <algorithm> /* std::max() */
#endif

#include "Corrade/TestSuite/Comparator.h"
#include "Corrade/Utility/DebugStl.h"
#include "Corrade/Utility/Directory.h"

namespace Corrade { namespace TestSuite {

#ifndef DOXYGEN_GENERATING_OUTPUT
Comparator<Compare::File>::Comparator(std::string pathPrefix): _actualState{State::ReadError}, _expectedState{State::ReadError}, _pathPrefix{std::move(pathPrefix)} {}

ComparisonStatusFlags Comparator<Compare::File>::operator()(const std::string& actualFilename, const std::string& expectedFilename) {
    _actualFilename = Utility::Directory::join(_pathPrefix, actualFilename);
    _expectedFilename = Utility::Directory::join(_pathPrefix, expectedFilename);

    if(!Utility::Directory::exists(_actualFilename))
        return ComparisonStatusFlag::Failed;

    _actualState = State::Success;

    /* Read the actual file contents before the expected so if the expected
       file can't be read, we can still save actual file contents */
    _actualContents = Utility::Directory::readString(_actualFilename);

    /* If this fails, we already have the actual contents so we can save them */
    if(!Utility::Directory::exists(_expectedFilename))
        return ComparisonStatusFlag::Diagnostic|ComparisonStatusFlag::Failed;

    _expectedContents = Utility::Directory::readString(_expectedFilename);
    _expectedState = State::Success;

    return _actualContents == _expectedContents ? ComparisonStatusFlags{} :
        ComparisonStatusFlag::Diagnostic|ComparisonStatusFlag::Failed;
}

void Comparator<Compare::File>::printMessage(ComparisonStatusFlags, Utility::Debug& out, const char* actual, const char* expected) const {
    if(_actualState != State::Success) {
        out << "Actual file" << actual << "(" + _actualFilename + ")" << "cannot be read.";
        return;
    }

    if(_expectedState != State::Success) {
        out << "Expected file" << expected << "(" + _expectedFilename + ")" << "cannot be read.";
        return;
    }

    out << "Files" << actual << "and" << expected << "have different";
    if(_actualContents.size() != _expectedContents.size())
        out << "size, actual" << _actualContents.size() << "but" << _expectedContents.size() << "expected.";
    else
        out << "contents.";

    for(std::size_t i = 0, end = std::max(_actualContents.size(), _expectedContents.size()); i != end; ++i) {
        if(_actualContents.size() > i && _expectedContents.size() > i && _actualContents[i] == _expectedContents[i]) continue;

        /** @todo do this without std::string */
        if(_actualContents.size() <= i)
            out << "Expected has character" << std::string() + _expectedContents[i];
        else if(_expectedContents.size() <= i)
            out << "Actual has character" << std::string() + _actualContents[i];
        else
            out << "Actual character" << std::string() + _actualContents[i] << "but" << std::string() + _expectedContents[i] << "expected";

        out << "on position" << i << Utility::Debug::nospace << ".";
        break;
    }
}

void Comparator<Compare::File>::saveDiagnostic(ComparisonStatusFlags, Utility::Debug& out, const std::string& path) {
    std::string filename = Utility::Directory::join(path, Utility::Directory::filename(_expectedFilename));
    if(Utility::Directory::writeString(filename, _actualContents))
        out << "->" << filename;
}
#endif

namespace Compare {

File::File(const std::string& pathPrefix): _c{pathPrefix} {}

#ifndef DOXYGEN_GENERATING_OUTPUT
Comparator<File> File::comparator() { return _c; }
#endif

}

}}
