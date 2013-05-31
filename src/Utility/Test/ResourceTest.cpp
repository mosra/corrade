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

#include <fstream>
#include <sstream>
#include <tuple>

#include "TestSuite/Tester.h"
#include "TestSuite/Compare/StringToFile.h"
#include "Utility/Directory.h"
#include "Utility/Resource.h"

#include "testConfigure.h"

namespace Corrade { namespace Utility { namespace Test {

class ResourceTest: public TestSuite::Tester {
    public:
        ResourceTest();

        void compile();
        void compileEmptyFile();
        void compileFrom();
        void get();
        void getInexistent();
};

ResourceTest::ResourceTest() {
    addTests({&ResourceTest::compile,
              &ResourceTest::compileEmptyFile,
              &ResourceTest::compileFrom,
              &ResourceTest::get,
              &ResourceTest::getInexistent});
}

void ResourceTest::compile() {
    /* Testing also null bytes and signed overflow, don't change binaries */
    std::ifstream predispositionIn(Directory::join(RESOURCE_TEST_DIR, "predisposition.bin"));
    std::ifstream consequenceIn(Directory::join(RESOURCE_TEST_DIR, "consequence.bin"));
    CORRADE_VERIFY(predispositionIn.good());
    CORRADE_VERIFY(consequenceIn.good());

    predispositionIn.seekg(0, std::ios::end);
    std::string predisposition(predispositionIn.tellg(), '\0');
    predispositionIn.seekg(0, std::ios::beg);
    predispositionIn.read(&predisposition[0], predisposition.size());

    consequenceIn.seekg(0, std::ios::end);
    std::string consequence(consequenceIn.tellg(), '\0');
    consequenceIn.seekg(0, std::ios::beg);
    consequenceIn.read(&consequence[0], consequence.size());

    std::vector<std::pair<std::string, std::string>> input{
        {"predisposition.bin", predisposition},
        {"consequence.bin", consequence}};
    CORRADE_COMPARE_AS(Resource::compile("ResourceTestData", "test", input),
                       Directory::join(RESOURCE_TEST_DIR, "compiled.cpp"),
                       TestSuite::Compare::StringToFile);
}

void ResourceTest::compileEmptyFile() {
    std::vector<std::pair<std::string, std::string>> input{
        {"empty.bin", ""}};
    CORRADE_COMPARE_AS(Resource::compile("ResourceTestData", "test", input),
                       Directory::join(RESOURCE_TEST_DIR, "compiledEmpty.cpp"),
                       TestSuite::Compare::StringToFile);
}

void ResourceTest::compileFrom() {
    std::ostringstream out;
    Debug::setOutput(&out);

    const std::string compiled = Resource::compileFrom("ResourceTestData",
        Directory::join(RESOURCE_TEST_DIR, "resources.conf"));
    CORRADE_COMPARE_AS(compiled, Directory::join(RESOURCE_TEST_DIR, "compiled.cpp"),
                       TestSuite::Compare::StringToFile);
    CORRADE_COMPARE(out.str(),
        "Reading file 1 of 2 in group 'test'\n"
        "    ../ResourceTestFiles/predisposition.bin\n"
        " -> predisposition.bin\n"
        "Reading file 2 of 2 in group 'test'\n"
        "    consequence.bin\n");
}

void ResourceTest::get() {
    Resource r("test");
    CORRADE_COMPARE_AS(r.get("predisposition.bin"),
                       Directory::join(RESOURCE_TEST_DIR, "predisposition.bin"),
                       TestSuite::Compare::StringToFile);
    CORRADE_COMPARE_AS(r.get("consequence.bin"),
                       Directory::join(RESOURCE_TEST_DIR, "consequence.bin"),
                       TestSuite::Compare::StringToFile);
}

void ResourceTest::getInexistent() {
    std::ostringstream out;
    Error::setOutput(&out);

    {
        Resource r("inexistentGroup");
        CORRADE_COMPARE(out.str(), "Utility::Resource: group 'inexistentGroup' was not found\n");
    }

    out.str({});

    {
        Resource r("test");
        CORRADE_VERIFY(r.get("inexistentFile").empty());
        CORRADE_COMPARE(out.str(), "Utility::Resource::get(): file 'inexistentFile' was not found in group 'test'\n");
    }

    Resource r("test");
    const unsigned char* data;
    std::size_t size;
    std::tie(data, size) = r.getRaw("inexistentFile");
    CORRADE_VERIFY(!data);
    CORRADE_VERIFY(!size);
}

}}}

CORRADE_TEST_MAIN(Corrade::Utility::Test::ResourceTest)
