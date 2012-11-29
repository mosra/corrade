/*
    Copyright © 2007, 2008, 2009, 2010, 2011, 2012
              Vladimír Vondruš <mosra@centrum.cz>

    This file is part of Corrade.

    Corrade is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License version 3
    only, as published by the Free Software Foundation.

    Corrade is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU Lesser General Public License version 3 for more details.
*/

#include "File.h"

#include <fstream>

#include "Utility/Directory.h"

using Corrade::Utility::Directory;

namespace Corrade { namespace TestSuite {

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
        e << "Actual file" << actual << '(' + actualFilename + ')' << "cannot be read.";
        return;
    }

    if(expectedState != State::Success) {
        e << "Expected file" << expected << '(' + expectedFilename + ')' << "cannot be read.";
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
            e << "Expected has character" << expectedContents[i];
        else if(expectedContents.size() <= i)
            e << "Actual has character" << actualContents[i];
        else
            e << "Actual character" << actualContents[i] << "but" << expectedContents[i] << "expected";

        e << "on position" << i;
        e.setFlag(Utility::Debug::SpaceAfterEachValue, false);
        e << '.';
        e.setFlag(Utility::Debug::SpaceAfterEachValue, true);

        break;
    }
}

}}
