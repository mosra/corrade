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

#include "FileToString.h"

#include <cstddef>

#include "Corrade/Containers/Optional.h"
#include "Corrade/Containers/String.h"
#include "Corrade/TestSuite/Comparator.h"
#include "Corrade/Utility/Math.h"
#include "Corrade/Utility/Path.h"

namespace Corrade { namespace TestSuite {

namespace {

enum class Result {
    Success,
    ReadError
};

}

struct Comparator<Compare::FileToString>::State {
    Result result;
    /* The whole comparison is done in a single expression so the filename and
       expected contents can stay as views, however actual contents are fetched
       from a file so they have be owned */
    Containers::StringView filename;
    Containers::String actualContents;
    Containers::StringView expectedContents;
};

Comparator<Compare::FileToString>::Comparator(): _state{InPlaceInit} {
    _state->result = Result::ReadError;
}

Comparator<Compare::FileToString>::~Comparator() = default;

ComparisonStatusFlags Comparator<Compare::FileToString>::operator()(const Containers::StringView filename, const Containers::StringView expectedContents) {
    _state->filename = filename;

    Containers::Optional<Containers::String> actualContents = Utility::Path::readString(filename);
    if(!actualContents)
        return ComparisonStatusFlag::Failed;

    _state->actualContents = *Utility::move(actualContents);
    _state->expectedContents = expectedContents;
    _state->result = Result::Success;

    return _state->actualContents == expectedContents ? ComparisonStatusFlags{} :
        ComparisonStatusFlag::Failed;
}

void Comparator<Compare::FileToString>::printMessage(ComparisonStatusFlags, Utility::Debug& out, const char* actual, const char* expected) const {
    if(_state->result != Result::Success) {
        out << "File" << actual << "(" + _state->filename + ")" << "cannot be read.";
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

}}
