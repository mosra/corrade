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

#include "StringToFile.h"

#include <cstddef>

#include "Corrade/Containers/ArrayView.h"
#include "Corrade/Containers/Optional.h"
#include "Corrade/Containers/Pair.h"
#include "Corrade/Containers/String.h"
#include "Corrade/Utility/Math.h"
#include "Corrade/Utility/Path.h"
#include "Corrade/TestSuite/Tester.h"

namespace Corrade { namespace TestSuite {

namespace {

enum class Result {
    Success,
    ReadError
};

}

struct Comparator<Compare::StringToFile>::State {
    Result result;
    /* The whole comparison is done in a single expression so the filename and
       actual contents can stay as views, however expected contents are fetched
       from a file so they have be owned */
    Containers::StringView filename, actualContents;
    Containers::String expectedContents;
};

#ifndef DOXYGEN_GENERATING_OUTPUT
Comparator<Compare::StringToFile>::Comparator(): _state{InPlaceInit} {
    _state->result = Result::ReadError;
}

Comparator<Compare::StringToFile>::~Comparator() = default;

ComparisonStatusFlags Comparator<Compare::StringToFile>::operator()(const Containers::StringView actualContents, const Containers::StringView filename) {
    _state->filename = filename;

    /* Save the actual file contents before the expected so if the expected
       file can't be read, we can still save actual file contents */
    _state->actualContents = actualContents;

    /* If this fails, we already have the actual contents so we can save them */
    Containers::Optional<Containers::String> expectedContents = Utility::Path::readString(filename);
    if(!expectedContents)
        return ComparisonStatusFlag::Diagnostic|ComparisonStatusFlag::Failed;

    _state->expectedContents = *Utility::move(expectedContents);
    _state->result = Result::Success;

    return actualContents == _state->expectedContents ? ComparisonStatusFlags{} :
        ComparisonStatusFlag::Diagnostic|ComparisonStatusFlag::Failed;
}

void Comparator<Compare::StringToFile>::printMessage(ComparisonStatusFlags, Utility::Debug& out, const char* actual, const char* expected) const {
    if(_state->result != Result::Success) {
        out << "File" << expected << "(" + _state->filename + ")" << "cannot be read.";
        return;
    }

    out << "Files" << actual << "and" << expected << "have different";
    if(_state->actualContents.size() != _state->expectedContents.size())
        out << "size, actual" << _state->actualContents.size() << "but" << _state->expectedContents.size() << "expected.";
    else
        out << "contents.";

    for(std::size_t i = 0, end = Utility::max(_state->actualContents.size(), _state->expectedContents.size()); i != end; ++i) {
        if(_state->actualContents.size() > i && _state->expectedContents.size() > i && _state->actualContents[i] == _state->expectedContents[i]) continue;

        if(_state->actualContents.size() <= i)
            out << "Expected has character" << _state->expectedContents.slice(i, i + 1);
        else if(_state->expectedContents.size() <= i)
            out << "Actual has character" << _state->actualContents.slice(i, i + 1);
        else
            out << "Actual character" << _state->actualContents.slice(i, i + 1) << "but" << _state->expectedContents.slice(i, i + 1) << "expected";

        out << "on position" << i << Utility::Debug::nospace << ".";
        break;
    }
}

void Comparator<Compare::StringToFile>::saveDiagnostic(ComparisonStatusFlags, Utility::Debug& out, const Containers::StringView path) {
    Containers::String filename = Utility::Path::join(path, Utility::Path::split(_state->filename).second());
    if(Utility::Path::write(filename, _state->actualContents))
        out << "->" << filename;
}
#endif

}}
