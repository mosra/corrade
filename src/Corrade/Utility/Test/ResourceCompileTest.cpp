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

#include "Corrade/TestSuite/Tester.h"
#include "Corrade/TestSuite/Compare/Container.h"
#include "Corrade/TestSuite/Compare/String.h"
#include "Corrade/TestSuite/Compare/StringToFile.h"
#include "Corrade/Utility/DebugStl.h"
#include "Corrade/Utility/Implementation/ResourceCompile.h"

#include "configure.h"

namespace Corrade { namespace Utility { namespace Test { namespace {

struct ResourceCompileTest: TestSuite::Tester {
    explicit ResourceCompileTest();

    void compile();
    void compileNotSorted();
    void compileNothing();
    void compileEmptyFile();

    void compileFrom();
    void compileFromNothing();
    void compileFromUtf8Filenames();
    void compileFromEmptyGroup();

    void compileFromInvalid();

    void compileSingle();
    void compileSingleNonexistentFile();
    void compileSingleEmptyFile();
};

const struct {
    const char* name;
    const char* file;
    /* If the message ends with a \n, it's a suffix, otherwise it's the full
       message without the Error: prefix and a newline */
    const char* message;
} CompileFromInvalidData[]{
    {"nonexistent resource file", "/nonexistent.conf",
        "file /nonexistent.conf does not exist"},
    {"nonexistent file", "resources-nonexistent.conf",
        /* There's an error message from Path::read() before */
        "\n    Error: cannot open file /nonexistent.dat of file 1 in group name\n"},
    /* Empty group= option is allowed, tested in compileFromEmptyGroup() */
    {"empty group", "resources-no-group.conf",
        "group name is not specified"},
    {"empty filename", "resources-empty-filename.conf",
        "filename or alias of file 1 in group name is empty"},
    {"empty alias", "resources-empty-alias.conf",
        "filename or alias of file 1 in group name is empty"},
};

ResourceCompileTest::ResourceCompileTest() {
    addTests({&ResourceCompileTest::compile,
              &ResourceCompileTest::compileNotSorted,
              &ResourceCompileTest::compileNothing,
              &ResourceCompileTest::compileEmptyFile,

              &ResourceCompileTest::compileFrom,
              &ResourceCompileTest::compileFromNothing,
              &ResourceCompileTest::compileFromUtf8Filenames,
              &ResourceCompileTest::compileFromEmptyGroup});


    addInstancedTests({&ResourceCompileTest::compileFromInvalid},
        Containers::arraySize(CompileFromInvalidData));

    addTests({&ResourceCompileTest::compileSingle,
              &ResourceCompileTest::compileSingleNonexistentFile,
              &ResourceCompileTest::compileSingleEmptyFile});
}

void ResourceCompileTest::compile() {
    /* Testing also null bytes and signed overflow, don't change binaries */
    Containers::Optional<Containers::Array<char>> consequence = Path::read(Path::join(RESOURCE_TEST_DIR, "consequence.bin"));
    Containers::Optional<Containers::Array<char>> predisposition = Path::read(Path::join(RESOURCE_TEST_DIR, "predisposition.bin"));
    CORRADE_VERIFY(consequence);
    CORRADE_VERIFY(predisposition);
    const Implementation::FileData input[]{
        {"consequence.bin", *std::move(consequence)},
        {"predisposition.bin", *std::move(predisposition)}
    };
    CORRADE_COMPARE_AS(Implementation::resourceCompile("ResourceTestData", "test", input),
        Path::join(RESOURCE_TEST_DIR, "compiled.cpp"),
        TestSuite::Compare::StringToFile);
}

void ResourceCompileTest::compileNotSorted() {
    CORRADE_SKIP_IF_NO_ASSERT();

    const Implementation::FileData input[]{
        {"predisposition.bin", {}},
        {"consequence.bin",{}}
    };

    std::ostringstream out;
    Error redirectError{&out};
    Implementation::resourceCompile("ResourceTestData", "test", input);
    CORRADE_COMPARE(out.str(), "Utility::Resource::compile(): the file list is not sorted\n");
}

void ResourceCompileTest::compileNothing() {
    CORRADE_COMPARE_AS(Implementation::resourceCompile("ResourceTestNothingData", "nothing", {}),
        Path::join(RESOURCE_TEST_DIR, "compiled-nothing.cpp"),
        TestSuite::Compare::StringToFile);
}

void ResourceCompileTest::compileEmptyFile() {
    const Implementation::FileData input[]{
        {"empty.bin", {}}
    };
    CORRADE_COMPARE_AS(Implementation::resourceCompile("ResourceTestData", "test", input),
        Path::join(RESOURCE_TEST_DIR, "compiled-empty.cpp"),
        TestSuite::Compare::StringToFile);
}

void ResourceCompileTest::compileFrom() {
    Containers::String conf = Path::join(RESOURCE_TEST_DIR, "resources.conf");
    CORRADE_COMPARE_AS(Implementation::resourceCompileFrom("ResourceTestData", conf),
        Path::join(RESOURCE_TEST_DIR, "compiled.cpp"),
        TestSuite::Compare::StringToFile);
}

void ResourceCompileTest::compileFromNothing() {
    Containers::String conf = Path::join(RESOURCE_TEST_DIR, "resources-nothing.conf");
    CORRADE_COMPARE_AS(Implementation::resourceCompileFrom("ResourceTestNothingData", conf),
        Path::join(RESOURCE_TEST_DIR, "compiled-nothing.cpp"),
        TestSuite::Compare::StringToFile);
}

void ResourceCompileTest::compileFromUtf8Filenames() {
    Containers::String conf = Path::join(RESOURCE_TEST_DIR, "hýždě.conf");
    CORRADE_COMPARE_AS(Implementation::resourceCompileFrom("ResourceTestUtf8Data", conf),
        Path::join(RESOURCE_TEST_DIR, "compiled-unicode.cpp"),
        TestSuite::Compare::StringToFile);
}

void ResourceCompileTest::compileFromEmptyGroup() {
    /* Empty group name is allowed */
    std::ostringstream out;
    Error redirectError{&out};
    CORRADE_VERIFY(Implementation::resourceCompileFrom("ResourceTestData",
        Path::join(RESOURCE_TEST_DIR, "resources-empty-group.conf")));
    CORRADE_COMPARE(out.str(), "");

    /* Missing group entry is not allowed -- tested in compileFromInvalid()
       below */
}


void ResourceCompileTest::compileFromInvalid() {
    auto&& data = CompileFromInvalidData[testCaseInstanceId()];
    setTestCaseDescription(data.name);

    std::ostringstream out;
    Error redirectError{&out};
    CORRADE_VERIFY(Implementation::resourceCompileFrom("ResourceTestData", Utility::Path::join(RESOURCE_TEST_DIR, data.file)).isEmpty());
    if(Containers::StringView{data.message}.hasSuffix('\n'))
        CORRADE_COMPARE_AS(out.str(), data.message, TestSuite::Compare::StringHasSuffix);
    else CORRADE_COMPARE(out.str(), Utility::formatString("    Error: {}\n", data.message));
}

void ResourceCompileTest::compileSingle() {
    CORRADE_COMPARE_AS(Implementation::resourceCompileSingle("ResourceTestData", Path::join(RESOURCE_TEST_DIR, "consequence.bin")),
        Path::join(RESOURCE_TEST_DIR, "compiled-single.cpp"),
        TestSuite::Compare::StringToFile);
}

void ResourceCompileTest::compileSingleNonexistentFile() {
    std::ostringstream out;
    Error redirectError{&out};
    CORRADE_VERIFY(!Implementation::resourceCompileSingle("ResourceTestData", "/nonexistent.dat"));
    /* There's an error message from Path::read() before */
    CORRADE_COMPARE_AS(out.str(),
        "\n    Error: cannot open file /nonexistent.dat\n",
        TestSuite::Compare::StringHasSuffix);
}

void ResourceCompileTest::compileSingleEmptyFile() {
    CORRADE_COMPARE_AS(Implementation::resourceCompileSingle("ResourceTestData", Path::join(RESOURCE_TEST_DIR, "empty.bin")),
        Path::join(RESOURCE_TEST_DIR, "compiled-single-empty.cpp"),
        TestSuite::Compare::StringToFile);
}

}}}}

CORRADE_TEST_MAIN(Corrade::Utility::Test::ResourceCompileTest)
