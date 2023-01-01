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
    void compileNothing();
    void compileEmptyFile();

    void compileNullTerminatedAligned();
    void compileNullTerminatedLastFile();
    void compileAlignmentLargerThanDataSize();

    void compileFrom();
    void compileFromNothing();
    void compileFromUtf8Filenames();
    void compileFromEmptyGroup();

    void compileFromNullTerminatedAligned();
    void compileFromNullTerminatedLastFile();
    void compileFromAlignmentLargerThanDataSize();

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
    {"zero global alignment", "resources-zero-global-align.conf",
        "alignment in group broken required to be a power-of-two value between 1 and 128, got 0"},
    {"zero alignment", "resources-zero-align.conf",
        "alignment of file 1 in group broken required to be a power-of-two value between 1 and 128, got 0"},
    {"non-power-of-two global alignment", "resources-npot-global-align.conf",
        "alignment in group broken required to be a power-of-two value between 1 and 128, got 56"},
    {"non-power-of-two alignment", "resources-npot-align.conf",
        "alignment of file 2 in group broken required to be a power-of-two value between 1 and 128, got 56"},
    {"too large global alignment", "resources-too-large-global-align.conf",
        "alignment in group broken required to be a power-of-two value between 1 and 128, got 256"},
    {"too large alignment", "resources-too-large-align.conf",
        "alignment of file 2 in group broken required to be a power-of-two value between 1 and 128, got 256"},
};

ResourceCompileTest::ResourceCompileTest() {
    addTests({&ResourceCompileTest::compile,
              &ResourceCompileTest::compileNothing,
              &ResourceCompileTest::compileEmptyFile,

              &ResourceCompileTest::compileNullTerminatedAligned,
              &ResourceCompileTest::compileNullTerminatedLastFile,
              &ResourceCompileTest::compileAlignmentLargerThanDataSize,

              &ResourceCompileTest::compileFrom,
              &ResourceCompileTest::compileFromNothing,
              &ResourceCompileTest::compileFromUtf8Filenames,
              &ResourceCompileTest::compileFromEmptyGroup,

              &ResourceCompileTest::compileFromNullTerminatedAligned,
              &ResourceCompileTest::compileFromNullTerminatedLastFile,
              &ResourceCompileTest::compileFromAlignmentLargerThanDataSize});

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
        {"consequence.bin", false, 1, *std::move(consequence)},
        {"predisposition.bin", false, 1, *std::move(predisposition)}
    };
    CORRADE_COMPARE_AS(Implementation::resourceCompile("ResourceTestData", "test", input),
        Path::join(RESOURCE_TEST_DIR, "compiled.cpp"),
        TestSuite::Compare::StringToFile);
}

void ResourceCompileTest::compileNothing() {
    CORRADE_COMPARE_AS(Implementation::resourceCompile("ResourceTestNothingData", "nothing", {}),
        Path::join(RESOURCE_TEST_DIR, "compiled-nothing.cpp"),
        TestSuite::Compare::StringToFile);
}

void ResourceCompileTest::compileEmptyFile() {
    const Implementation::FileData input[]{
        {"empty.bin", false, 1, {}}
    };
    CORRADE_COMPARE_AS(Implementation::resourceCompile("ResourceTestData", "test", input),
        Path::join(RESOURCE_TEST_DIR, "compiled-empty.cpp"),
        TestSuite::Compare::StringToFile);
}

void ResourceCompileTest::compileNullTerminatedAligned() {
    /* The same files are used in compileFromNullTerminatedAligned() which
       should give the same output, and also at build time for ResourceTest,
       for consistency it's easier to just load them */
    Containers::Optional<Containers::Array<char>> data17bytes66 = Path::read(Path::join(RESOURCE_TEST_DIR, "17bytes-66.bin"));
    CORRADE_VERIFY(data17bytes66);
    CORRADE_COMPARE(data17bytes66->size(), 17);

    Containers::Optional<Containers::Array<char>> data17bytes33 = Path::read(Path::join(RESOURCE_TEST_DIR, "17bytes-33.bin"));
    CORRADE_VERIFY(data17bytes33);
    CORRADE_COMPARE(data17bytes33->size(), 17);

    Containers::Optional<Containers::Array<char>> data55bytes66 = Path::read(Path::join(RESOURCE_TEST_DIR, "55bytes-66.bin"));
    CORRADE_VERIFY(data55bytes66);
    CORRADE_COMPARE(data55bytes66->size(), 55);

    Containers::Optional<Containers::Array<char>> data64bytes33 = Path::read(Path::join(RESOURCE_TEST_DIR, "64bytes-33.bin"));
    CORRADE_VERIFY(data64bytes33);
    CORRADE_COMPARE(data64bytes33->size(), 64);

    /* Aliases are numbered in order to guarantee the order, see
       Implementation/ResourceCompile.h for more details on the data packing
       options considered. */
    const Implementation::FileData input[]{
        /* This one is null-terminated so there should be exactly one byte
           after */
        {"0-null-terminated.bin", true, 1,
            Containers::Array<char>{*data17bytes66, [](char*, std::size_t){}}},
        /* This one is neither aligned nor null-terminated */
        {"1.bin", false, 1,
            Containers::Array<char>{*data17bytes33, [](char*, std::size_t){}}},
        /* This one is 16-byte aligned so there should be padding before */
        {"2-align16.bin", false, 16,
            Containers::Array<char>{*data17bytes66, [](char*, std::size_t){}}},
        /* An aligned empty file. There's padding before, but no actual
           content. */
        {"3-align4-empty.bin", false, 4,
            {}},
        /* A null-terminated empty file. A single byte, plus padding for the
           next which is aligned again. */
        {"4-null-terminated-empty.bin", true, 1,
            {}},
        /* A null-terminated aligned empty file. A single byte. */
        {"5-null-terminated-align8-empty.bin", true, 8,
            {}},
        /* This one is exactly 64 bytes, but because it is null-terminated,
           the next one has to be padded by another 64 bytes */
        {"6-null-terminated-align64.bin", true, 64,
            Containers::Array<char>{*data64bytes33, [](char*, std::size_t){}}},
        /* This one is 64-byte aligned but smaller than that, which is fine
           -- the next files will start right after */
        {"7-align64.bin", false, 64,
            Containers::Array<char>{*data55bytes66, [](char*, std::size_t){}}},
        /* A non-null-terminated non-aligned file at the end. There should be
           no padding after. If any alignment extends beyond the data end,
           there would be -- that's tested in
           compileAlignmentLargerThanDataSize() */
        {"8.bin", false, 1,
            Containers::Array<char>{*data17bytes33, [](char*, std::size_t){}}}
    };

    Containers::String out = Implementation::resourceCompile("ResourceTestNullTerminatedAlignedData", "nullTerminatedAligned", input);
    CORRADE_COMPARE_AS(out,
        Path::join(RESOURCE_TEST_DIR, "compiled-null-terminated-aligned.cpp"),
        TestSuite::Compare::StringToFile);
    CORRADE_COMPARE_AS(out, "alignas(64)", TestSuite::Compare::StringContains);
}

void ResourceCompileTest::compileNullTerminatedLastFile() {
    /* The same file is used in compileFromNullTerminatedLastFile() which
       should give the same output, and also at build time for ResourceTest,
       for consistency it's easier to just load it */
    Containers::Optional<Containers::Array<char>> data17bytes66 = Path::read(Path::join(RESOURCE_TEST_DIR, "17bytes-66.bin"));
    CORRADE_VERIFY(data17bytes66);
    CORRADE_COMPARE(data17bytes66->size(), 17);

    /* There should be exactly one byte after, and no alignment specifier */
    const Implementation::FileData input[]{
        {"0-null-terminated.bin", true, 1,
            Containers::Array<char>{*data17bytes66, [](char*, std::size_t){}}}
    };

    Containers::String out = Implementation::resourceCompile("ResourceTestNullTerminatedLastFileData", "nullTerminatedLastFile", input);
    CORRADE_COMPARE_AS(out,
        Path::join(RESOURCE_TEST_DIR, "compiled-null-terminated-last-file.cpp"),
        TestSuite::Compare::StringToFile);
    /* There should be no alignas if it's just null-terminated */
    CORRADE_COMPARE_AS(out, "alignas", TestSuite::Compare::StringNotContains);
}

void ResourceCompileTest::compileAlignmentLargerThanDataSize() {
    /* The same file is used in compileFromNullTerminatedLastFile() which
       should give the same output, and also at build time for ResourceTest,
       for consistency it's easier to just load it */
    Containers::Optional<Containers::Array<char>> data17bytes66 = Path::read(Path::join(RESOURCE_TEST_DIR, "17bytes-66.bin"));
    CORRADE_VERIFY(data17bytes66);
    CORRADE_COMPARE(data17bytes66->size(), 17);

    Containers::Optional<Containers::Array<char>> data64bytes33 = Path::read(Path::join(RESOURCE_TEST_DIR, "64bytes-33.bin"));
    CORRADE_VERIFY(data64bytes33);
    CORRADE_COMPARE(data64bytes33->size(), 64);

    /* There should be 46 padding bytes after the last (empty) file */
    const Implementation::FileData input[]{
        {"0-align128.bin", false, 128,
            Containers::Array<char>{*data17bytes66, [](char*, std::size_t){}}},
        {"1.bin", false, 1,
            Containers::Array<char>{*data64bytes33, [](char*, std::size_t){}}},
        {"2-align2-empty.bin", false, 2,
            {}},
    };

    Containers::String out = Implementation::resourceCompile("ResourceTestAlignmentLargerThanDataSizeData", "alignmentLargerThanDataSize", input);
    CORRADE_COMPARE_AS(out,
        Path::join(RESOURCE_TEST_DIR, "compiled-alignment-larger-than-data-size.cpp"),
        TestSuite::Compare::StringToFile);
    CORRADE_COMPARE_AS(out, "alignas(128)", TestSuite::Compare::StringContains);
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

void ResourceCompileTest::compileFromNullTerminatedAligned() {
    /* There's both global nullTerminated / align options and their local
       overrides; output same as compileNullTerminatedAligned() */
    Containers::String conf = Path::join(RESOURCE_TEST_DIR, "resources-null-terminated-aligned.conf");
    CORRADE_COMPARE_AS(Implementation::resourceCompileFrom("ResourceTestNullTerminatedAlignedData", conf),
        Path::join(RESOURCE_TEST_DIR, "compiled-null-terminated-aligned.cpp"),
        TestSuite::Compare::StringToFile);
}

void ResourceCompileTest::compileFromNullTerminatedLastFile() {
    /* output same as compileFromNullTerminatedLastFile() */
    Containers::String conf = Path::join(RESOURCE_TEST_DIR, "resources-null-terminated-last-file.conf");
    CORRADE_COMPARE_AS(Implementation::resourceCompileFrom("ResourceTestNullTerminatedLastFileData", conf),
        Path::join(RESOURCE_TEST_DIR, "compiled-null-terminated-last-file.cpp"),
        TestSuite::Compare::StringToFile);
}

void ResourceCompileTest::compileFromAlignmentLargerThanDataSize() {
    /* output same as compileFromAlignmentLargerThanDataSize() */
    Containers::String conf = Path::join(RESOURCE_TEST_DIR, "resources-alignment-larger-than-data-size.conf");
    CORRADE_COMPARE_AS(Implementation::resourceCompileFrom("ResourceTestAlignmentLargerThanDataSizeData", conf),
        Path::join(RESOURCE_TEST_DIR, "compiled-alignment-larger-than-data-size.cpp"),
        TestSuite::Compare::StringToFile);
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
