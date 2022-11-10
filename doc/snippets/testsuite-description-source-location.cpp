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

#include "Corrade/Containers/ArrayView.h"
#include "Corrade/Containers/Pair.h"
#include "Corrade/Containers/String.h"
#include "Corrade/TestSuite/Tester.h"
#include "Corrade/Utility/Path.h"

namespace {

using namespace Corrade;

Containers::StringView fileExtension(Containers::StringView filename) {
    return Utility::Path::splitExtension(filename).second();
}

struct PathTest: TestSuite::Tester {
    explicit PathTest();

    void extension();
};

/** [0] */
const struct {
    TestSuite::TestCaseDescriptionSourceLocation name;
    const char* filename;
    const char* ext;
} ExtensionData[]{
    {"simple", "file.txt", ".txt"},
    {"no extension", "Documents", ""},
    {"two extensions", "data.tar.gz", ".tar.gz"},
    {"directory with a dot", "/etc/conf.d/samba", ""},
};

PathTest::PathTest() {
    addInstancedTests({&PathTest::extension},
        Containers::arraySize(ExtensionData));
}

void PathTest::extension() {
    auto&& data = ExtensionData[testCaseInstanceId()];
    setTestCaseDescription(data.name);

    CORRADE_COMPARE(fileExtension(data.filename), data.ext);
}
/** [0] */

}

CORRADE_TEST_MAIN(PathTest)
