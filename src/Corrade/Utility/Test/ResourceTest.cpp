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

#include <map>
#include <sstream>

#include "Corrade/Containers/Array.h"
#include "Corrade/Containers/Optional.h"
#include "Corrade/Containers/String.h"
#include "Corrade/Containers/StringStl.h" /** @todo remove when <sstream> is gone */
#include "Corrade/TestSuite/Tester.h"
#include "Corrade/TestSuite/Compare/Container.h"
#include "Corrade/TestSuite/Compare/Numeric.h"
#include "Corrade/TestSuite/Compare/String.h"
#include "Corrade/TestSuite/Compare/StringToFile.h"
#include "Corrade/Utility/DebugStl.h" /** @todo remove when <sstream> is gone */
#include "Corrade/Utility/FormatStl.h"
#include "Corrade/Utility/Path.h"
#include "Corrade/Utility/Resource.h"
#include "Corrade/Utility/Implementation/Resource.h"

#include "configure.h"

namespace Corrade { namespace Utility { namespace Test { namespace {

struct ResourceTest: TestSuite::Tester {
    explicit ResourceTest();

    void resourceFilenameAt();
    void resourceDataAt();
    void resourceLookup();

    void benchmarkLookupInPlace();
    void benchmarkLookupStdMap();

    void hasGroup();
    void emptyGroup();
    void nonexistentGroup();
    void list();
    void listEmptyGroup();
    void getRaw();
    void getString();
    void getEmptyFileRaw();
    void getEmptyFileString();
    void getNonexistentFile();

    void overrideGroup();
    void overrideGroupNonexistent();
    void overrideGroupDifferent();
    void overrideGroupFileNonexistent();
    void overrideGroupFileFallback();
    void overrideGroupFileFallbackReadError();
};

ResourceTest::ResourceTest() {
    addTests({&ResourceTest::resourceFilenameAt,
              &ResourceTest::resourceDataAt,
              &ResourceTest::resourceLookup});

    addBenchmarks({&ResourceTest::benchmarkLookupInPlace,
                   &ResourceTest::benchmarkLookupStdMap}, 100);

    addTests({&ResourceTest::hasGroup,
              &ResourceTest::emptyGroup,
              &ResourceTest::nonexistentGroup,
              &ResourceTest::list,
              &ResourceTest::listEmptyGroup,
              &ResourceTest::getRaw,
              &ResourceTest::getString,
              &ResourceTest::getEmptyFileRaw,
              &ResourceTest::getEmptyFileString,
              &ResourceTest::getNonexistentFile,

              &ResourceTest::overrideGroup,
              &ResourceTest::overrideGroupNonexistent,
              &ResourceTest::overrideGroupDifferent,
              &ResourceTest::overrideGroupFileNonexistent,
              &ResourceTest::overrideGroupFileFallback,
              &ResourceTest::overrideGroupFileFallbackReadError});
}

using namespace Containers::Literals;

constexpr unsigned int Positions[] {
    3, 6,
    11, 17,
    20, 21,
    30, 25,
    40, 44
};

constexpr unsigned char Filenames[] =
    "TOC"           // 3    3
    "data.txt"      // 8    11
    "image.png"     // 9    20
    "image2.png"    // 10   30
    "license.md"    // 10   40
    ;

constexpr unsigned char Data[] =
    "Don't."                    // 6    6
    "hello world"               // 11   17
    "!PNG"                      // 4    21
    "!PNG"                      // 4    25
    "GPL?!\n#####\n\nDon't."    // 19   44
    ;

void ResourceTest::resourceFilenameAt() {
    /* Last position says how large the filenames are */
    CORRADE_COMPARE(sizeof(Filenames) - 1, Positions[4*2]);

    /* First is a special case */
    Containers::StringView toc = Implementation::resourceFilenameAt(Positions, Filenames, 0);
    CORRADE_COMPARE(toc, "TOC");
    CORRADE_COMPARE(toc.flags(), Containers::StringViewFlag::Global);

    Containers::StringView png = Implementation::resourceFilenameAt(Positions, Filenames, 2);
    CORRADE_COMPARE(png, "image.png");
    CORRADE_COMPARE(png.flags(), Containers::StringViewFlag::Global);
}

void ResourceTest::resourceDataAt() {
    /* Last position says how large the filenames are */
    CORRADE_COMPARE(sizeof(Data) - 1, Positions[4*2 + 1]);

    /* First is a special case */
    CORRADE_COMPARE(Implementation::resourceDataAt(Positions, Data, 0), "Don't."_s);
    CORRADE_COMPARE(Implementation::resourceDataAt(Positions, Data, 4), "GPL?!\n#####\n\nDon't."_s);
}

void ResourceTest::resourceLookup() {
    /* The filenames should be sorted */
    for(std::size_t i = 0; i != 4; ++i) {
        CORRADE_ITERATION(i);
        CORRADE_COMPARE_AS(
            Implementation::resourceFilenameAt(Positions, Filenames, 0),
            Implementation::resourceFilenameAt(Positions, Filenames, 1),
            TestSuite::Compare::Less);
    }

    /* Those exist */
    CORRADE_COMPARE(Implementation::resourceLookup(5, Positions, Filenames,
        "TOC"), 0);
    CORRADE_COMPARE(Implementation::resourceLookup(5, Positions, Filenames,
        "data.txt"), 1);
    CORRADE_COMPARE(Implementation::resourceLookup(5, Positions, Filenames,
        "image.png"), 2);
    CORRADE_COMPARE(Implementation::resourceLookup(5, Positions, Filenames,
        "image2.png"), 3);
    CORRADE_COMPARE(Implementation::resourceLookup(5, Positions, Filenames,
        "license.md"), 4);

    /* An extra null terminator won't match */
    CORRADE_COMPARE(Implementation::resourceLookup(5, Positions, Filenames, "TOC\0"_s), 5);

    /* Lower bound returns license.md, but filename match discards that */
    CORRADE_COMPARE(Implementation::resourceLookup(5, Positions, Filenames, "image3.png"), 5);

    /* Last name is license.md, this is after, so lower bound returns end */
    CORRADE_COMPARE(Implementation::resourceLookup(5, Positions, Filenames, "termcap.info"), 5);
}

CORRADE_NEVER_INLINE unsigned int lookupInPlace(Containers::StringView key) {
    return Implementation::resourceLookup(5, Positions, Filenames, key);
}

CORRADE_NEVER_INLINE unsigned int lookupStdMap(const std::map<std::string, unsigned int>& map, const std::string& key) {
    return map.at(key);
}

void ResourceTest::benchmarkLookupInPlace() {
    const Containers::StringView key = "license.md";
    unsigned int out = 0;
    CORRADE_BENCHMARK(10)
        out += lookupInPlace(key);

    CORRADE_COMPARE(out, 40);
}

void ResourceTest::benchmarkLookupStdMap() {
    std::map<std::string, unsigned int> map{
        {"TOC", 0},
        {"data.txt", 1},
        {"image.png", 2},
        {"image2.png", 3},
        {"license.md", 4},
    };

    std::string key = "license.md";
    unsigned int out = 0;
    CORRADE_BENCHMARK(10)
        out += lookupStdMap(map, key);

    CORRADE_COMPARE(out, 40);
}

void ResourceTest::hasGroup() {
    CORRADE_VERIFY(Resource::hasGroup("test"));
    CORRADE_VERIFY(!Resource::hasGroup("nonexistent"));
}

void ResourceTest::emptyGroup() {
    /* Should not print any error messages about anything */
    std::ostringstream out;
    Error redirectError{&out};
    Resource rs{"nothing"};
    CORRADE_COMPARE(out.str(), "");
}

void ResourceTest::nonexistentGroup() {
    #ifdef CORRADE_NO_ASSERT
    CORRADE_SKIP("CORRADE_NO_ASSERT defined, can't test assertions");
    #endif

    std::ostringstream out;
    Error redirectError{&out};
    Resource rs{"nonexistentGroup"};
    CORRADE_COMPARE(out.str(), "Utility::Resource: group 'nonexistentGroup' was not found\n");
}

void ResourceTest::list() {
    Resource rs{"test"};
    Containers::Array<Containers::StringView> list = rs.list();
    CORRADE_COMPARE_AS(list,
        Containers::arrayView({"consequence.bin"_s, "predisposition.bin"_s}),
        TestSuite::Compare::Container);
    CORRADE_COMPARE(list[0].flags(), Containers::StringViewFlag::Global);
    CORRADE_COMPARE(list[1].flags(), Containers::StringViewFlag::Global);
}

void ResourceTest::listEmptyGroup() {
    Resource rs{"nothing"};
    CORRADE_COMPARE_AS(rs.list(),
        Containers::ArrayView<const Containers::StringView>{},
        TestSuite::Compare::Container);
}

void ResourceTest::getRaw() {
    Resource rs{"test"};

    CORRADE_COMPARE_AS(rs.getRaw("predisposition.bin"),
        Path::join(RESOURCE_TEST_DIR, "predisposition.bin"),
        TestSuite::Compare::StringToFile);

    CORRADE_COMPARE_AS(rs.getRaw("consequence.bin"),
        Path::join(RESOURCE_TEST_DIR, "consequence.bin"),
        TestSuite::Compare::StringToFile);
}

void ResourceTest::getString() {
    Resource rs{"test"};

    Containers::StringView predisposition = rs.getString("predisposition.bin");
    CORRADE_COMPARE_AS(predisposition,
        Path::join(RESOURCE_TEST_DIR, "predisposition.bin"),
        TestSuite::Compare::StringToFile);
    CORRADE_COMPARE(predisposition.flags(), Containers::StringViewFlag::Global);

    Containers::StringView consequence = rs.getString("consequence.bin");
    CORRADE_COMPARE_AS(consequence,
        Path::join(RESOURCE_TEST_DIR, "consequence.bin"),
        TestSuite::Compare::StringToFile);
    CORRADE_COMPARE(consequence.flags(), Containers::StringViewFlag::Global);
}

void ResourceTest::getEmptyFileRaw() {
    Resource rs{"empty"};

    Containers::ArrayView<const char> empty = rs.getRaw("empty.bin");
    CORRADE_VERIFY(!empty.data());
    CORRADE_VERIFY(!empty.size());
}

void ResourceTest::getEmptyFileString() {
    Resource rs{"empty"};

    Containers::StringView empty = rs.getString("empty.bin");
    CORRADE_VERIFY(!empty.data());
    CORRADE_VERIFY(!empty.size());
    CORRADE_COMPARE(empty.flags(), Containers::StringViewFlag::Global);
}

void ResourceTest::getNonexistentFile() {
    #ifdef CORRADE_NO_ASSERT
    CORRADE_SKIP("CORRADE_NO_ASSERT defined, can't test assertions");
    #endif

    std::ostringstream out;
    Error redirectError{&out};
    Resource rs{"test"};
    rs.getString("nonexistentFile");
    rs.getRaw("nonexistentFile");
    /* The message is still get() as that's what eventually will getRaw() be
       renamed to; and getString() uses the same underlying code */
    CORRADE_COMPARE(out.str(),
        "Utility::Resource::get(): file 'nonexistentFile' was not found in group 'test'\n"
        "Utility::Resource::get(): file 'nonexistentFile' was not found in group 'test'\n");
}

void ResourceTest::overrideGroup() {
    Resource::overrideGroup("test", Path::join(RESOURCE_TEST_DIR, "resources-overridden.conf"));

    std::ostringstream out;
    Debug redirectDebug{&out};
    Resource rs{"test"};
    CORRADE_COMPARE(out.str(), formatString("Utility::Resource: group 'test' overridden with '{}'\n", Path::join(RESOURCE_TEST_DIR, "resources-overridden.conf")));

    /* Overriden files are not marked as global */
    Containers::StringView predisposition = rs.getString("predisposition.bin");
    CORRADE_COMPARE(predisposition, "overridden predisposition\n");
    CORRADE_COMPARE(predisposition.flags(), Containers::StringViewFlags{});

    /* Two subsequent calls should point to the same location (the file doesn't
    get read again) */
    CORRADE_VERIFY(rs.getString("predisposition.bin").data() == predisposition.data());
}

void ResourceTest::overrideGroupNonexistent() {
    #ifdef CORRADE_NO_ASSERT
    CORRADE_SKIP("CORRADE_NO_ASSERT defined, can't test assertions");
    #endif

    std::ostringstream out;
    Error redirectError{&out};
    Resource::overrideGroup("nonexistentGroup", {});
    CORRADE_COMPARE(out.str(), "Utility::Resource::overrideGroup(): group 'nonexistentGroup' was not found\n");
}

void ResourceTest::overrideGroupDifferent() {
    Resource::overrideGroup("test", Path::join(RESOURCE_TEST_DIR, "resources-overridden-different.conf"));

    std::ostringstream out;
    Warning redirectWarning{&out};
    Resource rs{"test"};
    CORRADE_COMPARE(out.str(), "Utility::Resource: overridden with different group, found 'wat' but expected 'test'\n");
}

void ResourceTest::overrideGroupFileFallback() {
    #ifdef CORRADE_NO_ASSERT
    CORRADE_SKIP("CORRADE_NO_ASSERT defined, can't test assertions");
    #endif

    Resource::overrideGroup("test", Path::join(RESOURCE_TEST_DIR, "resources-overridden-none.conf"));
    Resource rs{"test"};

    std::ostringstream out;
    Warning redirectWarning{&out};
    Containers::StringView consequence = rs.getString("consequence.bin");
    CORRADE_COMPARE(out.str(), "Utility::Resource::get(): file 'consequence.bin' was not found in overridden group, fallback to compiled-in resources\n");

    /* Original compiled-in file, global flag */
    CORRADE_COMPARE_AS(consequence,
        Path::join(RESOURCE_TEST_DIR, "consequence.bin"),
        TestSuite::Compare::StringToFile);
    CORRADE_COMPARE(consequence.flags(), Containers::StringViewFlag::Global);
}

void ResourceTest::overrideGroupFileFallbackReadError() {
    Resource::overrideGroup("test", Path::join(RESOURCE_TEST_DIR, "resources-overridden-nonexistent-file.conf"));
    Resource rs{"test"};

    std::ostringstream out;
    Error redirectError{&out};
    Warning redirectWarning(&out);
    Containers::StringView consequence = rs.getString("consequence.bin");
    /* There's an error message from Path::read() before */
    CORRADE_COMPARE_AS(out.str(),
        "\nUtility::Resource::get(): cannot open file path/to/nonexistent.bin from overridden group\n"
        "Utility::Resource::get(): file 'consequence.bin' was not found in overridden group, fallback to compiled-in resources\n",
        TestSuite::Compare::StringHasSuffix);

    /* Original compiled-in file, global flag */
    CORRADE_COMPARE_AS(consequence,
        Path::join(RESOURCE_TEST_DIR, "consequence.bin"),
        TestSuite::Compare::StringToFile);
    CORRADE_COMPARE(consequence.flags(), Containers::StringViewFlag::Global);
}

void ResourceTest::overrideGroupFileNonexistent() {
    #ifdef CORRADE_NO_ASSERT
    CORRADE_SKIP("CORRADE_NO_ASSERT defined, can't test assertions");
    #endif

    Resource::overrideGroup("test", Path::join(RESOURCE_TEST_DIR, "resources-overridden-nonexistent-file.conf"));
    Resource rs{"test"};

    std::ostringstream out;
    Error redirectError{&out};
    rs.getString("consequence2.txt");
    /* The file is in the overriden group, but not in the compiled-in data and
       thus it fails */
    CORRADE_COMPARE(out.str(), "Utility::Resource::get(): file 'consequence2.txt' was not found in group 'test'\n");
}

}}}}

CORRADE_TEST_MAIN(Corrade::Utility::Test::ResourceTest)
