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

#include <sstream>

#include "Corrade/TestSuite/Tester.h"
#include "Corrade/TestSuite/Compare/Container.h"
#include "Corrade/TestSuite/Compare/StringToFile.h"
#include "Corrade/Utility/Directory.h"
#include "Corrade/Utility/Resource.h"

#include "configure.h"

namespace Corrade { namespace Utility { namespace Test {

struct ResourceTest: TestSuite::Tester {
    explicit ResourceTest();

    void compile();
    void compileNothing();
    void compileEmptyFile();

    void compileFrom();
    void compileFromUtf8Filenames();
    void compileFromNonexistentResource();
    void compileFromNonexistentFile();
    void compileFromEmptyGroup();
    void compileFromEmptyFilename();
    void compileFromEmptyAlias();

    void hasGroup();
    void list();
    void get();
    void getEmptyFile();
    void getNonexistent();
    void getNothing();

    void overrideGroup();
    void overrideGroupFallback();
    void overrideNonexistentFile();
    void overrideNonexistentGroup();
    void overrideDifferentGroup();
};

ResourceTest::ResourceTest() {
    addTests({&ResourceTest::compile,
              &ResourceTest::compileNothing,
              &ResourceTest::compileEmptyFile,

              &ResourceTest::compileFrom,
              &ResourceTest::compileFromUtf8Filenames,
              &ResourceTest::compileFromNonexistentResource,
              &ResourceTest::compileFromNonexistentFile,
              &ResourceTest::compileFromEmptyGroup,
              &ResourceTest::compileFromEmptyFilename,
              &ResourceTest::compileFromEmptyAlias,

              &ResourceTest::hasGroup,
              &ResourceTest::list,
              &ResourceTest::get,
              &ResourceTest::getEmptyFile,
              &ResourceTest::getNonexistent,
              &ResourceTest::getNothing,

              &ResourceTest::overrideGroup,
              &ResourceTest::overrideGroupFallback,
              &ResourceTest::overrideNonexistentFile,
              &ResourceTest::overrideNonexistentGroup,
              &ResourceTest::overrideDifferentGroup});
}

void ResourceTest::compile() {
    /* Testing also null bytes and signed overflow, don't change binaries */
    std::vector<std::pair<std::string, std::string>> input{
        {"predisposition.bin", Directory::readString(Directory::join(RESOURCE_TEST_DIR, "predisposition.bin"))},
        {"consequence.bin", Directory::readString(Directory::join(RESOURCE_TEST_DIR, "consequence.bin"))}};
    CORRADE_COMPARE_AS(Resource::compile("ResourceTestData", "test", input),
                       Directory::join(RESOURCE_TEST_DIR, "compiled.cpp"),
                       TestSuite::Compare::StringToFile);
}

void ResourceTest::compileNothing() {
    CORRADE_COMPARE_AS(Resource::compile("ResourceTestNothingData", "nothing", {}),
                       Directory::join(RESOURCE_TEST_DIR, "compiled-nothing.cpp"),
                       TestSuite::Compare::StringToFile);
}

void ResourceTest::compileEmptyFile() {
    std::vector<std::pair<std::string, std::string>> input{
        {"empty.bin", ""}};
    CORRADE_COMPARE_AS(Resource::compile("ResourceTestData", "test", input),
                       Directory::join(RESOURCE_TEST_DIR, "compiled-empty.cpp"),
                       TestSuite::Compare::StringToFile);
}

void ResourceTest::compileFrom() {
    const std::string compiled = Resource::compileFrom("ResourceTestData",
        Directory::join(RESOURCE_TEST_DIR, "resources.conf"));
    CORRADE_COMPARE_AS(compiled, Directory::join(RESOURCE_TEST_DIR, "compiled.cpp"),
                       TestSuite::Compare::StringToFile);
}

void ResourceTest::compileFromUtf8Filenames() {
    const std::string compiled = Resource::compileFrom("ResourceTestUtf8Data",
        Directory::join(RESOURCE_TEST_DIR, "hýždě.conf"));
    CORRADE_COMPARE_AS(compiled, Directory::join(RESOURCE_TEST_DIR, "compiled-unicode.cpp"),
                       TestSuite::Compare::StringToFile);
}

void ResourceTest::compileFromNonexistentResource() {
    std::ostringstream out;
    Error redirectError{&out};

    CORRADE_VERIFY(Resource::compileFrom("ResourceTestData", "nonexistent.conf").empty());
    CORRADE_COMPARE(out.str(), "    Error: file nonexistent.conf does not exist\n");
}

void ResourceTest::compileFromNonexistentFile() {
    std::ostringstream out;
    Error redirectError{&out};

    CORRADE_VERIFY(Resource::compileFrom("ResourceTestData",
        Directory::join(RESOURCE_TEST_DIR, "resources-nonexistent.conf")).empty());
    CORRADE_COMPARE(out.str(), "    Error: cannot open file /nonexistent.dat of file 1 in group name\n");
}

void ResourceTest::compileFromEmptyGroup() {
    std::ostringstream out;
    Error redirectError{&out};

    /* Empty group name is allowed */
    CORRADE_VERIFY(!Resource::compileFrom("ResourceTestData",
        Directory::join(RESOURCE_TEST_DIR, "resources-empty-group.conf")).empty());
    CORRADE_COMPARE(out.str(), "");

    /* Missing group entry is not allowed */
    CORRADE_VERIFY(Resource::compileFrom("ResourceTestData",
        Directory::join(RESOURCE_TEST_DIR, "resources-no-group.conf")).empty());
    CORRADE_COMPARE(out.str(), "    Error: group name is not specified\n");
}

void ResourceTest::compileFromEmptyFilename() {
    std::ostringstream out;
    Error redirectError{&out};

    CORRADE_VERIFY(Resource::compileFrom("ResourceTestData",
        Directory::join(RESOURCE_TEST_DIR, "resources-empty-filename.conf")).empty());
    CORRADE_COMPARE(out.str(), "    Error: filename or alias of file 1 in group name is empty\n");
}

void ResourceTest::compileFromEmptyAlias() {
    std::ostringstream out;
    Error redirectError{&out};

    CORRADE_VERIFY(Resource::compileFrom("ResourceTestData",
        Directory::join(RESOURCE_TEST_DIR, "resources-empty-alias.conf")).empty());
    CORRADE_COMPARE(out.str(), "    Error: filename or alias of file 1 in group name is empty\n");
}

void ResourceTest::hasGroup() {
    CORRADE_VERIFY(Resource::hasGroup("test"));
    CORRADE_VERIFY(!Resource::hasGroup("nonexistent"));
}

void ResourceTest::list() {
    Resource r("test");
    CORRADE_COMPARE_AS(r.list(),
                       (std::vector<std::string>{"consequence.bin", "predisposition.bin"}),
                       TestSuite::Compare::Container);
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

void ResourceTest::getEmptyFile() {
    Resource r("empty");
    CORRADE_VERIFY(!r.getRaw("empty.bin"));
    CORRADE_COMPARE(r.get("empty.bin"), "");
}

void ResourceTest::getNonexistent() {
    std::ostringstream out;
    Error redirectError{&out};

    {
        Resource r("nonexistentGroup");
        CORRADE_COMPARE(out.str(), "Utility::Resource: group 'nonexistentGroup' was not found\n");
    }

    out.str({});

    {
        Resource r("test");
        CORRADE_VERIFY(r.get("nonexistentFile").empty());
        CORRADE_COMPARE(out.str(), "Utility::Resource::get(): file 'nonexistentFile' was not found in group 'test'\n");
    }

    Resource r("test");
    const auto data = r.getRaw("nonexistentFile");
    CORRADE_VERIFY(!data);
    CORRADE_VERIFY(!data.size());
}

void ResourceTest::getNothing() {
    std::ostringstream out;
    Error redirectError{&out};

    Resource r("nothing");
    CORRADE_VERIFY(out.str().empty());
    CORRADE_VERIFY(r.get("nonexistentFile").empty());
}

void ResourceTest::overrideGroup() {
    std::ostringstream out;
    Debug redirectDebug{&out};

    Resource::overrideGroup("test", Directory::join(RESOURCE_TEST_DIR, "resources-overriden.conf"));
    Resource r("test");

    CORRADE_COMPARE(out.str(), "Utility::Resource: group 'test' overriden with '" + Directory::join(RESOURCE_TEST_DIR, "resources-overriden.conf") + "'\n");
    CORRADE_COMPARE(r.get("predisposition.bin"), "overriden predisposition\n");
    CORRADE_COMPARE(r.get("consequence2.txt"), "overriden consequence\n");

    /* Test that two subsequence r.getRaw() point to the same location */
    const auto ptr = r.getRaw("predisposition.bin").begin();
    CORRADE_VERIFY(r.getRaw("predisposition.bin").begin() == ptr);
}

void ResourceTest::overrideGroupFallback() {
    std::ostringstream out;
    Warning redirectWarning{&out};

    Resource::overrideGroup("test", Directory::join(RESOURCE_TEST_DIR, "resources-overriden-none.conf"));
    Resource r("test");

    CORRADE_COMPARE_AS(r.get("consequence.bin"),
                       Directory::join(RESOURCE_TEST_DIR, "consequence.bin"),
                       TestSuite::Compare::StringToFile);
    CORRADE_COMPARE(out.str(), "Utility::Resource::get(): file 'consequence.bin' was not found in overriden group, fallback to compiled-in resources\n");
}

void ResourceTest::overrideNonexistentFile() {
    std::ostringstream out;
    Error redirectError{&out};
    Warning redirectWarning(&out);

    Resource::overrideGroup("test", Directory::join(RESOURCE_TEST_DIR, "resources-overriden-nonexistent-file.conf"));
    Resource r("test");

    CORRADE_COMPARE_AS(r.get("consequence.bin"),
                       Directory::join(RESOURCE_TEST_DIR, "consequence.bin"),
                       TestSuite::Compare::StringToFile);
    CORRADE_COMPARE(out.str(),
        "Utility::Resource::get(): cannot open file path/to/nonexistent.bin from overriden group\n"
        "Utility::Resource::get(): file 'consequence.bin' was not found in overriden group, fallback to compiled-in resources\n");
}

void ResourceTest::overrideNonexistentGroup() {
    std::ostringstream out;
    Error redirectError{&out};

    /* Nonexistent group */
    Resource::overrideGroup("nonexistentGroup", {});
    CORRADE_COMPARE(out.str(), "Utility::Resource::overrideGroup(): group 'nonexistentGroup' was not found\n");
}

void ResourceTest::overrideDifferentGroup() {
    std::ostringstream out;
    Resource::overrideGroup("test", Directory::join(RESOURCE_TEST_DIR, "resources-overriden-different.conf"));

    Warning redirectWarning{&out};
    Resource r("test");
    CORRADE_COMPARE(out.str(), "Utility::Resource: overriden with different group, found 'wat' but expected 'test'\n");
}

}}}

CORRADE_TEST_MAIN(Corrade::Utility::Test::ResourceTest)
