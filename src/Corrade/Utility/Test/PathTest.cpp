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

#include <sstream>

#include "Corrade/Containers/Array.h"
#include "Corrade/Containers/GrowableArray.h"
#include "Corrade/Containers/Optional.h"
#include "Corrade/Containers/Pair.h"
#include "Corrade/Containers/ScopeGuard.h"
#include "Corrade/Containers/StringStl.h"
#include "Corrade/TestSuite/Tester.h"
#include "Corrade/TestSuite/Compare/Container.h"
#include "Corrade/TestSuite/Compare/File.h"
#include "Corrade/TestSuite/Compare/FileToString.h"
#include "Corrade/TestSuite/Compare/Numeric.h"
#include "Corrade/TestSuite/Compare/String.h"
#include "Corrade/TestSuite/Compare/SortedContainer.h"
#include "Corrade/Utility/Algorithms.h"
#include "Corrade/Utility/DebugStl.h"
#include "Corrade/Utility/FormatStl.h"
#include "Corrade/Utility/Path.h"

#include "configure.h"

#ifdef CORRADE_TARGET_UNIX
/* Needed for chdir() in currentInvalid() */
#include <unistd.h>
#endif

#ifdef CORRADE_TARGET_APPLE
#include "Corrade/Utility/System.h" /* isSandboxed() */
#endif

namespace Corrade { namespace Utility { namespace Test { namespace {

struct PathTest: TestSuite::Tester {
    explicit PathTest();

    void fromNativeSeparators();
    #ifdef CORRADE_TARGET_WINDOWS
    void fromNativeSeparatorsSmall();
    void fromNativeSeparatorsNonOwned();
    #endif
    void toNativeSeparators();
    #ifdef CORRADE_TARGET_WINDOWS
    void toNativeSeparatorsSmall();
    void toNativeSeparatorsNonOwned();
    #endif

    void split();
    void splitFlags();
    void splitExtension();
    void splitExtensionFlags();

    void join();
    #ifdef CORRADE_TARGET_WINDOWS
    void joinWindows();
    #endif
    void joinMultiple();
    void joinMultipleAbsolute();
    void joinMultipleOneEmpty();
    void joinMultipleJustOne();
    void joinMultipleNone();

    void exists();
    void existsNoPermission();
    void existsNonNullTerminated();
    void existsUtf8();

    void isDirectory();
    void isDirectorySymlink();
    void isDirectoryNoPermission();
    void isDirectoryNonNullTerminated();
    void isDirectoryUtf8();

    void make();
    void makeDotDotDot();
    void makeNoPermission();
    void makeNonNullTerminated();
    void makeUtf8();

    void removeFile();
    void removeDirectory();
    void removeFileNonexistent();
    void removeDirectoryNonEmpty();
    void removeFileNonNullTerminated();
    void removeDirectoryNonNullTerminated();
    void removeFileUtf8();
    void removeDirectoryUtf8();

    void moveFile();
    void moveDirectory();
    void moveSourceNonexistent();
    void moveDestinationNoPermission();
    void moveNonNullTerminated();
    void moveUtf8();

    void currentDirectory();
    void currentDirectoryNonexistent();
    void currentDirectoryUtf8();

    #ifndef CORRADE_BUILD_STATIC
    void libraryLocation();
    #else
    void libraryLocationStatic();
    #endif
    void libraryLocationNull();
    void libraryLocationInvalid();
    void libraryLocationUtf8();

    void executableLocation();
    void executableLocationInvalid();
    void executableLocationUtf8();

    void homeDirectory();
    void homeDirectoryInvalid();
    void homeDirectoryUtf8();

    void configurationDirectory();
    void configurationDirectoryInvalid();
    void configurationDirectoryUtf8();

    void temporaryDirectory();
    void temporaryDirectoryInvalid();
    void temporaryDirectoryUtf8();

    void list();
    void listIterateRangeFor();
    void listEmptyDirectory();
    void listSkipDirectories();
    void listSkipDirectoriesSymlinks();
    void listSkipFiles();
    void listSkipFilesSymlinks();
    void listSkipSpecial();
    void listSkipSpecialSymlink();
    void listSkipDotAndDotDot();
    void listSkipEverything();
    void listSort();
    void listNonexistent();
    void listNonNullTerminated();
    void listTrailingSlash();
    void listUtf8Result();
    void listUtf8Path();

    void size();
    void sizeEmpty();
    void sizeNonSeekable();
    void sizeEarlyEof();
    void sizeDirectory();
    void sizeNonexistent();
    void sizeNonNullTerminated();
    void sizeUtf8();

    void read();
    void readString();
    void readEmpty();
    void readEmptyString();
    void readNonSeekable();
    void readNonSeekableString();
    void readEarlyEof();
    void readEarlyEofString();
    void readDirectory();
    void readNonexistent();
    void readNonNullTerminated();
    void readUtf8();

    void write();
    void writeDisabledOverloads();
    void writeEmpty();
    void writeDirectory();
    void writeNoPermission();
    void writeNonNullTerminated();
    void writeUtf8();

    void append();
    void appendDisabledOverloads();
    void appendToNonexistent();
    void appendEmpty();
    void appendDirectory();
    void appendNoPermission();
    void appendNonNullTerminated();
    void appendUtf8();

    void prepareFileToCopy();
    void copy();
    void copyEmpty();
    void copyDirectory();
    void copyReadNonexistent();
    void copyWriteNoPermission();
    void copyNonNullTerminated();
    void copyUtf8();

    #ifndef CORRADE_TARGET_EMSCRIPTEN
    void prepareFileToBenchmarkCopy();
    void copy100MReadWrite();
    void copy100MReadWriteString();
    void copy100MCopy();
    #if defined(DOXYGEN_GENERATING_OUTPUT) || defined(CORRADE_TARGET_UNIX) || (defined(CORRADE_TARGET_WINDOWS) && !defined(CORRADE_TARGET_WINDOWS_RT))
    void copy100MMap();
    #endif
    #endif

    void map();
    void mapEmpty();
    void mapDirectory();
    void mapNonexistent();
    void mapNonNullTerminated();
    void mapUtf8();

    void mapRead();
    void mapReadEmpty();
    void mapReadDirectory();
    void mapReadNonexistent();
    void mapReadNonNullTerminated();
    void mapReadUtf8();

    void mapWrite();
    void mapWriteEmpty();
    void mapWriteDirectory();
    void mapWriteNoPermission();
    void mapWriteNonNullTerminated();
    void mapWriteUtf8();

    Containers::String _testDir,
        _testDirSymlink,
        _testDirUtf8,
        _writeTestDir;
};

PathTest::PathTest() {
    addTests({&PathTest::fromNativeSeparators,
              #ifdef CORRADE_TARGET_WINDOWS
              &PathTest::fromNativeSeparatorsSmall,
              &PathTest::fromNativeSeparatorsNonOwned,
              #endif
              &PathTest::toNativeSeparators,
              #ifdef CORRADE_TARGET_WINDOWS
              &PathTest::toNativeSeparatorsSmall,
              &PathTest::toNativeSeparatorsNonOwned,
              #endif

              &PathTest::split,
              &PathTest::splitFlags,
              &PathTest::splitExtension,
              &PathTest::splitExtensionFlags,

              &PathTest::join,
              #ifdef CORRADE_TARGET_WINDOWS
              &PathTest::joinWindows,
              #endif
              &PathTest::joinMultiple,
              &PathTest::joinMultipleAbsolute,
              &PathTest::joinMultipleOneEmpty,
              &PathTest::joinMultipleJustOne,
              &PathTest::joinMultipleNone,

              &PathTest::exists,
              &PathTest::existsNoPermission,
              &PathTest::existsNonNullTerminated,
              &PathTest::existsUtf8,

              &PathTest::isDirectory,
              &PathTest::isDirectorySymlink,
              &PathTest::isDirectoryNoPermission,
              &PathTest::isDirectoryNonNullTerminated,
              &PathTest::isDirectoryUtf8,

              &PathTest::make,
              &PathTest::makeDotDotDot,
              &PathTest::makeNoPermission,
              &PathTest::makeNonNullTerminated,
              &PathTest::makeUtf8,

              &PathTest::removeFile,
              &PathTest::removeDirectory,
              &PathTest::removeFileNonexistent,
              &PathTest::removeDirectoryNonEmpty,
              &PathTest::removeFileNonNullTerminated,
              &PathTest::removeDirectoryNonNullTerminated,
              &PathTest::removeFileUtf8,
              &PathTest::removeDirectoryUtf8,

              &PathTest::moveFile,
              &PathTest::moveDirectory,
              &PathTest::moveSourceNonexistent,
              &PathTest::moveDestinationNoPermission,
              &PathTest::moveNonNullTerminated,
              &PathTest::moveUtf8,

              /* These don't pass any strings to system APIs, so no need to
                 verify non-null-terminated variants */
              &PathTest::currentDirectory,
              &PathTest::currentDirectoryNonexistent,
              &PathTest::currentDirectoryUtf8,

              #ifndef CORRADE_BUILD_STATIC
              &PathTest::libraryLocation,
              #else
              &PathTest::libraryLocationStatic,
              #endif
              &PathTest::libraryLocationNull,
              &PathTest::libraryLocationInvalid,
              &PathTest::libraryLocationUtf8,

              &PathTest::executableLocation,
              &PathTest::executableLocationInvalid,
              &PathTest::executableLocationUtf8,

              &PathTest::homeDirectory,
              &PathTest::homeDirectoryInvalid,
              &PathTest::homeDirectoryUtf8,

              &PathTest::configurationDirectory,
              &PathTest::configurationDirectoryInvalid,
              &PathTest::configurationDirectoryUtf8,

              &PathTest::temporaryDirectory,
              &PathTest::temporaryDirectoryInvalid,
              &PathTest::temporaryDirectoryUtf8,

              &PathTest::list,
              &PathTest::listIterateRangeFor,
              &PathTest::listEmptyDirectory,
              &PathTest::listSkipDirectories,
              &PathTest::listSkipDirectoriesSymlinks,
              &PathTest::listSkipFiles,
              &PathTest::listSkipFilesSymlinks,
              &PathTest::listSkipSpecial,
              &PathTest::listSkipSpecialSymlink,
              &PathTest::listSkipDotAndDotDot,
              &PathTest::listSkipEverything,
              &PathTest::listSort,
              &PathTest::listNonexistent,
              &PathTest::listNonNullTerminated,
              &PathTest::listTrailingSlash,
              &PathTest::listUtf8Result,
              &PathTest::listUtf8Path,

              &PathTest::size,
              &PathTest::sizeEmpty,
              &PathTest::sizeNonSeekable,
              &PathTest::sizeEarlyEof,
              &PathTest::sizeDirectory,
              &PathTest::sizeNonexistent,
              &PathTest::sizeNonNullTerminated,
              &PathTest::sizeUtf8,

              &PathTest::read,
              &PathTest::readString,
              &PathTest::readEmpty,
              &PathTest::readEmptyString,
              &PathTest::readNonSeekable,
              &PathTest::readNonSeekableString,
              &PathTest::readEarlyEof,
              &PathTest::readEarlyEofString,
              &PathTest::readDirectory,
              &PathTest::readNonexistent,
              &PathTest::readNonNullTerminated,
              &PathTest::readUtf8,

              &PathTest::write,
              &PathTest::writeDisabledOverloads,
              &PathTest::writeEmpty,
              &PathTest::writeDirectory,
              &PathTest::writeNoPermission,
              &PathTest::writeNonNullTerminated,
              &PathTest::writeUtf8,

              &PathTest::append,
              &PathTest::appendDisabledOverloads,
              &PathTest::appendToNonexistent,
              &PathTest::appendEmpty,
              &PathTest::appendDirectory,
              &PathTest::appendNoPermission,
              &PathTest::appendNonNullTerminated,
              &PathTest::appendUtf8});

    addTests({&PathTest::copy},
        &PathTest::prepareFileToCopy,
        &PathTest::prepareFileToCopy);

    addTests({&PathTest::copyEmpty,
              &PathTest::copyDirectory,
              &PathTest::copyReadNonexistent,
              &PathTest::copyWriteNoPermission});

    addTests({&PathTest::copyNonNullTerminated},
        &PathTest::prepareFileToCopy,
        &PathTest::prepareFileToCopy);

    addTests({&PathTest::copyUtf8});

    #ifndef CORRADE_TARGET_EMSCRIPTEN
    addBenchmarks({
        &PathTest::copy100MReadWrite,
        &PathTest::copy100MReadWriteString,
        &PathTest::copy100MCopy,
        #if defined(DOXYGEN_GENERATING_OUTPUT) || defined(CORRADE_TARGET_UNIX) || (defined(CORRADE_TARGET_WINDOWS) && !defined(CORRADE_TARGET_WINDOWS_RT))
        &PathTest::copy100MMap
        #endif
        }, 5,
        &PathTest::prepareFileToBenchmarkCopy,
        &PathTest::prepareFileToBenchmarkCopy);
    #endif

    addTests({&PathTest::map,
              &PathTest::mapEmpty,
              &PathTest::mapDirectory,
              &PathTest::mapNonexistent,
              &PathTest::mapNonNullTerminated,
              &PathTest::mapUtf8,

              &PathTest::mapRead,
              &PathTest::mapReadEmpty,
              &PathTest::mapReadDirectory,
              &PathTest::mapReadNonexistent,
              &PathTest::mapReadNonNullTerminated,
              &PathTest::mapReadUtf8,

              &PathTest::mapWrite,
              &PathTest::mapWriteEmpty,
              &PathTest::mapWriteDirectory,
              &PathTest::mapWriteNoPermission,
              &PathTest::mapWriteNonNullTerminated,
              &PathTest::mapWriteUtf8});

    #ifdef CORRADE_TARGET_APPLE
    if(System::isSandboxed()
        #if defined(CORRADE_TARGET_IOS) && defined(CORRADE_TESTSUITE_TARGET_XCTEST)
        /** @todo Fix this once I persuade CMake to run XCTest tests properly */
        && std::getenv("SIMULATOR_UDID")
        #endif
    ) {
        _testDir = Path::join(Path::split(*Path::executableLocation()).first(), "PathTestFiles");
        _testDirSymlink = Path::join(Path::split(*Path::executableLocation()).first(), "PathTestFilesSymlink");
        _testDirUtf8 = Path::join(Path::split(*Path::executableLocation()).first(), "PathTestFilesUtf8");
        _writeTestDir = Path::join(*Path::homeDirectory(), "Library/Caches");
    } else
    #endif
    {
        _testDir = Containers::String::nullTerminatedView(PATH_TEST_DIR);
        _testDirSymlink = Containers::String::nullTerminatedView(PATH_TEST_DIR_SYMLINK);
        _testDirUtf8 = Containers::String::nullTerminatedView(PATH_TEST_DIR_UTF8);
        _writeTestDir = Containers::String::nullTerminatedView(PATH_WRITE_TEST_DIR);
    }

    /* Delete the file for copy tests to avoid using a stale version */
    if(Path::exists(Path::join(_writeTestDir, "copySource.dat")))
        Path::remove(Path::join(_writeTestDir, "copySource.dat"));
    if(Path::exists(Path::join(_writeTestDir, "copyBenchmarkSource.dat")))
        Path::remove(Path::join(_writeTestDir, "copyBenchmarkSource.dat"));
}

using namespace Containers::Literals;

void PathTest::fromNativeSeparators() {
    Containers::String nativeSeparators = Path::fromNativeSeparators("put\\ that/somewhere\\ else");
    #ifdef CORRADE_TARGET_WINDOWS
    CORRADE_COMPARE(nativeSeparators, "put/ that/somewhere/ else");
    #else
    CORRADE_COMPARE(nativeSeparators, "put\\ that/somewhere\\ else");
    #endif
}

#ifdef CORRADE_TARGET_WINDOWS
void PathTest::fromNativeSeparatorsSmall() {
    Containers::String in = "C:\\foo/";
    CORRADE_VERIFY(in.isSmall());
    CORRADE_COMPARE(Path::fromNativeSeparators(in), "C:/foo/");
}

void PathTest::fromNativeSeparatorsNonOwned() {
    const char* data = "put\\ that/somewhere\\ else";
    Containers::String in = Containers::String::nullTerminatedView(data);
    CORRADE_VERIFY(!in.isSmall());
    CORRADE_VERIFY(in.deleter());

    /* Will make a copy as it can't touch a potentially immutable data */
    Containers::String out = Path::fromNativeSeparators(std::move(in));
    CORRADE_COMPARE(out, "put/ that/somewhere/ else");
    CORRADE_VERIFY(out.data() != data);
}
#endif

void PathTest::toNativeSeparators() {
    Containers::String nativeSeparators = Path::toNativeSeparators("this\\is a weird/system\\right");
    #ifdef CORRADE_TARGET_WINDOWS
    CORRADE_COMPARE(nativeSeparators, "this\\is a weird\\system\\right");
    #else
    CORRADE_COMPARE(nativeSeparators, "this\\is a weird/system\\right");
    #endif
}

#ifdef CORRADE_TARGET_WINDOWS
void PathTest::toNativeSeparatorsSmall() {
    Containers::String in = "C:\\foo/";
    CORRADE_VERIFY(in.isSmall());
    CORRADE_COMPARE(Path::toNativeSeparators(in), "C:\\foo\\");
}

void PathTest::toNativeSeparatorsNonOwned() {
    const char* data = "this\\is a weird/system\\right";
    Containers::String in = Containers::String::nullTerminatedView(data);
    CORRADE_VERIFY(!in.isSmall());
    CORRADE_VERIFY(in.deleter());

    /* Will make a copy as it can't touch a potentially immutable data */
    Containers::String out = Path::toNativeSeparators(std::move(in));
    CORRADE_COMPARE(out, "this\\is a weird\\system\\right");
    CORRADE_VERIFY(out.data() != data);
}
#endif

void PathTest::split() {
    /* In case you're not sure about the behavior, cross-check with Python's
       os.path.split(). */

    /* Empty */
    CORRADE_COMPARE(Path::split(""),
        Containers::pair(""_s, ""_s));

    /* No path */
    CORRADE_COMPARE(Path::split("foo.txt"),
        Containers::pair(""_s, "foo.txt"_s));

    /* No filename */
    CORRADE_COMPARE(Path::split(".config/corrade/"),
        Containers::pair(".config/corrade"_s, ""_s));

    /* Common case */
    CORRADE_COMPARE(Path::split("foo/bar/map.conf"),
        Containers::pair("foo/bar"_s, "map.conf"_s));

    /* Absolute path */
    CORRADE_COMPARE(Path::split("/foo/bar/map.conf"),
        Containers::pair("/foo/bar"_s, "map.conf"_s));

    /* Absolute network path */
    CORRADE_COMPARE(Path::split("//computer/foo/bar/map.conf"),
        Containers::pair("//computer/foo/bar"_s, "map.conf"_s));

    /* Not dropping the root slash */
    CORRADE_COMPARE(Path::split("/root"),
        Containers::pair("/"_s, "root"_s));
    CORRADE_COMPARE(Path::split("/"),
        Containers::pair("/"_s, ""_s));

    /* Not dropping the double root slash */
    CORRADE_COMPARE(Path::split("//computer"),
        Containers::pair("//"_s, "computer"_s));
    CORRADE_COMPARE(Path::split("//"),
        Containers::pair("//"_s, ""_s));
}

void PathTest::splitFlags() {
    /* Empty should preserve both null-terminated flags */
    {
        Containers::Pair<Containers::StringView, Containers::StringView> a = Path::split(""_s);
        CORRADE_COMPARE(a, Containers::pair(""_s, ""_s));
        CORRADE_COMPARE(a.first().flags(), Containers::StringViewFlag::Global|Containers::StringViewFlag::NullTerminated);
        CORRADE_COMPARE(a.second().flags(), Containers::StringViewFlag::Global|Containers::StringViewFlag::NullTerminated);

    /* Filename only the second */
    } {
        Containers::Pair<Containers::StringView, Containers::StringView> a = Path::split("/path"_s);
        CORRADE_COMPARE(a, Containers::pair("/"_s, "path"_s));
        CORRADE_COMPARE(a.first().flags(), Containers::StringViewFlag::Global);
        CORRADE_COMPARE(a.second().flags(), Containers::StringViewFlag::Global|Containers::StringViewFlag::NullTerminated);

    /* Path alone only the second as / gets dropped */
    } {
        Containers::Pair<Containers::StringView, Containers::StringView> a = Path::split("path/"_s);
        CORRADE_COMPARE(a, Containers::pair("path"_s, ""_s));
        CORRADE_COMPARE(a.first().flags(), Containers::StringViewFlag::Global);
        CORRADE_COMPARE(a.second().flags(), Containers::StringViewFlag::Global|Containers::StringViewFlag::NullTerminated);

    /* Root alone both */
    } {
        Containers::Pair<Containers::StringView, Containers::StringView> a = Path::split("/"_s);
        CORRADE_COMPARE(a, Containers::pair("/"_s, ""_s));
        CORRADE_COMPARE(a.first().flags(), Containers::StringViewFlag::Global|Containers::StringViewFlag::NullTerminated);
        CORRADE_COMPARE(a.second().flags(), Containers::StringViewFlag::Global|Containers::StringViewFlag::NullTerminated);

    /* Non-literal should not be global */
    } {
        Containers::Pair<Containers::StringView, Containers::StringView> a = Path::split("path/file.txt");
        CORRADE_COMPARE(a, Containers::pair("path"_s, "file.txt"_s));
        CORRADE_COMPARE(a.first().flags(), Containers::StringViewFlags{});
        CORRADE_COMPARE(a.second().flags(), Containers::StringViewFlag::NullTerminated);
    }
}

void PathTest::splitExtension() {
    /* In case you're not sure about the behavior, cross-check with Python's
       os.path.splitext(). */

    /* Empty */
    CORRADE_COMPARE(Path::splitExtension(""),
        Containers::pair(""_s, ""_s));

    /* Common case */
    CORRADE_COMPARE(Path::splitExtension("file.txt"),
        Containers::pair("file"_s, ".txt"_s));

    /* Double extension */
    CORRADE_COMPARE(Path::splitExtension("file.tar.gz"),
        Containers::pair("file.tar"_s, ".gz"_s));

    /* No extension */
    CORRADE_COMPARE(Path::splitExtension("/etc/passwd"),
        Containers::pair("/etc/passwd"_s, ""_s));

    /* Dot not a part of the file */
    CORRADE_COMPARE(Path::splitExtension("/etc/rc.d/file"),
        Containers::pair("/etc/rc.d/file"_s, ""_s));

    /* Dot at the end */
    CORRADE_COMPARE(Path::splitExtension("/home/no."),
        Containers::pair("/home/no"_s, "."_s));

    /* Dotfile, prefixed or not */
    CORRADE_COMPARE(Path::splitExtension("/home/mosra/.bashrc"),
        Containers::pair("/home/mosra/.bashrc"_s, ""_s));
    CORRADE_COMPARE(Path::splitExtension(".bashrc"),
        Containers::pair(".bashrc"_s, ""_s));

    /* One level up, prefixed or not */
    CORRADE_COMPARE(Path::splitExtension("/home/mosra/Code/.."),
        Containers::pair("/home/mosra/Code/.."_s, ""_s));
    CORRADE_COMPARE(Path::splitExtension(".."),
        Containers::pair(".."_s, ""_s));

    /* This directory */
    CORRADE_COMPARE(Path::splitExtension("/home/mosra/."),
        Containers::pair("/home/mosra/."_s, ""_s));
    CORRADE_COMPARE(Path::splitExtension("."),
        Containers::pair("."_s, ""_s));

    /* More dots at the start */
    CORRADE_COMPARE(Path::splitExtension("... And Justice For All.mp3"),
        Containers::pair("... And Justice For All"_s, ".mp3"_s));
    CORRADE_COMPARE(Path::splitExtension("... And Justice For All"),
        Containers::pair("... And Justice For All"_s, ""_s));
}

void PathTest::splitExtensionFlags() {
    /* Empty should preserve both null-terminated flags */
    {
        Containers::Pair<Containers::StringView, Containers::StringView> a = Path::splitExtension(""_s);
        CORRADE_COMPARE(a.first().flags(), Containers::StringViewFlag::Global|Containers::StringViewFlag::NullTerminated);
        CORRADE_COMPARE(a.second().flags(), Containers::StringViewFlag::Global|Containers::StringViewFlag::NullTerminated);

    /* Filename with extension only the second */
    } {
        Containers::Pair<Containers::StringView, Containers::StringView> a = Path::splitExtension("file.txt"_s);
        CORRADE_COMPARE(a.first().flags(), Containers::StringViewFlag::Global);
        CORRADE_COMPARE(a.second().flags(), Containers::StringViewFlag::Global|Containers::StringViewFlag::NullTerminated);

    /* Extension-less file both */
    } {
        Containers::Pair<Containers::StringView, Containers::StringView> a = Path::splitExtension("file"_s);
        CORRADE_COMPARE(a.first().flags(), Containers::StringViewFlag::Global|Containers::StringViewFlag::NullTerminated);
        CORRADE_COMPARE(a.second().flags(), Containers::StringViewFlag::Global|Containers::StringViewFlag::NullTerminated);

    /* Non-literal should not be global */
    } {
        Containers::Pair<Containers::StringView, Containers::StringView> a = Path::splitExtension("file.txt");
        CORRADE_COMPARE(a.first().flags(), Containers::StringViewFlags{});
        CORRADE_COMPARE(a.second().flags(), Containers::StringViewFlag::NullTerminated);
    }
}

void PathTest::join() {
    /* Empty path */
    CORRADE_COMPARE(Path::join("", "/foo.txt"), "/foo.txt");

    /* Empty all */
    CORRADE_COMPARE(Path::join("", ""), "");

    /* Absolute filename */
    CORRADE_COMPARE(Path::join("/foo/bar", "/file.txt"), "/file.txt");

    /* Trailing slash */
    CORRADE_COMPARE(Path::join("/foo/bar/", "file.txt"), "/foo/bar/file.txt");

    /* Common case */
    CORRADE_COMPARE(Path::join("/foo/bar", "file.txt"), "/foo/bar/file.txt");
}

#ifdef CORRADE_TARGET_WINDOWS
void PathTest::joinWindows() {
    /* Drive letter */
    CORRADE_COMPARE(Path::join("/foo/bar", "X:/path/file.txt"), "X:/path/file.txt");
}
#endif

void PathTest::joinMultiple() {
    CORRADE_COMPARE(Path::join({"foo", "bar", "file.txt"}), "foo/bar/file.txt");
}

void PathTest::joinMultipleAbsolute() {
    CORRADE_COMPARE(Path::join({"foo", "/bar", "file.txt"}), "/bar/file.txt");
}

void PathTest::joinMultipleOneEmpty() {
    CORRADE_COMPARE(Path::join({"foo", "", "file.txt"}), "foo/file.txt");
}

void PathTest::joinMultipleJustOne() {
    CORRADE_COMPARE(Path::join({"file.txt"}), "file.txt");
}

void PathTest::joinMultipleNone() {
    CORRADE_COMPARE(Path::join({}), "");
}

void PathTest::exists() {
    /* File */
    CORRADE_VERIFY(Path::exists(Path::join(_testDir, "file")));

    /* Directory */
    CORRADE_VERIFY(Path::exists(_testDir));

    /* Nonexistent file */
    CORRADE_VERIFY(!Path::exists(Path::join(_testDir, "nonexistentFile")));

    /* Current directory, empty */
    CORRADE_VERIFY(Path::exists("."));
    CORRADE_VERIFY(!Path::exists(""));
}

void PathTest::existsNoPermission() {
    #ifdef CORRADE_TARGET_EMSCRIPTEN
    CORRADE_SKIP("Everything is accessible under Emscripten.");
    #else
    /* macOS or BSD doesn't have /proc */
    #if defined(__unix__) && !defined(CORRADE_TARGET_EMSCRIPTEN) && \
        !defined(__FreeBSD__) && !defined(__OpenBSD__) && !defined(__bsdi__) && \
        !defined(__NetBSD__) && !defined(__DragonFly__)
    /* Assuming there's no real possibility to run as root on Apple so this
       checks only other Unix systems */
    if(Path::homeDirectory() == "/root")
        CORRADE_SKIP("Running under root, can't test for permissions.");

    /* /proc/self/fd exists, PID 1 is the "root" process and should always
       exist -- thus exists() reporting true, but any attempts to read it
       should fail. */
    CORRADE_VERIFY(Path::exists("/proc/self/mem"));
    CORRADE_VERIFY(Path::exists("/proc/1"));
    CORRADE_VERIFY(Path::exists("/proc/1/mem"));
    /* Just to be sure we're not giving back bullshit -- a random file in the
       same inaccessible directory should fail, opening that inacessible file
       should fail */
    CORRADE_VERIFY(!Path::exists("/proc/1/nonexistent"));
    CORRADE_VERIFY(!Path::size("/proc/1/mem"));
    #else
    /** @todo find something that actually exists, to test the same case as on
        Unix. No idea what in C:/Program Files/WindowsApps could be guaranteed
        to exist. */
    CORRADE_SKIP("Not sure how to test this.");
    #endif
    #endif
}

void PathTest::existsNonNullTerminated() {
    CORRADE_VERIFY(Path::exists(Path::join(_testDir, "fileX").exceptSuffix(1)));
}

void PathTest::existsUtf8() {
    CORRADE_VERIFY(Path::exists(Path::join(_testDirUtf8, "hýždě")));
}

void PathTest::isDirectory() {
    {
        #if defined(CORRADE_TARGET_IOS) && defined(CORRADE_TESTSUITE_TARGET_XCTEST)
        CORRADE_EXPECT_FAIL_IF(!std::getenv("SIMULATOR_UDID"),
            "iOS (in a simulator) has no idea about file types.");
        #endif
        CORRADE_VERIFY(Path::isDirectory(Path::join(_testDir, "dir")));
    }

    CORRADE_VERIFY(!Path::isDirectory(Path::join(_testDir, "file")));

    /* Nonexistent file */
    CORRADE_VERIFY(!Path::isDirectory(Path::join(_testDir, "nonexistentFile")));
}

void PathTest::isDirectorySymlink() {
    CORRADE_VERIFY(Path::exists(Path::join(_testDirSymlink, "file-symlink")));
    CORRADE_VERIFY(!Path::isDirectory(Path::join(_testDirSymlink, "file-symlink")));

    CORRADE_VERIFY(Path::exists(Path::join(_testDirSymlink, "dir-symlink")));
    {
        #if !defined(CORRADE_TARGET_UNIX) && !defined(CORRADE_TARGET_EMSCRIPTEN)
        /* Possible on Windows too, but there we'd need to first detect if the
           Git clone has the symlinks preserved */
        CORRADE_EXPECT_FAIL("Symlink support is implemented on Unix systems and Emscripten only.");
        #endif
        #if defined(CORRADE_TARGET_IOS) && defined(CORRADE_TESTSUITE_TARGET_XCTEST)
        CORRADE_EXPECT_FAIL_IF(!std::getenv("SIMULATOR_UDID"),
            "iOS (in a simulator) has no idea about file types.");
        #endif
        CORRADE_VERIFY(Path::isDirectory(Path::join(_testDirSymlink, "dir-symlink")));
    }
}

void PathTest::isDirectoryNoPermission() {
    /* Similar to existsNoPermission(), but with isDirectory() being tested */

    #ifdef CORRADE_TARGET_EMSCRIPTEN
    CORRADE_SKIP("Everything is accessible under Emscripten.");
    #else
    /* macOS or BSD doesn't have /proc */
    #if defined(__unix__) && !defined(CORRADE_TARGET_EMSCRIPTEN) && \
        !defined(__FreeBSD__) && !defined(__OpenBSD__) && !defined(__bsdi__) && \
        !defined(__NetBSD__) && !defined(__DragonFly__)
    /* Assuming there's no real possibility to run as root on Apple so this
       checks only other Unix systems */
    if(Path::homeDirectory() == "/root")
        CORRADE_SKIP("Running under root, can't test for permissions.");

    /* /proc/self/fd exists and is a directory, PID 1 is the "root" process and
       should always exist, but we shouldn't be able to peek into it */
    CORRADE_VERIFY(Path::isDirectory("/proc/self/fd"));
    CORRADE_VERIFY(Path::isDirectory("/proc/1"));
    CORRADE_VERIFY(Path::isDirectory("/proc/1/fd"));
    /* Just to be sure we're not giving back bullshit -- a random file in the
       same inaccessible directory should fail, opening that inacessible file
       should fail */
    CORRADE_VERIFY(!Path::exists("/proc/1/nonexistent"));
    CORRADE_VERIFY(!Path::size("/proc/1/fd"));
    #elif defined(CORRADE_TARGET_WINDOWS)
    /* This file doesn't exist, but the whole contents of the WindowsApps
       directory is accessible only by the TrustedInstaller system user so this
       should fail */
    CORRADE_VERIFY(Path::isDirectory("C:/Program Files/WindowsApps"));
    CORRADE_VERIFY(!Path::isDirectory("C:/Program Files/WindowsApps/someDir"));
    #else
    CORRADE_SKIP("Not sure how to test this.");
    #endif
    #endif
}

void PathTest::isDirectoryNonNullTerminated() {
    {
        #if defined(CORRADE_TARGET_IOS) && defined(CORRADE_TESTSUITE_TARGET_XCTEST)
        CORRADE_EXPECT_FAIL_IF(!std::getenv("SIMULATOR_UDID"),
            "iOS (in a simulator) has no idea about file types.");
        #endif
        CORRADE_VERIFY(Path::isDirectory(Path::join(_testDir, "dirX").exceptSuffix(1)));
    }
    CORRADE_VERIFY(!Path::isDirectory(Path::join(_testDir, "fileX").exceptSuffix(1)));
}

void PathTest::isDirectoryUtf8() {
    {
        #if defined(CORRADE_TARGET_IOS) && defined(CORRADE_TESTSUITE_TARGET_XCTEST)
        CORRADE_EXPECT_FAIL_IF(!std::getenv("SIMULATOR_UDID"),
            "iOS (in a simulator) has no idea about file types.");
        #endif
        CORRADE_VERIFY(Path::isDirectory(Path::join(_testDirUtf8, "šňůra")));
    }
    CORRADE_VERIFY(!Path::isDirectory(Path::join(_testDirUtf8, "hýždě")));
}

void PathTest::make() {
    /* Existing */
    CORRADE_VERIFY(Path::exists(_testDir));
    /* Well... gotta make the test dir first to avoid failures later */
    CORRADE_VERIFY(Path::make(_writeTestDir));

    /* Leaf */
    Containers::String leaf = Path::join(_writeTestDir, "leaf");
    if(Path::exists(leaf))
        CORRADE_VERIFY(Path::remove(leaf));
    CORRADE_VERIFY(Path::make(leaf));
    CORRADE_VERIFY(Path::exists(leaf));

    /* Path */
    Containers::String path = Path::join(_writeTestDir, "path/to/new/dir");
    if(Path::exists(path))
        CORRADE_VERIFY(Path::remove(path));
    if(Path::exists(Path::join(_writeTestDir, "path/to/new")))
        CORRADE_VERIFY(Path::remove(Path::join(_writeTestDir, "path/to/new")));
    if(Path::exists(Path::join(_writeTestDir, "path/to")))
        CORRADE_VERIFY(Path::remove(Path::join(_writeTestDir, "path/to")));
    if(Path::exists(Path::join(_writeTestDir, "path")))
        CORRADE_VERIFY(Path::remove(Path::join(_writeTestDir, "path")));

    CORRADE_VERIFY(Path::make(leaf));
    CORRADE_VERIFY(Path::exists(leaf));

    /* Empty should be just a no-op without checking anything. Not like
       in Python, where `os.makedirs('', exist_ok=True)` stupidly fails with
        FileNotFoundError: [Errno 2] No such file or directory: '' */
    CORRADE_VERIFY(Path::make(""));
}

void PathTest::makeDotDotDot() {
    /* Creating current directory should be a no-op because it exists */
    CORRADE_VERIFY(Path::exists("."));
    {
        #ifdef CORRADE_TARGET_EMSCRIPTEN
        CORRADE_EXPECT_FAIL("Emscripten doesn't return EEXIST on mdkir(\".\") but fails instead.");
        #endif
        CORRADE_VERIFY(Path::make("."));
    }

    /* Parent as well */
    CORRADE_VERIFY(Path::exists(".."));
    {
        #ifdef CORRADE_TARGET_EMSCRIPTEN
        CORRADE_EXPECT_FAIL("Emscripten doesn't return EEXIST on mdkir(\"..\") but fails instead.");
        #endif
        CORRADE_VERIFY(Path::make(".."));
    }
}

void PathTest::makeNoPermission() {
    #ifdef CORRADE_TARGET_EMSCRIPTEN
    CORRADE_SKIP("Everything is writable under Emscripten.");
    #else
    #ifdef CORRADE_TARGET_APPLE
    /* Assuming there's no real possibility to run as root on Apple */
    Containers::StringView prefix = "/var/root";
    #elif defined(CORRADE_TARGET_ANDROID)
    /* Same here, would need a rooted device */
    Containers::StringView prefix = "/data/local";
    #elif defined(CORRADE_TARGET_UNIX)
    Containers::StringView prefix = "/root";
    if(Path::homeDirectory() == "/root")
        CORRADE_SKIP("Running under root, can't test for permissions.");
    #elif defined(CORRADE_TARGET_WINDOWS)
    /* Only the TrustedInstaller system user is supposed to have access into
       WindowsApps */
    Containers::StringView prefix = "C:/Program Files/WindowsApps";
    #else
    Containers::StringView prefix;
    CORRADE_SKIP("Not sure how to test on this system.");
    #endif

    std::ostringstream out;
    Error redirectError{&out};
    CORRADE_VERIFY(!Path::make(Path::join(prefix, "nope/never")));
    #ifndef CORRADE_TARGET_WINDOWS
    CORRADE_COMPARE_AS(out.str(),
        formatString("Utility::Path::make(): can't create {}/nope: error 13 (", prefix),
        TestSuite::Compare::StringHasPrefix);
    #else
    /* Windows APIs fill GetLastError() instead of errno, leading to a
       different code */
    CORRADE_COMPARE_AS(out.str(),
        formatString("Utility::Path::make(): can't create {}/nope: error 5 (", prefix),
        TestSuite::Compare::StringHasPrefix);
    #endif
    #endif
}

void PathTest::makeNonNullTerminated() {
    Containers::String leaf = Path::join(_writeTestDir, "leaf");
    if(Path::exists(leaf))
        CORRADE_VERIFY(Path::remove(leaf));
    CORRADE_VERIFY(Path::make(Path::join(_writeTestDir, "leafX").exceptSuffix(1)));
    CORRADE_VERIFY(Path::exists(leaf));
}

void PathTest::makeUtf8() {
    Containers::String leaf = Path::join(_writeTestDir, "šňůra");
    if(Path::exists(leaf))
        CORRADE_VERIFY(Path::remove(leaf));
    CORRADE_VERIFY(Path::make(leaf));
    CORRADE_VERIFY(Path::exists(leaf));
}

void PathTest::removeFile() {
    Containers::String file = Path::join(_writeTestDir, "file.txt");
    CORRADE_VERIFY(Path::make(_writeTestDir));
    CORRADE_VERIFY(Path::write(file, "a"_s));
    CORRADE_VERIFY(Path::exists(file));
    CORRADE_VERIFY(Path::remove(file));
    CORRADE_VERIFY(!Path::exists(file));
}

void PathTest::removeDirectory() {
    Containers::String directory = Path::join(_writeTestDir, "directory");
    CORRADE_VERIFY(Path::make(directory));
    CORRADE_VERIFY(Path::exists(directory));
    CORRADE_VERIFY(Path::remove(directory));
    CORRADE_VERIFY(!Path::exists(directory));
}

void PathTest::removeFileNonexistent() {
    CORRADE_VERIFY(!Path::exists("nonexistent"));

    std::ostringstream out;
    Error redirectError{&out};
    CORRADE_VERIFY(!Path::remove("nonexistent"));
    #ifdef CORRADE_TARGET_EMSCRIPTEN
    /* Emscripten uses a different errno for "No such file or directory" */
    CORRADE_COMPARE_AS(out.str(),
        "Utility::Path::remove(): can't remove nonexistent: error 44 (",
        TestSuite::Compare::StringHasPrefix);
    #else
    CORRADE_COMPARE_AS(out.str(),
        "Utility::Path::remove(): can't remove nonexistent: error 2 (",
        TestSuite::Compare::StringHasPrefix);
    #endif
}

void PathTest::removeDirectoryNonEmpty() {
    Containers::String directory = Path::join(_writeTestDir, "nonEmptyDirectory");
    CORRADE_VERIFY(Path::make(directory));
    CORRADE_VERIFY(Path::write(Path::join(directory, "file.txt"), "a"_s));

    std::ostringstream out;
    Error redirectError{&out};
    CORRADE_VERIFY(!Path::remove(directory));
    #ifdef CORRADE_TARGET_EMSCRIPTEN
    /* Emscripten uses a different errno for "Directory not empty",
       also there's a dedicated code path (and message) for directories */
    CORRADE_COMPARE_AS(out.str(),
        formatString("Utility::Path::remove(): can't remove directory {}: error 55 (", directory),
        TestSuite::Compare::StringHasPrefix);
    #elif defined(CORRADE_TARGET_WINDOWS)
    /* Windows also have a dedicated code path and message for dirs,
       "The directory is not empty." */
    CORRADE_COMPARE_AS(out.str(),
        formatString("Utility::Path::remove(): can't remove directory {}: error 145 (", directory),
        TestSuite::Compare::StringHasPrefix);
    #elif defined(CORRADE_TARGET_APPLE)
    /* Otherwise there's common handling for files and dirs, however Apple has
       to be special and also have a different error code for the same thing */
    CORRADE_COMPARE_AS(out.str(),
        formatString("Utility::Path::remove(): can't remove {}: error 66 (", directory),
        TestSuite::Compare::StringHasPrefix);
    #else
    CORRADE_COMPARE_AS(out.str(),
        formatString("Utility::Path::remove(): can't remove {}: error 39 (", directory),
        TestSuite::Compare::StringHasPrefix);
    #endif
}

void PathTest::removeFileNonNullTerminated() {
    Containers::String file = Path::join(_writeTestDir, "file.txt");
    CORRADE_VERIFY(Path::make(_writeTestDir));
    CORRADE_VERIFY(Path::write(file, "a"_s));
    CORRADE_VERIFY(Path::exists(file));
    CORRADE_VERIFY(Path::remove(Path::join(_writeTestDir, "file.txtX").exceptSuffix(1)));
    CORRADE_VERIFY(!Path::exists(file));
}

void PathTest::removeDirectoryNonNullTerminated() {
    Containers::String directory = Path::join(_writeTestDir, "directory");
    CORRADE_VERIFY(Path::make(directory));
    CORRADE_VERIFY(Path::exists(directory));
    CORRADE_VERIFY(Path::remove(Path::join(_writeTestDir, "directoryX").exceptSuffix(1)));
    CORRADE_VERIFY(!Path::exists(directory));
}

void PathTest::removeFileUtf8() {
    Containers::String file = Path::join(_writeTestDir, "hýždě.txt");
    CORRADE_VERIFY(Path::make(_writeTestDir));
    CORRADE_VERIFY(Path::write(file, "a"_s));
    CORRADE_VERIFY(Path::exists(file));
    CORRADE_VERIFY(Path::remove(file));
    CORRADE_VERIFY(!Path::exists(file));
}

void PathTest::removeDirectoryUtf8() {
    Containers::String directory = Path::join(_writeTestDir, "složka");
    CORRADE_VERIFY(Path::make(directory));
    CORRADE_VERIFY(Path::exists(directory));
    CORRADE_VERIFY(Path::remove(directory));
    CORRADE_VERIFY(!Path::exists(directory));
}

void PathTest::moveFile() {
    CORRADE_VERIFY(Path::make(_writeTestDir));

    /* Old file */
    Containers::String oldFile = Path::join(_writeTestDir, "oldFile.txt");
    CORRADE_VERIFY(Path::write(oldFile, "a"_s));

    /* New file, remove if exists */
    Containers::String newFile = Path::join(_writeTestDir, "newFile.txt");
    if(Path::exists(newFile))
        CORRADE_VERIFY(Path::remove(newFile));

    CORRADE_VERIFY(Path::move(oldFile, newFile));
    CORRADE_VERIFY(!Path::exists(oldFile));
    CORRADE_VERIFY(Path::exists(newFile));
}

void PathTest::moveDirectory() {
    /* Old directory, create if not exists */
    Containers::String oldDirectory = Path::join(_writeTestDir, "oldDirectory");
    if(!Path::exists(oldDirectory))
        CORRADE_VERIFY(Path::make(oldDirectory));

    /* New directory, remove if exists */
    Containers::String newDirectory = Path::join(_writeTestDir, "newDirectory");
    if(Path::exists(newDirectory))
        CORRADE_VERIFY(Path::remove(newDirectory));

    CORRADE_VERIFY(Path::move(oldDirectory, newDirectory));
    CORRADE_VERIFY(!Path::exists(oldDirectory));
    CORRADE_VERIFY(Path::exists(newDirectory));
}

void PathTest::moveSourceNonexistent() {
    CORRADE_VERIFY(Path::make(_writeTestDir));

    Containers::String to = Path::join(_writeTestDir, "empty");

    std::ostringstream out;
    Error redirectError{&out};
    CORRADE_VERIFY(!Path::move("nonexistent", to));
    #ifdef CORRADE_TARGET_EMSCRIPTEN
    /* Emscripten uses a different errno for "No such file or directory" */
    CORRADE_COMPARE_AS(out.str(),
        formatString("Utility::Path::move(): can't move nonexistent to {}: error 44 (", to),
        TestSuite::Compare::StringHasPrefix);
    #else
    CORRADE_COMPARE_AS(out.str(),
        formatString("Utility::Path::move(): can't move nonexistent to {}: error 2 (", to),
        TestSuite::Compare::StringHasPrefix);
    #endif
}

void PathTest::moveDestinationNoPermission() {
    #ifdef CORRADE_TARGET_EMSCRIPTEN
    CORRADE_SKIP("Everything is writable under Emscripten.");
    #else
    Containers::String from = Path::join(_testDir, "dir/dummy");
    #ifdef CORRADE_TARGET_APPLE
    /* Assuming there's no real possibility to run as root on Apple */
    Containers::StringView to = "/var/root/writtenFile";
    #elif defined(CORRADE_TARGET_ANDROID)
    /* Same here, would need a rooted device */
    Containers::StringView to = "/data/local/writtenFile";
    #elif defined(CORRADE_TARGET_UNIX)
    Containers::StringView to = "/root/writtenFile";
    if(Path::homeDirectory() == "/root")
        CORRADE_SKIP("Running under root, can't test for permissions.");
    #elif defined(CORRADE_TARGET_WINDOWS)
    /* Only the TrustedInstaller system user is supposed to have access into
       WindowsApps */
    Containers::StringView to = "C:/Program Files/WindowsApps/writtenFile";
    #else
    Containers::StringView to;
    CORRADE_SKIP("Not sure how to test on this system.");
    #endif

    std::ostringstream out;
    Error redirectError{&out};
    CORRADE_VERIFY(!Path::move(from, to));
    CORRADE_COMPARE_AS(out.str(),
        formatString("Utility::Path::move(): can't move {} to {}: error 13 (", from, to),
        TestSuite::Compare::StringHasPrefix);
    #endif
}

void PathTest::moveNonNullTerminated() {
    CORRADE_VERIFY(Path::make(_writeTestDir));

    /* Old file */
    Containers::String oldFile = Path::join(_writeTestDir, "oldFile.txt");
    CORRADE_VERIFY(Path::write(oldFile, "a"_s));

    /* New file, remove if exists */
    Containers::String newFile = Path::join(_writeTestDir, "newFile.txt");
    if(Path::exists(newFile))
        CORRADE_VERIFY(Path::remove(newFile));

    CORRADE_VERIFY(Path::move(
        Path::join(_writeTestDir, "oldFile.txtX").exceptSuffix(1),
        Path::join(_writeTestDir, "newFile.txtX").exceptSuffix(1)));
    CORRADE_VERIFY(!Path::exists(oldFile));
    CORRADE_VERIFY(Path::exists(newFile));
}

void PathTest::moveUtf8() {
    CORRADE_VERIFY(Path::make(_writeTestDir));

    /* Old file */
    Containers::String oldFile = Path::join(_writeTestDir, "starý hýždě.txt");
    CORRADE_VERIFY(Path::write(oldFile, "a"_s));

    /* New file, remove if exists */
    Containers::String newFile = Path::join(_writeTestDir, "nový hýždě.txt");
    if(Path::exists(newFile))
        CORRADE_VERIFY(Path::remove(newFile));

    CORRADE_VERIFY(Path::exists(oldFile));
    CORRADE_VERIFY(!Path::exists(newFile));
    CORRADE_VERIFY(Path::move(oldFile, newFile));
    CORRADE_VERIFY(!Path::exists(oldFile));
    CORRADE_VERIFY(Path::exists(newFile));
}

void PathTest::currentDirectory() {
    Containers::Optional<Containers::String> current = Path::currentDirectory();
    CORRADE_VERIFY(current);
    CORRADE_VERIFY(*current);
    CORRADE_INFO("Current directory found as:" << current);

    /* Ensure the test is not accidentally false positive due to stale files */
    if(Path::exists("currentPathTestDir.mark"))
        CORRADE_VERIFY(Path::remove("currentPathTestDir.mark"));

    /* Create a file on a relative path. If current directory is correctly
       queried, it should exist there */
    CORRADE_VERIFY(Path::write("currentPathTestDir.mark",
        "hi, i'm testing Utility::Path::current()"_s));
    CORRADE_VERIFY(Path::exists(Path::join(*current, "currentPathTestDir.mark")));

    /* Clean up after ourselves */
    CORRADE_VERIFY(Path::remove("currentPathTestDir.mark"));

    /* It shouldn't contain null bytes anywhere, especially not at the end */
    CORRADE_COMPARE_AS(*current, "\0"_s, TestSuite::Compare::StringNotContains);

    /* On Windows it shouldn't contain backslashes */
    #ifdef CORRADE_TARGET_WINDOWS
    CORRADE_COMPARE_AS(*current, "\\", TestSuite::Compare::StringNotContains);
    #endif
}

void PathTest::currentDirectoryNonexistent() {
    #ifdef CORRADE_TARGET_UNIX
    Containers::Optional<Containers::String> currentDirectory = Path::currentDirectory();
    CORRADE_VERIFY(currentDirectory);
    CORRADE_VERIFY(*currentDirectory);

    /* On Android the write dir is relative, so append it to current */
    Containers::String newCurrent = Path::join({*currentDirectory, _writeTestDir, "cwd"});
    CORRADE_VERIFY(Path::make(newCurrent));

    {
        /** @todo remove (and <unistd.h>) once Path::setCurrent() exists */
        Containers::ScopeGuard resetCurrent{&*currentDirectory, [](Containers::String* current) {
            chdir(current->data());
        }};
        chdir(newCurrent.data());
        CORRADE_COMPARE(Path::currentDirectory(), newCurrent);

        CORRADE_VERIFY(Path::exists("."));
        CORRADE_VERIFY(Path::remove(newCurrent));

        /* Interestingly, this doesn't fail */
        CORRADE_VERIFY(Path::exists("."));

        std::ostringstream out;
        Error redirectError{&out};
        CORRADE_VERIFY(!Path::currentDirectory());
        CORRADE_COMPARE_AS(out.str(),
            "Utility::Path::currentDirectory(): error 2 (",
            TestSuite::Compare::StringHasPrefix);
    }

    /* Verify that we're back to the original directory so other tests relying
       on it keep working. Should be also done in case anything above fails. */
    CORRADE_COMPARE(Path::currentDirectory(), currentDirectory);
    #else
    CORRADE_SKIP("Known to fail only on UNIX, not sure how to test elsewhere.");
    #endif
}

void PathTest::currentDirectoryUtf8() {
    /** @todo test once setCurrent() exists -- otherwise we'd have to implement
        UTF-16 conversion for Windows by hand here to pass to
        SetCurrentDirectory() and ... ew */
    CORRADE_SKIP("Not sure how to test this.");
}

#ifndef CORRADE_BUILD_STATIC
void PathTest::libraryLocation() {
    #if defined(CORRADE_TARGET_UNIX) || (defined(CORRADE_TARGET_WINDOWS) && !defined(CORRADE_TARGET_WINDOWS_RT))
    Containers::Optional<Containers::String> libraryLocation = Path::libraryLocation(&Path::remove);
    CORRADE_VERIFY(libraryLocation);
    CORRADE_VERIFY(*libraryLocation);
    CORRADE_INFO("Corrade::Utility library location found as:" << libraryLocation);

    {
        /* https://sourceware.org/bugzilla/show_bug.cgi?id=20292 probably?
           doesn't seem like that, but couldn't find anything else in the
           changelog that would be relevant */
        #ifdef __GLIBC__
        #if __GLIBC__*100 + __GLIBC_MINOR__ < 225
        CORRADE_EXPECT_FAIL("glibc < 2.25 returns executable location from dladdr()");
        #endif
        CORRADE_VERIFY(libraryLocation != Path::executableLocation());
        #endif

        /* There should be a TestSuite library next to this one */
        Containers::StringView testSuiteLibraryName =
            #ifdef CORRADE_TARGET_WINDOWS
            #ifdef __MINGW32__
            "lib"
            #endif
            #ifdef CORRADE_IS_DEBUG_BUILD
            "CorradeTestSuite-d.dll"
            #else
            "CorradeTestSuite.dll"
            #endif
            #elif defined(CORRADE_TARGET_APPLE)
            #ifdef CORRADE_IS_DEBUG_BUILD
            "libCorradeTestSuite-d.dylib"
            #else
            "libCorradeTestSuite.dylib"
            #endif
            #else
            #ifdef CORRADE_IS_DEBUG_BUILD
            "libCorradeTestSuite-d.so"
            #else
            "libCorradeTestSuite.so"
            #endif
            #endif
            ;
        CORRADE_VERIFY(Path::exists(Path::join(Path::split(*libraryLocation).first(), testSuiteLibraryName)));
    }

    /* It shouldn't contain null bytes anywhere, especially not at the end */
    CORRADE_COMPARE_AS(*libraryLocation, "\0"_s, TestSuite::Compare::StringNotContains);

    /* On Windows it shouldn't contain backslashes */
    #ifdef CORRADE_TARGET_WINDOWS
    CORRADE_COMPARE_AS(*libraryLocation, "\\", TestSuite::Compare::StringNotContains);
    #endif
    #else
    CORRADE_SKIP("Not implemented on this platform.");
    #endif
}
#else
void PathTest::libraryLocationStatic() {
    #if defined(CORRADE_TARGET_UNIX) || (defined(CORRADE_TARGET_WINDOWS) && !defined(CORRADE_TARGET_WINDOWS_RT))
    Containers::Optional<Containers::String> libraryLocation = Path::libraryLocation(&Path::remove);
    CORRADE_VERIFY(libraryLocation);
    CORRADE_VERIFY(*libraryLocation);
    CORRADE_INFO("Corrade::Utility library location found as:" << libraryLocation);

    /* No libraries in a static build, so this will print the final executable
       instead */
    #ifdef CORRADE_TARGET_WINDOWS
    CORRADE_COMPARE_AS(*libraryLocation,
        "UtilityPathTest.exe",
        TestSuite::Compare::StringHasSuffix);
    #else
    CORRADE_COMPARE_AS(*libraryLocation,
        "UtilityPathTest",
        TestSuite::Compare::StringHasSuffix);
    #endif
    #else
    CORRADE_SKIP("Not implemented on this platform.");
    #endif
}
#endif

void PathTest::libraryLocationNull() {
    #if defined(CORRADE_TARGET_UNIX) || (defined(CORRADE_TARGET_WINDOWS) && !defined(CORRADE_TARGET_WINDOWS_RT))
    std::ostringstream out;
    Error redirectError{&out};
    CORRADE_VERIFY(!Path::libraryLocation(nullptr));
    #ifdef CORRADE_TARGET_WINDOWS
    CORRADE_COMPARE_AS(out.str(),
        "Utility::Path::libraryLocation(): can't get library location: error 87 (",
        TestSuite::Compare::StringHasPrefix);
    #else
    CORRADE_COMPARE(out.str(), "Utility::Path::libraryLocation(): can't get library location\n");
    #endif
    #else
    CORRADE_SKIP("Not implemented on this platform.");
    #endif
}

void PathTest::libraryLocationInvalid() {
    #if defined(CORRADE_TARGET_UNIX) || (defined(CORRADE_TARGET_WINDOWS) && !defined(CORRADE_TARGET_WINDOWS_RT))
    std::ostringstream out;
    Error redirectError{&out};
    CORRADE_VERIFY(!Path::libraryLocation(reinterpret_cast<const void*>(0xbadcafe)));
    #ifdef CORRADE_TARGET_WINDOWS
    /* "The specified module could not be found." */
    CORRADE_COMPARE_AS(out.str(),
        "Utility::Path::libraryLocation(): can't get library location: error 126 (",
        TestSuite::Compare::StringHasPrefix);
    #else
    CORRADE_COMPARE(out.str(), "Utility::Path::libraryLocation(): can't get library location\n");
    #endif
    #else
    CORRADE_SKIP("Not implemented on this platform.");
    #endif
}

void PathTest::libraryLocationUtf8() {
    #if defined(CORRADE_TARGET_UNIX) || (defined(CORRADE_TARGET_WINDOWS) && !defined(CORRADE_TARGET_WINDOWS_RT))
    CORRADE_SKIP("Not sure how to test this.");
    #else
    CORRADE_SKIP("Not implemented on this platform.");
    #endif
}

void PathTest::executableLocation() {
    Containers::Optional<Containers::String> executableLocation = Path::executableLocation();
    CORRADE_VERIFY(executableLocation);
    CORRADE_VERIFY(*executableLocation);
    CORRADE_INFO("Executable location found as:" << executableLocation);

    /* On sandboxed macOS and iOS verify that the directory contains Info.plist
       file */
    #ifdef CORRADE_TARGET_APPLE
    if(System::isSandboxed()) {
        #if defined(CORRADE_TARGET_IOS) && defined(CORRADE_TESTSUITE_TARGET_XCTEST)
        CORRADE_EXPECT_FAIL_IF(!std::getenv("SIMULATOR_UDID"),
            "CTest is not able to run XCTest executables properly in the simulator.");
        #endif

        CORRADE_VERIFY(Path::exists(Path::join(Path::split(*executableLocation).first(), "Info.plist")));
    } else
    #endif

    /* On Emscripten we should have access to the bundled files */
    #ifdef CORRADE_TARGET_EMSCRIPTEN
    CORRADE_VERIFY(Path::exists(Path::join(Path::split(*executableLocation).first(), "PathTestFiles")));

    /* On Android we can't be sure about anything, so just test that the
       executable exists and it has access to the bundled files */
    #elif defined(CORRADE_TARGET_ANDROID)
    CORRADE_VERIFY(Path::exists(*executableLocation));
    CORRADE_COMPARE_AS(*executableLocation,
        "UtilityPathTest",
        TestSuite::Compare::StringContains);
    CORRADE_VERIFY(Path::exists(Path::join(Path::split(*executableLocation).first(), "PathTestFiles")));

    /* Otherwise it should contain other executables and libraries as we put
       all together */
    #else
    {
        #ifndef CORRADE_TARGET_WINDOWS
        CORRADE_VERIFY(Path::exists(Path::join(Path::split(*executableLocation).first(), "corrade-rc")));
        #else
        CORRADE_VERIFY(Path::exists(Path::join(Path::split(*executableLocation).first(), "corrade-rc.exe")));
        #endif
    }
    #endif

    /* It shouldn't contain null bytes anywhere, especially not at the end */
    CORRADE_COMPARE_AS(*executableLocation, "\0"_s, TestSuite::Compare::StringNotContains);

    /* On Windows it shouldn't contain backslashes */
    #ifdef CORRADE_TARGET_WINDOWS
    CORRADE_COMPARE_AS(*executableLocation, "\\", TestSuite::Compare::StringNotContains);
    #endif
}

void PathTest::executableLocationInvalid() {
    CORRADE_SKIP("Not sure how to test this.");
}

void PathTest::executableLocationUtf8() {
    CORRADE_SKIP("Not sure how to test this.");
}

void PathTest::homeDirectory() {
    Containers::Optional<Containers::String> homeDirectory = Path::homeDirectory();
    CORRADE_VERIFY(homeDirectory);
    CORRADE_VERIFY(*homeDirectory);
    CORRADE_INFO("Home directory found as:" << homeDirectory);

    /* On macOS and iOS verify that the home dir contains `Library` directory */
    #ifdef CORRADE_TARGET_APPLE
    CORRADE_VERIFY(Path::exists(Path::join(*homeDirectory, "Library")));

    /* On other Unixes (except Android, which is shit) verify that the home dir
       contains `.local` directory or is /root. Ugly and hacky, but it's the
       best I came up with. Can't test for e.g. `/home/` substring, as that can
       be overridden. */
    #elif defined(CORRADE_TARGET_UNIX) && !defined(CORRADE_TARGET_ANDROID)
    CORRADE_VERIFY(Path::exists(*homeDirectory));
    CORRADE_VERIFY(Path::exists(Path::join(*homeDirectory, ".local")) || homeDirectory == "/root");

    /* On Emscripten verify that the directory exists (it's empty by default) */
    #elif defined(CORRADE_TARGET_EMSCRIPTEN)
    CORRADE_VERIFY(Path::exists(*homeDirectory));

    /* On Windows verify that the home dir contains `desktop.ini` file. Ugly
       and hacky, but it's the best I came up with. Can't test for e.g.
       `/Users/` substring, as that can be overridden. */
    #elif defined(CORRADE_TARGET_WINDOWS)
    CORRADE_VERIFY(Path::exists(Path::join(*homeDirectory, "desktop.ini")));

    /* No idea elsewhere */
    #else
    {
        CORRADE_EXPECT_FAIL("Not implemented yet.");
        CORRADE_COMPARE(*homeDirectory, "(not implemented)");
    }
    #endif

    /* It shouldn't contain null bytes anywhere, especially not at the end */
    CORRADE_COMPARE_AS(*homeDirectory, "\0"_s, TestSuite::Compare::StringNotContains);

    /* On Windows it shouldn't contain backslashes */
    #ifdef CORRADE_TARGET_WINDOWS
    CORRADE_COMPARE_AS(*homeDirectory, "\\", TestSuite::Compare::StringNotContains);
    #endif
}

void PathTest::homeDirectoryInvalid() {
    /* Could be tested by temporarily removing $HOME, but ... ahem */
    CORRADE_SKIP("Not sure how to test this.");
}

void PathTest::homeDirectoryUtf8() {
    CORRADE_SKIP("Not sure how to test this.");
}

void PathTest::configurationDirectory() {
    Containers::Optional<Containers::String> configurationDirectory = Path::configurationDirectory("Corrade");
    CORRADE_VERIFY(configurationDirectory);
    CORRADE_VERIFY(*configurationDirectory);
    CORRADE_INFO("Configuration dir found as:" << configurationDirectory);

    #ifdef CORRADE_TARGET_APPLE
    CORRADE_COMPARE_AS(*configurationDirectory,
        "Corrade",
        TestSuite::Compare::StringHasSuffix);
    if(System::isSandboxed())
        CORRADE_VERIFY(Path::exists(Path::join(Path::split(Path::split(*configurationDirectory).first()).first(), "Caches")));
    else
        /* App Store is not present on *some* Travis VMs since 2018-08-05.
           CrashReporter is. */
        CORRADE_VERIFY(
            Path::exists(Path::join(Path::split(*configurationDirectory).first(), "App Store")) ||
            Path::exists(Path::join(Path::split(*configurationDirectory).first(), "CrashReporter")));

    /* On Linux verify that the parent dir contains `autostart` directory,
       something from GTK or something from Qt. Ugly and hacky, but it's the
       best I could come up with. Can't test for e.g. `/home/` substring, as
       that can be overridden. */
    #elif defined(__linux__) && !defined(CORRADE_TARGET_ANDROID)
    CORRADE_COMPARE_AS(*configurationDirectory,
        "corrade",
        TestSuite::Compare::StringHasSuffix);
    CORRADE_VERIFY(
        Path::exists(Path::join(Path::split(*configurationDirectory).first(), "autostart")) ||
        Path::exists(Path::join(Path::split(*configurationDirectory).first(), "dconf")) ||
        Path::exists(Path::join(Path::split(*configurationDirectory).first(), "Trolltech.conf")));

    /* Emscripten -- just compare to hardcoded value */
    #elif defined(CORRADE_TARGET_EMSCRIPTEN)
    CORRADE_COMPARE(*configurationDirectory, "/home/web_user/.config/corrade");

    /* On Windows verify that the parent dir contains `Microsoft` subdirectory.
       Ugly and hacky, but it's the best I came up with. Can't test for e.g.
       `/Users/` substring, as that can be overridden. */
    #elif defined(CORRADE_TARGET_WINDOWS)
    CORRADE_COMPARE_AS(*configurationDirectory,
        "Corrade",
        TestSuite::Compare::StringHasSuffix);
    CORRADE_VERIFY(Path::exists(Path::join(Path::split(*configurationDirectory).first(), "Microsoft")));

    /* No idea elsewhere */
    #else
    {
        CORRADE_EXPECT_FAIL("Not implemented yet.");
        CORRADE_COMPARE(*configurationDirectory, "(not implemented)");
    }
    #endif

    /* It shouldn't contain null bytes anywhere, especially not at the end */
    CORRADE_COMPARE_AS(*configurationDirectory, "\0"_s, TestSuite::Compare::StringNotContains);

    /* On Windows it shouldn't contain backslashes */
    #ifdef CORRADE_TARGET_WINDOWS
    CORRADE_COMPARE_AS(*configurationDirectory, "\\", TestSuite::Compare::StringNotContains);
    #endif
}

void PathTest::configurationDirectoryInvalid() {
    /* Could be tested by temporarily removing $XDG_CONFIG_HOME and $HOME, but
       ... ahem */
    CORRADE_SKIP("Not sure how to test this.");
}

void PathTest::configurationDirectoryUtf8() {
    CORRADE_SKIP("Not sure how to test this.");
}

void PathTest::temporaryDirectory() {
    Containers::Optional<Containers::String> temporaryDirectory = Path::temporaryDirectory();
    CORRADE_VERIFY(temporaryDirectory);
    CORRADE_VERIFY(*temporaryDirectory);
    CORRADE_INFO("Temporary dir found as:" << temporaryDirectory);

    #if defined(CORRADE_TARGET_UNIX) || defined(CORRADE_TARGET_EMSCRIPTEN)
    {
        #if defined(CORRADE_TARGET_IOS) && defined(CORRADE_TESTSUITE_TARGET_XCTEST)
        CORRADE_EXPECT_FAIL_IF(!std::getenv("SIMULATOR_UDID"),
            "CTest is not able to run XCTest executables properly in the simulator.");
        #endif
        CORRADE_VERIFY(Path::exists(*temporaryDirectory));
    }
    CORRADE_COMPARE_AS(*temporaryDirectory,
        "tmp",
        TestSuite::Compare::StringContains);

    #elif defined(CORRADE_TARGET_WINDOWS) && !defined(CORRADE_TARGET_WINDOWS_RT)
    CORRADE_VERIFY(Path::exists(*temporaryDirectory));
    #ifndef __MINGW32__
    CORRADE_COMPARE_AS(*temporaryDirectory,
        "Temp",
        TestSuite::Compare::StringContains);
    #else
    /* MinGW shell maps temp to a different directory, e.g. C:/msys64/tmp, so
       check for both */
    /** @todo needs some StringContainsAny, taking a list */
    if(temporaryDirectory->contains("tmp"))
        CORRADE_COMPARE_AS(*temporaryDirectory,
            "tmp",
            TestSuite::Compare::StringContains);
    else
        CORRADE_COMPARE_AS(*temporaryDirectory,
            "Temp",
            TestSuite::Compare::StringContains);
    #endif

    /* No idea elsewhere */
    #else
    {
        CORRADE_EXPECT_FAIL("Not implemented yet.");
        CORRADE_COMPARE(temporaryDirectory, "(not implemented)");
    }
    #endif

    /* It shouldn't contain null bytes anywhere, especially not at the end */
    CORRADE_COMPARE_AS(*temporaryDirectory, "\0"_s, TestSuite::Compare::StringNotContains);

    /* On Windows it shouldn't contain backslashes */
    #ifdef CORRADE_TARGET_WINDOWS
    CORRADE_COMPARE_AS(*temporaryDirectory, "\\", TestSuite::Compare::StringNotContains);
    #endif

    /* Verify that it's possible to write stuff there */
    {
        #if defined(CORRADE_TARGET_IOS) && defined(CORRADE_TESTSUITE_TARGET_XCTEST)
        CORRADE_EXPECT_FAIL_IF(!std::getenv("SIMULATOR_UDID"),
            "CTest is not able to run XCTest executables properly in the simulator.");
        #endif
        CORRADE_VERIFY(Path::write(Path::join(*temporaryDirectory, "a"), "hello"_s));
        CORRADE_VERIFY(Path::remove(Path::join(*temporaryDirectory, "a")));
    }
}

void PathTest::temporaryDirectoryInvalid() {
    CORRADE_SKIP("Not known to fail on any known system.");
}

void PathTest::temporaryDirectoryUtf8() {
    CORRADE_SKIP("Not sure how to test this.");
}

void PathTest::list() {
    Containers::Optional<Containers::Array<Containers::String>> list = Path::list(_testDir);
    CORRADE_VERIFY(list);

    {
        #if defined(CORRADE_TARGET_IOS) && defined(CORRADE_TESTSUITE_TARGET_XCTEST)
        CORRADE_EXPECT_FAIL_IF(!std::getenv("SIMULATOR_UDID"),
            "CTest is not able to run XCTest executables properly in the simulator.");
        #endif
        CORRADE_COMPARE_AS(*list, Containers::array<Containers::String>({
            ".", "..", "dir", "file"
        }), TestSuite::Compare::SortedContainer);
    }
}

void PathTest::listIterateRangeFor() {
    /* Verify that the directory can be listed to make the unconditional
       dereference in the for() below unlikely to assert */
    CORRADE_VERIFY(Path::list(_testDir));

    /* It should not happen that the original Optional somehow gets out of
       scope before we get to iterating the array contained in it. This is a
       yet-unsolved problem in C++ with std::optional and other STL containers:
       http://josuttis.com/download/std/D2012R0_fix_rangebasedfor_201029.pdf

       However, in this case, and unlike with std::make, the operator*()
       returns a T instead of T&&, and the reference lifetime extension takes
       care of the rest. See also OptionalTest::accessRvalueLifetimeExtension(). */
    Containers::Array<Containers::String> out;
    for(Containers::String& a: *Path::list(_testDir))
        arrayAppend(out, a);

    #if defined(CORRADE_TARGET_IOS) && defined(CORRADE_TESTSUITE_TARGET_XCTEST)
    CORRADE_EXPECT_FAIL_IF(!std::getenv("SIMULATOR_UDID"),
        "CTest is not able to run XCTest executables properly in the simulator.");
    #endif
    CORRADE_COMPARE_AS(out, Containers::array<Containers::String>({
        ".", "..", "dir", "file"
    }), TestSuite::Compare::SortedContainer);
}

void PathTest::listEmptyDirectory() {
    /* Create an empty directory. To be sure it's empty, recreate it. */
    Containers::String emptyDir = Path::join(_writeTestDir, "EmptyDir");
    if(Path::exists(emptyDir))
        CORRADE_VERIFY(Path::remove(emptyDir));
    CORRADE_VERIFY(Path::make(emptyDir));

    /* It shouldn't fail if there's nothing inside */
    Containers::Optional<Containers::Array<Containers::String>> list = Path::list(emptyDir);
    CORRADE_VERIFY(list);

    {
        #if defined(CORRADE_TARGET_IOS) && defined(CORRADE_TESTSUITE_TARGET_XCTEST)
        CORRADE_EXPECT_FAIL_IF(!std::getenv("SIMULATOR_UDID"),
            "CTest is not able to run XCTest executables properly in the simulator.");
        #endif
        CORRADE_COMPARE_AS(*list, Containers::array<Containers::String>({
            ".", ".."
        }), TestSuite::Compare::SortedContainer);
    }
}

void PathTest::listSkipDirectories() {
    Containers::Optional<Containers::Array<Containers::String>> list = Path::list(_testDir, Path::ListFlag::SkipDirectories);
    CORRADE_VERIFY(list);

    {
        #if defined(CORRADE_TARGET_IOS) && defined(CORRADE_TESTSUITE_TARGET_XCTEST)
        CORRADE_EXPECT_FAIL_IF(!std::getenv("SIMULATOR_UDID"),
            "CTest is not able to run XCTest executables properly in the simulator.");
        #endif
        CORRADE_COMPARE_AS(*list, Containers::array<Containers::String>({
            "file"
        }), TestSuite::Compare::SortedContainer);
    }
}

void PathTest::listSkipDirectoriesSymlinks() {
    Containers::Optional<Containers::Array<Containers::String>> list = Path::list(_testDirSymlink, Path::ListFlag::SkipDirectories);
    CORRADE_VERIFY(list);

    {
        #if defined(CORRADE_TARGET_IOS) && defined(CORRADE_TESTSUITE_TARGET_XCTEST)
        CORRADE_EXPECT_FAIL_IF(!std::getenv("SIMULATOR_UDID"),
            "CTest is not able to run XCTest executables properly in the simulator.");
        #endif
        #if !defined(CORRADE_TARGET_UNIX) && !defined(CORRADE_TARGET_EMSCRIPTEN)
        /* Possible on Windows too, but there we'd need to first detect if the
           Git clone has the symlinks preserved */
        CORRADE_EXPECT_FAIL("Symlink support is implemented on Unix systems and Emscripten only.");
        #endif
        CORRADE_COMPARE_AS(*list, Containers::array<Containers::String>({
            "file", "file-symlink"
        }), TestSuite::Compare::SortedContainer);
    }
}

void PathTest::listSkipFiles() {
    Containers::Optional<Containers::Array<Containers::String>> list = Path::list(_testDir, Path::ListFlag::SkipFiles);
    CORRADE_VERIFY(list);

    {
        #if defined(CORRADE_TARGET_IOS) && defined(CORRADE_TESTSUITE_TARGET_XCTEST)
        CORRADE_EXPECT_FAIL_IF(!std::getenv("SIMULATOR_UDID"),
            "CTest is not able to run XCTest executables properly in the simulator.");
        #endif
        CORRADE_COMPARE_AS(*list, Containers::array<Containers::String>({
            ".", "..", "dir"
        }), TestSuite::Compare::SortedContainer);
    }
}

void PathTest::listSkipFilesSymlinks() {
    Containers::Optional<Containers::Array<Containers::String>> list = Path::list(_testDirSymlink, Path::ListFlag::SkipFiles);
    CORRADE_VERIFY(list);

    {
        #if defined(CORRADE_TARGET_IOS) && defined(CORRADE_TESTSUITE_TARGET_XCTEST)
        CORRADE_EXPECT_FAIL_IF(!std::getenv("SIMULATOR_UDID"),
            "CTest is not able to run XCTest executables properly in the simulator.");
        #endif
        #if !defined(CORRADE_TARGET_UNIX) && !defined(CORRADE_TARGET_EMSCRIPTEN)
        /* Possible on Windows too, but there we'd need to first detect if the
           Git clone has the symlinks preserved */
        CORRADE_EXPECT_FAIL("Symlink support is implemented on Unix systems and Emscripten only.");
        #endif
        CORRADE_COMPARE_AS(*list, Containers::array<Containers::String>({
            ".", "..", "dir", "dir-symlink"
        }), TestSuite::Compare::SortedContainer);
    }
}

void PathTest::listSkipSpecial() {
    Containers::Optional<Containers::Array<Containers::String>> list = Path::list(_testDir, Path::ListFlag::SkipSpecial);
    CORRADE_VERIFY(list);

    {
        #if defined(CORRADE_TARGET_IOS) && defined(CORRADE_TESTSUITE_TARGET_XCTEST)
        CORRADE_EXPECT_FAIL_IF(!std::getenv("SIMULATOR_UDID"),
            "CTest is not able to run XCTest executables properly in the simulator.");
        #endif
        CORRADE_COMPARE_AS(*list, Containers::array<Containers::String>({
            ".", "..", "dir", "file"
        }), TestSuite::Compare::SortedContainer);
    }
}

void PathTest::listSkipSpecialSymlink() {
    /* Symlinks should not be treated as special files, so they're shown */

    Containers::Optional<Containers::Array<Containers::String>> list = Path::list(_testDirSymlink, Path::ListFlag::SkipSpecial);
    CORRADE_VERIFY(list);

    {
        #if defined(CORRADE_TARGET_IOS) && defined(CORRADE_TESTSUITE_TARGET_XCTEST)
        CORRADE_EXPECT_FAIL_IF(!std::getenv("SIMULATOR_UDID"),
            "CTest is not able to run XCTest executables properly in the simulator.");
        #endif
        CORRADE_COMPARE_AS(*list, Containers::array<Containers::String>({
            ".", "..", "dir", "dir-symlink", "file", "file-symlink"
        }), TestSuite::Compare::SortedContainer);
    }
}

void PathTest::listSkipDotAndDotDot() {
    Containers::Optional<Containers::Array<Containers::String>> list = Path::list(_testDir, Path::ListFlag::SkipDotAndDotDot);
    CORRADE_VERIFY(list);

    {
        #if defined(CORRADE_TARGET_IOS) && defined(CORRADE_TESTSUITE_TARGET_XCTEST)
        CORRADE_EXPECT_FAIL_IF(!std::getenv("SIMULATOR_UDID"),
            "CTest is not able to run XCTest executables properly in the simulator.");
        #endif
        CORRADE_COMPARE_AS(*list, Containers::array<Containers::String>({
            "dir", "file"
        }), TestSuite::Compare::SortedContainer);
    }
}

void PathTest::listSkipEverything() {
    Containers::Optional<Containers::Array<Containers::String>> list = Path::list(_testDir, Path::ListFlag::SkipFiles|Path::ListFlag::SkipDirectories);
    CORRADE_VERIFY(list);

    {
        #if defined(CORRADE_TARGET_IOS) && defined(CORRADE_TESTSUITE_TARGET_XCTEST)
        CORRADE_EXPECT_FAIL_IF(!std::getenv("SIMULATOR_UDID"),
            "CTest is not able to run XCTest executables properly in the simulator.");
        #endif
        CORRADE_COMPARE_AS(*list, Containers::array<Containers::String>({
        }), TestSuite::Compare::SortedContainer);
    }
}

void PathTest::listSort() {
    {
        Containers::Optional<Containers::Array<Containers::String>> list = Path::list(_testDir, Path::ListFlag::SortAscending);
        CORRADE_VERIFY(list);

        #if defined(CORRADE_TARGET_IOS) && defined(CORRADE_TESTSUITE_TARGET_XCTEST)
        CORRADE_EXPECT_FAIL_IF(!std::getenv("SIMULATOR_UDID"),
            "CTest is not able to run XCTest executables properly in the simulator.");
        #endif
        CORRADE_COMPARE_AS(*list, Containers::array<Containers::String>({
            ".", "..", "dir", "file"
        }), TestSuite::Compare::Container);
    } {
        Containers::Optional<Containers::Array<Containers::String>> list = Path::list(_testDir, Path::ListFlag::SortDescending);
        CORRADE_VERIFY(list);

        #if defined(CORRADE_TARGET_IOS) && defined(CORRADE_TESTSUITE_TARGET_XCTEST)
        CORRADE_EXPECT_FAIL_IF(!std::getenv("SIMULATOR_UDID"),
            "CTest is not able to run XCTest executables properly in the simulator.");
        #endif
        CORRADE_COMPARE_AS(*list, Containers::array<Containers::String>({
            "file", "dir", "..", "."
        }), TestSuite::Compare::Container);
    } {
        /* Ascending and descending together will pick ascending */
        Containers::Optional<Containers::Array<Containers::String>> list = Path::list(_testDir, Path::ListFlag::SortAscending|Path::ListFlag::SortDescending);
        CORRADE_VERIFY(list);

        #if defined(CORRADE_TARGET_IOS) && defined(CORRADE_TESTSUITE_TARGET_XCTEST)
        CORRADE_EXPECT_FAIL_IF(!std::getenv("SIMULATOR_UDID"),
            "CTest is not able to run XCTest executables properly in the simulator.");
        #endif
        CORRADE_COMPARE_AS(*list, Containers::array<Containers::String>({
            ".", "..", "dir", "file"
        }), TestSuite::Compare::Container);
    }
}

void PathTest::listNonexistent() {
    std::ostringstream out;
    Error redirectError{&out};
    CORRADE_VERIFY(!Path::list("nonexistent"));
    #ifdef CORRADE_TARGET_WINDOWS
    /* Windows has its own code path and thus different errors */
    CORRADE_COMPARE_AS(out.str(),
        "Utility::Path::list(): can't list nonexistent: error 3 (",
        TestSuite::Compare::StringHasPrefix);
    #elif defined(CORRADE_TARGET_EMSCRIPTEN)
    /* Emscripten uses a different errno for "No such file or directory" */
    CORRADE_COMPARE_AS(out.str(),
        "Utility::Path::list(): can't list nonexistent: error 44 (",
        TestSuite::Compare::StringHasPrefix);
    #else
    CORRADE_COMPARE_AS(out.str(),
        "Utility::Path::list(): can't list nonexistent: error 2 (",
        TestSuite::Compare::StringHasPrefix);
    #endif
}

void PathTest::listNonNullTerminated() {
    Containers::Optional<Containers::Array<Containers::String>> list = Path::list((_testDir + "X").exceptSuffix(1));
    CORRADE_VERIFY(list);

    {
        #if defined(CORRADE_TARGET_IOS) && defined(CORRADE_TESTSUITE_TARGET_XCTEST)
        CORRADE_EXPECT_FAIL_IF(!std::getenv("SIMULATOR_UDID"),
            "CTest is not able to run XCTest executables properly in the simulator.");
        #endif
        CORRADE_COMPARE_AS(*list, Containers::array<Containers::String>({
            ".", "..", "dir", "file"
        }), TestSuite::Compare::SortedContainer);
    }
}

void PathTest::listTrailingSlash() {
    /* Should have the same result as without */
    Containers::Optional<Containers::Array<Containers::String>> list = Path::list(_testDir + "/");
    CORRADE_VERIFY(list);

    {
        #if defined(CORRADE_TARGET_IOS) && defined(CORRADE_TESTSUITE_TARGET_XCTEST)
        CORRADE_EXPECT_FAIL_IF(!std::getenv("SIMULATOR_UDID"),
            "CTest is not able to run XCTest executables properly in the simulator.");
        #endif
        CORRADE_COMPARE_AS(*list, Containers::array<Containers::String>({
            ".", "..", "dir", "file"
        }), TestSuite::Compare::SortedContainer);
    }
}

void PathTest::listUtf8Result() {
    const Containers::String list[]{".", "..", "hýždě", "šňůra"};

    Containers::Optional<Containers::Array<Containers::String>> actual = Path::list(_testDirUtf8, Path::ListFlag::SortAscending);
    CORRADE_VERIFY(actual);

    #ifdef CORRADE_TARGET_APPLE
    /* Apple HFS+ stores filenames in a decomposed normalized form to avoid
       e.g. `e` + `ˇ` and `ě` being treated differently. That makes sense. I
       wonder why neither Linux nor Windows do this. */
    const Containers::String listDecomposedUnicode[]{".", "..", "hýždě", "šňůra"};
    CORRADE_COMPARE_AS(list[3],
        listDecomposedUnicode[3],
        TestSuite::Compare::NotEqual);
    #endif

    #ifdef CORRADE_TARGET_APPLE
    #if defined(CORRADE_TARGET_IOS) && defined(CORRADE_TESTSUITE_TARGET_XCTEST)
    CORRADE_EXPECT_FAIL_IF(!std::getenv("SIMULATOR_UDID"),
        "CTest is not able to run XCTest executables properly in the simulator.");
    #endif

    /* However, Apple systems still can use filesystems other than HFS+, so
       be prepared that it can compare to either */
    if((*actual)[3] == listDecomposedUnicode[3]) {
        CORRADE_COMPARE_AS(*actual,
            Containers::arrayView(listDecomposedUnicode),
            TestSuite::Compare::Container);
    } else
    #endif
    {
        CORRADE_COMPARE_AS(*actual,
            Containers::arrayView(list),
            TestSuite::Compare::Container);
    }
}

void PathTest::listUtf8Path() {
    const Containers::String list[]{".", "..", "dummy", "klíče"};

    Containers::Optional<Containers::Array<Containers::String>> actual = Path::list(Path::join(_testDirUtf8, "šňůra"), Path::ListFlag::SortAscending);
    CORRADE_VERIFY(actual);

    #if defined(CORRADE_TARGET_IOS) && defined(CORRADE_TESTSUITE_TARGET_XCTEST)
    CORRADE_EXPECT_FAIL_IF(!std::getenv("SIMULATOR_UDID"),
        "CTest is not able to run XCTest executables properly in the simulator.");
    #endif
    CORRADE_COMPARE_AS(*actual,
        Containers::arrayView(list),
        TestSuite::Compare::Container);
}

/* Checks if we are reading it as binary (CR+LF is not converted to LF),
   nothing after \0 gets lost, and invalid UTF-8 chars (over 0x80) also cause
   no issues */
constexpr const char Data[]{'\xCA', '\xFE', '\xBA', '\xBE', '\x0D', '\x0A', '\x00', '\xDE', '\xAD', '\xBE', '\xEF'};

void PathTest::size() {
    /* Existing file, containing the above data */
    CORRADE_COMPARE(Path::size(Path::join(_testDir, "file")),
        Containers::arraySize(Data));
}

void PathTest::sizeEmpty() {
    Containers::String empty = Path::join(_testDir, "dir/dummy");
    CORRADE_VERIFY(Path::exists(empty));
    CORRADE_COMPARE(Path::size(empty), 0);
}

void PathTest::sizeNonSeekable() {
    /* macOS or BSD doesn't have /proc */
    #if defined(__unix__) && !defined(CORRADE_TARGET_EMSCRIPTEN) && \
        !defined(__FreeBSD__) && !defined(__OpenBSD__) && !defined(__bsdi__) && \
        !defined(__NetBSD__) && !defined(__DragonFly__)
    std::ostringstream out;
    Error redirectError{&out};
    /* /proc/loadavg works on Android Emulator but not on a real device;
       /proc/zoneinfo works everywhere */
    CORRADE_VERIFY(!Path::size("/proc/zoneinfo"));
    CORRADE_COMPARE(out.str(), "Utility::Path::size(): /proc/zoneinfo is not seekable\n");
    #else
    CORRADE_SKIP("Not implemented on this platform.");
    #endif
}

void PathTest::sizeEarlyEof() {
    #ifdef __linux__
    Containers::StringView file = "/sys/devices/system/cpu/cpu0/cpufreq/scaling_governor";
    if(!Path::exists(file))
        CORRADE_SKIP(file << "doesn't exist, can't test");
    Containers::Optional<std::size_t> size = Path::size(file);
    Containers::Optional<Containers::Array<char>> data = Path::read(file);
    CORRADE_VERIFY(size);
    CORRADE_VERIFY(data);
    CORRADE_COMPARE_AS(*size, data->size(), TestSuite::Compare::Greater);
    #else
    CORRADE_SKIP("Not sure how to test on this platform.");
    #endif
}

void PathTest::sizeDirectory() {
    #if defined(CORRADE_TARGET_IOS) && defined(CORRADE_TESTSUITE_TARGET_XCTEST)
    if(!std::getenv("SIMULATOR_UDID"))
        CORRADE_SKIP("iOS (in a simulator) has no idea about file types, can't test.");
    #endif

    std::ostringstream out;
    Error redirectError{&out};
    CORRADE_COMPARE(Path::size(_testDir), Containers::NullOpt);

    /* On Windows the opening itself fails, on Unix we have an explicit check.
       On other systems no idea, so let's say we expect the same message as on
       Unix. */
    #ifdef CORRADE_TARGET_WINDOWS
    CORRADE_COMPARE_AS(out.str(),
        formatString("Utility::Path::size(): can't open {}: error 13 (", _testDir),
        TestSuite::Compare::StringHasPrefix);
    #else
    CORRADE_COMPARE(out.str(), formatString("Utility::Path::size(): {} is a directory\n", _testDir));
    #endif
}

void PathTest::sizeNonexistent() {
    std::ostringstream out;
    Error redirectError{&out};
    CORRADE_COMPARE(Path::size("nonexistent"), Containers::NullOpt);
    #ifdef CORRADE_TARGET_EMSCRIPTEN
    /* Emscripten uses a different errno for "No such file or directory" */
    CORRADE_COMPARE_AS(out.str(),
        "Utility::Path::size(): can't open nonexistent: error 44 (",
        TestSuite::Compare::StringHasPrefix);
    #else
    CORRADE_COMPARE_AS(out.str(),
        "Utility::Path::size(): can't open nonexistent: error 2 (",
        TestSuite::Compare::StringHasPrefix);
    #endif
}

void PathTest::sizeNonNullTerminated() {
    /* Existing file, containing the above data */
    CORRADE_COMPARE(Path::size(Path::join(_testDir, "fileX").exceptSuffix(1)),
        Containers::arraySize(Data));
}

void PathTest::sizeUtf8() {
    CORRADE_COMPARE(Path::size(Path::join(_testDirUtf8, "hýždě")),
        Containers::arraySize(Data));
}

void PathTest::read() {
    Containers::Optional<Containers::Array<char>> data = Path::read(Path::join(_testDir, "file"));
    CORRADE_VERIFY(data);
    CORRADE_COMPARE_AS(*data,
        Containers::arrayView(Data),
        TestSuite::Compare::Container);
}

void PathTest::readString() {
    Containers::Optional<Containers::String> string = Path::readString(Path::join(_testDir, "file"));
    CORRADE_VERIFY(string);
    /* Data is a char array *without* a null terminator, so take the whole
       size */
    CORRADE_COMPARE(*string, (Containers::StringView{Data, Containers::arraySize(Data)}));
    /* There should be a null terminator at the end. With assertions enabled
       the String constructor checks for this on its own, but let's double
       check here as well. */
    CORRADE_COMPARE(*string->end(), '\0');
}

void PathTest::readEmpty() {
    Containers::String empty = Path::join(_testDir, "dir/dummy");
    CORRADE_VERIFY(Path::exists(empty));

    /* The optional is set, but the array is empty */
    Containers::Optional<Containers::Array<char>> data = Path::read(empty);
    CORRADE_VERIFY(data);
    CORRADE_VERIFY(data->isEmpty());
}

void PathTest::readEmptyString() {
    Containers::String empty = Path::join(_testDir, "dir/dummy");
    CORRADE_VERIFY(Path::exists(empty));

    /* The optional is set, but the string is empty */
    Containers::Optional<Containers::String> string = Path::readString(empty);
    CORRADE_VERIFY(string);
    CORRADE_VERIFY(!*string);
    /* There should be a null terminator at the end. With assertions enabled
       the String constructor checks for this on its own, but let's double
       check here as well. */
    CORRADE_COMPARE(*string->end(), '\0');
}

void PathTest::readNonSeekable() {
    /* macOS or BSD doesn't have /proc */
    #if defined(__unix__) && !defined(CORRADE_TARGET_EMSCRIPTEN) && \
        !defined(__FreeBSD__) && !defined(__OpenBSD__) && !defined(__bsdi__) && \
        !defined(__NetBSD__) && !defined(__DragonFly__)
    /** @todo Test more thoroughly than this */
    /* /proc/loadavg works on Android Emulator but not on a real device;
       /proc/zoneinfo works everywhere */
    Containers::Optional<Containers::Array<char>> data = Path::read("/proc/zoneinfo");
    CORRADE_VERIFY(data);
    CORRADE_VERIFY(!data->isEmpty());
    /* The array is growable */
    CORRADE_VERIFY(data->deleter());
    /* But it shouldn't contain null bytes anywhere (which would point to
       issues with growing the array) */
    CORRADE_COMPARE_AS(Containers::StringView{*data}, "\0"_s, TestSuite::Compare::StringNotContains);
    #else
    CORRADE_SKIP("Not implemented on this platform.");
    #endif
}

void PathTest::readNonSeekableString() {
    /* macOS or BSD doesn't have /proc */
    #if defined(__unix__) && !defined(CORRADE_TARGET_EMSCRIPTEN) && \
        !defined(__FreeBSD__) && !defined(__OpenBSD__) && !defined(__bsdi__) && \
        !defined(__NetBSD__) && !defined(__DragonFly__)
    /** @todo Test more thoroughly than this */
    /* /proc/loadavg works on Android Emulator but not on a real device;
       /proc/zoneinfo works everywhere */
    Containers::Optional<Containers::String> string = Path::readString("/proc/zoneinfo");
    CORRADE_VERIFY(string);
    CORRADE_VERIFY(*string);
    /* The array is growable and the deleter should be preserved in the string
       as well */
    CORRADE_VERIFY(string->deleter());
    /* There should be a null terminator at the end. With assertions enabled
       the String constructor checks for this on its own, but let's double
       check here as well. */
    CORRADE_COMPARE(*string->end(), '\0');
    /* But it shouldn't contain null bytes anywhere else (which would point to
       issues with adding the extra null terminator byte) */
    CORRADE_COMPARE_AS(*string, "\0"_s, TestSuite::Compare::StringNotContains);
    #else
    CORRADE_SKIP("Not implemented on this platform.");
    #endif
}

void PathTest::readEarlyEof() {
    #ifdef __linux__
    if(!Path::exists("/sys/devices/system/cpu/cpu0/cpufreq/scaling_governor"))
        CORRADE_SKIP("/sys/devices/system/cpu/cpu0/cpufreq/scaling_governor doesn't exist, can't test");
    Containers::Optional<Containers::Array<char>> data = Path::read("/sys/devices/system/cpu/cpu0/cpufreq/scaling_governor");
    CORRADE_VERIFY(data);
    CORRADE_VERIFY(!data->isEmpty());
    #else
    CORRADE_SKIP("Not sure how to test on this platform.");
    #endif
}

void PathTest::readEarlyEofString() {
    #ifdef __linux__
    if(!Path::exists("/sys/devices/system/cpu/cpu0/cpufreq/scaling_governor"))
        CORRADE_SKIP("/sys/devices/system/cpu/cpu0/cpufreq/scaling_governor doesn't exist, can't test");
    Containers::Optional<Containers::String> string = Path::readString("/sys/devices/system/cpu/cpu0/cpufreq/scaling_governor");
    CORRADE_VERIFY(string);
    CORRADE_VERIFY(*string);
    /* There should be a null terminator at the end in this case as well, not
       just when the array is growable or when we know the correct size in
       advance. With assertions enabled the String constructor checks for this
       on its own, but let's double check here as well. */
    CORRADE_COMPARE(*string->end(), '\0');
    #else
    CORRADE_SKIP("Not sure how to test on this platform.");
    #endif
}

void PathTest::readDirectory() {
    #if defined(CORRADE_TARGET_IOS) && defined(CORRADE_TESTSUITE_TARGET_XCTEST)
    if(!std::getenv("SIMULATOR_UDID"))
        CORRADE_SKIP("iOS (in a simulator) has no idea about file types, can't test.");
    #endif

    std::ostringstream out;
    Error redirectError{&out};
    CORRADE_VERIFY(!Path::read(_testDir));

    /* On Windows the opening itself fails, on Unix we have an explicit check.
       On other systems no idea, so let's say we expect the same message as on
       Unix. */
    #ifdef CORRADE_TARGET_WINDOWS
    CORRADE_COMPARE_AS(out.str(),
        formatString("Utility::Path::read(): can't open {}: error 13 (", _testDir),
        TestSuite::Compare::StringHasPrefix);
    #else
    CORRADE_COMPARE(out.str(), formatString(
        "Utility::Path::read(): {} is a directory\n", _testDir));
    #endif

    /* String variant should return a NullOpt as well, not testing the message
       as it should be coming from read() */
    CORRADE_VERIFY(!Path::readString(_testDir));
}

void PathTest::readNonexistent() {
    std::ostringstream out;
    Error redirectError{&out};
    CORRADE_VERIFY(!Path::read("nonexistent"));
    #ifdef CORRADE_TARGET_EMSCRIPTEN
    /* Emscripten uses a different errno for "No such file or directory" */
    CORRADE_COMPARE_AS(out.str(),
        "Utility::Path::read(): can't open nonexistent: error 44 (",
        TestSuite::Compare::StringHasPrefix);
    #else
    CORRADE_COMPARE_AS(out.str(),
        "Utility::Path::read(): can't open nonexistent: error 2 (",
        TestSuite::Compare::StringHasPrefix);
    #endif

    /* String variant should return a NullOpt as well, not testing the message
       as it should be coming from read() */
    CORRADE_VERIFY(!Path::readString("nonexistent"));
}

void PathTest::readNonNullTerminated() {
    Containers::Optional<Containers::Array<char>> data = Path::read(Path::join(_testDir, "fileX").exceptSuffix(1));
    CORRADE_VERIFY(data);
    CORRADE_COMPARE_AS(*data,
        Containers::arrayView(Data),
        TestSuite::Compare::Container);
}

void PathTest::readUtf8() {
    Containers::Optional<Containers::Array<char>> data = Path::read(Path::join(_testDirUtf8, "hýždě"));
    CORRADE_VERIFY(data);
    CORRADE_COMPARE_AS(*data,
        Containers::arrayView(Data),
        TestSuite::Compare::Container);
}

void PathTest::write() {
    CORRADE_VERIFY(Path::make(_writeTestDir));
    Containers::String file = Path::join(_writeTestDir, "file");

    if(Path::exists(file))
        CORRADE_VERIFY(Path::remove(file));
    CORRADE_VERIFY(Path::write(file, Containers::arrayView(Data)));
    CORRADE_COMPARE_AS(file, Path::join(_testDir, "file"),
        TestSuite::Compare::File);
}

CORRADE_HAS_TYPE(CanWriteBeCalledWith, decltype(Path::write("", std::declval<T>())));

void PathTest::writeDisabledOverloads() {
    /* I thought I could use std::is_invocable (or a C++11 backport of it) to
       easily test this. Boy I was wrong, that API is absolutely useless, while
       the CORRADE_HAS_TYPE() macro is the best thing ever.

       Same as appendDisabledOverloads(), please keep consistent */
    CORRADE_VERIFY(CanWriteBeCalledWith<Containers::ArrayView<char>>::value);
    CORRADE_VERIFY(CanWriteBeCalledWith<Containers::ArrayView<const char>>::value);
    CORRADE_VERIFY(CanWriteBeCalledWith<Containers::StringView>::value);
    CORRADE_VERIFY(CanWriteBeCalledWith<Containers::Array<char>>::value);
    CORRADE_VERIFY(CanWriteBeCalledWith<Containers::String>::value);
    /* This is desirable for writing empty files */
    CORRADE_VERIFY(CanWriteBeCalledWith<std::nullptr_t>::value);
    /* This is desirable for writing arbitrary arrays */
    CORRADE_VERIFY(CanWriteBeCalledWith<const float(&)[4]>::value);
    CORRADE_VERIFY(CanWriteBeCalledWith<float(&)[4]>::value);

    /* None of these should work as it may lead to the null terminator getting
       written by accident */
    CORRADE_VERIFY(!CanWriteBeCalledWith<const char*>::value);
    CORRADE_VERIFY(!CanWriteBeCalledWith<char*>::value);
    CORRADE_VERIFY(!CanWriteBeCalledWith<const char(&)[4]>::value);
    CORRADE_VERIFY(!CanWriteBeCalledWith<char(&)[4]>::value);
    CORRADE_VERIFY(!CanWriteBeCalledWith<void*>::value);
}

void PathTest::writeEmpty() {
    CORRADE_VERIFY(Path::make(_writeTestDir));
    Containers::String file = Path::join(_writeTestDir, "empty");

    if(Path::exists(file))
        CORRADE_VERIFY(Path::remove(file));
    CORRADE_VERIFY(Path::write(file, nullptr));
    CORRADE_COMPARE_AS(file, "",
        TestSuite::Compare::FileToString);
}

void PathTest::writeDirectory() {
    CORRADE_VERIFY(Path::make(_writeTestDir));

    std::ostringstream out;
    Error redirectError{&out};
    CORRADE_VERIFY(!Path::write(_writeTestDir, nullptr));
    /* Fortunately enough, opening the directory for writing fails already,
       without having to do anything special */
    #ifdef CORRADE_TARGET_WINDOWS
    /* Windows APIs use "Permission denied" instead of "Is a directory" */
    CORRADE_COMPARE_AS(out.str(),
        formatString("Utility::Path::write(): can't open {}: error 13 (", _writeTestDir),
        TestSuite::Compare::StringHasPrefix);
    #elif defined(CORRADE_TARGET_EMSCRIPTEN)
    /* Emscripten uses a different errno for "Is a directory" */
    CORRADE_COMPARE_AS(out.str(),
        formatString("Utility::Path::write(): can't open {}: error 31 (", _writeTestDir),
        TestSuite::Compare::StringHasPrefix);
    #else
    CORRADE_COMPARE_AS(out.str(),
        formatString("Utility::Path::write(): can't open {}: error 21 (", _writeTestDir),
        TestSuite::Compare::StringHasPrefix);
    #endif
}

void PathTest::writeNoPermission() {
    #ifdef CORRADE_TARGET_EMSCRIPTEN
    CORRADE_SKIP("Everything is writable under Emscripten.");
    #else
    #ifdef CORRADE_TARGET_APPLE
    /* Assuming there's no real possibility to run as root on Apple */
    Containers::StringView filename = "/var/root/writtenFile";
    #elif defined(CORRADE_TARGET_ANDROID)
    /* Same here, would need a rooted device */
    Containers::StringView filename = "/data/local/writtenFile";
    #elif defined(CORRADE_TARGET_UNIX)
    Containers::StringView filename = "/root/writtenFile";
    if(Path::homeDirectory() == "/root")
        CORRADE_SKIP("Running under root, can't test for permissions.");
    #elif defined(CORRADE_TARGET_WINDOWS)
    /* Only the TrustedInstaller system user is supposed to have access into
       WindowsApps */
    Containers::StringView filename = "C:/Program Files/WindowsApps/writtenFile";
    #elif defined(CORRADE_TARGET_EMSCRIPTEN)
    #else
    Containers::StringView filename;
    CORRADE_SKIP("Not sure how to test on this system.");
    #endif

    std::ostringstream out;
    Error redirectError{&out};
    CORRADE_VERIFY(!Path::write(filename, nullptr));
    CORRADE_COMPARE_AS(out.str(),
        formatString("Utility::Path::write(): can't open {}: error 13 (", filename),
        TestSuite::Compare::StringHasPrefix);
    #endif
}

void PathTest::writeNonNullTerminated() {
    CORRADE_VERIFY(Path::make(_writeTestDir));

    Containers::String file = Path::join(_writeTestDir, "file");

    if(Path::exists(file))
        CORRADE_VERIFY(Path::remove(file));
    CORRADE_VERIFY(Path::write(Path::join(_writeTestDir, "fileX").exceptSuffix(1), Containers::arrayView(Data)));
    CORRADE_COMPARE_AS(file, Path::join(_testDir, "file"),
        TestSuite::Compare::File);
}

void PathTest::writeUtf8() {
    CORRADE_VERIFY(Path::make(_writeTestDir));

    Containers::String file = Path::join(_writeTestDir, "hýždě");
    if(Path::exists(file))
        CORRADE_VERIFY(Path::remove(file));

    CORRADE_VERIFY(Path::write(file, Containers::arrayView(Data)));
    CORRADE_COMPARE_AS(file, Path::join(_testDirUtf8, "hýždě"),
        TestSuite::Compare::File);
}

void PathTest::append() {
    CORRADE_VERIFY(Path::make(_writeTestDir));

    constexpr const char expected[]{'h', 'e', 'l', 'l', 'o', '\xCA', '\xFE', '\xBA', '\xBE', '\x0D', '\x0A', '\x00', '\xDE', '\xAD', '\xBE', '\xEF'};

    Containers::String file = Path::join(_writeTestDir, "file");
    if(Path::exists(file))
        CORRADE_VERIFY(Path::remove(file));

    CORRADE_VERIFY(Path::write(file, "hello"_s));
    CORRADE_VERIFY(Path::append(file, Containers::arrayView(Data)));
    CORRADE_COMPARE_AS(file, (std::string{expected, Containers::arraySize(expected)}),
        TestSuite::Compare::FileToString);
}

CORRADE_HAS_TYPE(CanAppendBeCalledWith, decltype(Path::append("", std::declval<T>())));

void PathTest::appendDisabledOverloads() {
    /* Same as writeDisabledOverloads(), please keep consistent */
    CORRADE_VERIFY(CanAppendBeCalledWith<Containers::ArrayView<char>>::value);
    CORRADE_VERIFY(CanAppendBeCalledWith<Containers::ArrayView<const char>>::value);
    CORRADE_VERIFY(CanAppendBeCalledWith<Containers::StringView>::value);
    CORRADE_VERIFY(CanAppendBeCalledWith<Containers::Array<char>>::value);
    CORRADE_VERIFY(CanAppendBeCalledWith<Containers::String>::value);
    /* This is desirable for writing empty files */
    CORRADE_VERIFY(CanAppendBeCalledWith<std::nullptr_t>::value);
    /* This is desirable for writing arbitrary arrays */
    CORRADE_VERIFY(CanAppendBeCalledWith<const float(&)[4]>::value);
    CORRADE_VERIFY(CanAppendBeCalledWith<float(&)[4]>::value);

    /* None of these should work as it may lead to the null terminator getting
       written by accident */
    CORRADE_VERIFY(!CanAppendBeCalledWith<const char*>::value);
    CORRADE_VERIFY(!CanAppendBeCalledWith<char*>::value);
    CORRADE_VERIFY(!CanAppendBeCalledWith<const char(&)[4]>::value);
    CORRADE_VERIFY(!CanAppendBeCalledWith<char(&)[4]>::value);
    CORRADE_VERIFY(!CanAppendBeCalledWith<void*>::value);
}

void PathTest::appendToNonexistent() {
    CORRADE_VERIFY(Path::make(_writeTestDir));

    Containers::String file = Path::join(_writeTestDir, "empty");
    if(Path::exists(file))
        CORRADE_VERIFY(Path::remove(file));

    CORRADE_VERIFY(Path::append(file, "hello"_s));
    CORRADE_COMPARE_AS(file, "hello",
        TestSuite::Compare::FileToString);
}

void PathTest::appendEmpty() {
    CORRADE_VERIFY(Path::make(_writeTestDir));

    Containers::String file = Path::join(_writeTestDir, "empty");
    if(Path::exists(file))
        CORRADE_VERIFY(Path::remove(file));

    CORRADE_VERIFY(Path::write(file, "hello"_s));
    CORRADE_VERIFY(Path::append(file, nullptr));
    CORRADE_COMPARE_AS(file, "hello",
        TestSuite::Compare::FileToString);
}

void PathTest::appendDirectory() {
    CORRADE_VERIFY(Path::make(_writeTestDir));

    std::ostringstream out;
    Error redirectError{&out};
    CORRADE_VERIFY(!Path::append(_writeTestDir, nullptr));
    /* Fortunately enough, opening the directory for writing fails already,
       without having to do anything special */
    #ifdef CORRADE_TARGET_WINDOWS
    /* Windows APIs use "Permission denied" instead of "Is a directory" */
    CORRADE_COMPARE_AS(out.str(),
        formatString("Utility::Path::append(): can't open {}: error 13 (", _writeTestDir),
        TestSuite::Compare::StringHasPrefix);
    #elif defined(CORRADE_TARGET_EMSCRIPTEN)
    /* Emscripten uses a different errno for "Is a directory" */
    CORRADE_COMPARE_AS(out.str(),
        formatString("Utility::Path::append(): can't open {}: error 31 (", _writeTestDir),
        TestSuite::Compare::StringHasPrefix);
    #else
    CORRADE_COMPARE_AS(out.str(),
        formatString("Utility::Path::append(): can't open {}: error 21 (", _writeTestDir),
        TestSuite::Compare::StringHasPrefix);
    #endif
}

void PathTest::appendNoPermission() {
    #ifdef CORRADE_TARGET_EMSCRIPTEN
    CORRADE_SKIP("Everything is writable under Emscripten.");
    #else
    #ifdef CORRADE_TARGET_APPLE
    /* Assuming there's no real possibility to run as root on Apple */
    Containers::StringView filename = "/var/root/writtenFile";
    #elif defined(CORRADE_TARGET_ANDROID)
    /* Same here, would need a rooted device */
    Containers::StringView filename = "/data/local/writtenFile";
    #elif defined(CORRADE_TARGET_UNIX)
    Containers::StringView filename = "/root/writtenFile";
    if(Path::homeDirectory() == "/root")
        CORRADE_SKIP("Running under root, can't test for permissions.");
    #elif defined(CORRADE_TARGET_WINDOWS)
    /* Only the TrustedInstaller system user is supposed to have access into
       WindowsApps */
    Containers::StringView filename = "C:/Program Files/WindowsApps/writtenFile";
    #else
    Containers::StringView filename;
    CORRADE_SKIP("Not sure how to test on this system.");
    #endif

    std::ostringstream out;
    Error redirectError{&out};
    CORRADE_VERIFY(!Path::append(filename, nullptr));
    CORRADE_COMPARE_AS(out.str(),
        formatString("Utility::Path::append(): can't open {}: error 13 (", filename),
        TestSuite::Compare::StringHasPrefix);
    #endif
}

void PathTest::appendNonNullTerminated() {
    CORRADE_VERIFY(Path::make(_writeTestDir));

    constexpr const char expected[]{'h', 'e', 'l', 'l', 'o', '\xCA', '\xFE', '\xBA', '\xBE', '\x0D', '\x0A', '\x00', '\xDE', '\xAD', '\xBE', '\xEF'};

    Containers::String file = Path::join(_writeTestDir, "file");
    if(Path::exists(file))
        CORRADE_VERIFY(Path::remove(file));

    CORRADE_VERIFY(Path::write(file, "hello"_s));
    CORRADE_VERIFY(Path::append(Path::join(_writeTestDir, "fileX").exceptSuffix(1), Containers::arrayView(Data)));
    CORRADE_COMPARE_AS(file, (std::string{expected, Containers::arraySize(expected)}),
        TestSuite::Compare::FileToString);
}

void PathTest::appendUtf8() {
    CORRADE_VERIFY(Path::make(_writeTestDir));

    Containers::String file = Path::join(_writeTestDir, "hýždě");
    if(Path::exists(file))
        CORRADE_VERIFY(Path::remove(file));

    CORRADE_VERIFY(Path::append(file, Containers::arrayView(Data)));
    CORRADE_COMPARE_AS(file, Path::join(_testDirUtf8, "hýždě"),
        TestSuite::Compare::File);
}

void PathTest::prepareFileToCopy() {
    Path::make(_writeTestDir);

    if(Path::exists(Path::join(_writeTestDir, "copySource.dat")))
        return;

    Containers::Array<int> data{NoInit, 150000};
    for(std::size_t i = 0; i != data.size(); ++i) data[i] = 4678641 + i;

    Path::write(Path::join(_writeTestDir, "copySource.dat"), data);
}

void PathTest::copy() {
    Containers::String source = Path::join(_writeTestDir, "copySource.dat");
    CORRADE_VERIFY(Path::exists(source));

    Containers::String destination = Path::join(_writeTestDir, "copyDestination.dat");
    if(Path::exists(destination))
        CORRADE_VERIFY(Path::remove(destination));

    CORRADE_VERIFY(Path::copy(source, destination));
    CORRADE_COMPARE_AS(source, destination, TestSuite::Compare::File);
}

void PathTest::copyEmpty() {
    Containers::String source = Path::join(_testDir, "dir/dummy");
    CORRADE_VERIFY(Path::exists(source));

    Containers::String destination = Path::join(_writeTestDir, "empty");
    if(Path::exists(destination))
        CORRADE_VERIFY(Path::remove(destination));

    CORRADE_VERIFY(Path::copy(source, destination));
    CORRADE_COMPARE_AS(destination, "",
        TestSuite::Compare::FileToString);
}

void PathTest::copyDirectory() {
    #if defined(CORRADE_TARGET_IOS) && defined(CORRADE_TESTSUITE_TARGET_XCTEST)
    if(!std::getenv("SIMULATOR_UDID"))
        CORRADE_SKIP("iOS (in a simulator) has no idea about file types, can't test.");
    #endif

    Containers::String source = Path::join(_writeTestDir, "copySource.dat");
    Containers::String destination = Path::join(_writeTestDir, "copyDestination.dat");
    CORRADE_VERIFY(Path::exists(source));

    {
        std::ostringstream out;
        Error redirectError{&out};
        CORRADE_VERIFY(!Path::copy(source, _writeTestDir));
        /* Opening a directory for writing fails on its own, so there's no need
           for a special message */
        #ifdef CORRADE_TARGET_WINDOWS
        /* Windows APIs use "Permission denied" instead of "Is a directory" */
        CORRADE_COMPARE_AS(out.str(),
            formatString("Utility::Path::copy(): can't open {} for writing: error 13 (", _writeTestDir),
            TestSuite::Compare::StringHasPrefix);
        #elif defined(CORRADE_TARGET_EMSCRIPTEN)
        /* Emscripten uses a different errno for "Is a directory" */
        CORRADE_COMPARE_AS(out.str(),
            formatString("Utility::Path::copy(): can't open {} for writing: error 31 (", _writeTestDir),
            TestSuite::Compare::StringHasPrefix);
        #else
        CORRADE_COMPARE_AS(out.str(),
            formatString("Utility::Path::copy(): can't open {} for writing: error 21 (", _writeTestDir),
            TestSuite::Compare::StringHasPrefix);
        #endif
    } {
        std::ostringstream out;
        Error redirectError{&out};
        CORRADE_VERIFY(!Path::copy(_writeTestDir, destination));

        /* On Windows the opening itself fails, on Unix we have an explicit
           check. On other systems no idea, so let's say we expect the same
           message as on Unix. */
        #ifdef CORRADE_TARGET_WINDOWS
        CORRADE_COMPARE_AS(out.str(),
            formatString("Utility::Path::copy(): can't open {} for reading: error 13 (", _writeTestDir),
            TestSuite::Compare::StringHasPrefix);
        #else
        CORRADE_COMPARE(out.str(), formatString("Utility::Path::copy(): can't read from {} which is a directory\n", _writeTestDir));
        #endif
    }
}

void PathTest::copyReadNonexistent() {
    CORRADE_VERIFY(Path::make(_writeTestDir));

    std::ostringstream out;
    Error redirectError{&out};
    CORRADE_VERIFY(!Path::copy("nonexistent", Path::join(_writeTestDir, "empty")));
    #ifdef CORRADE_TARGET_EMSCRIPTEN
    /* Emscripten uses a different errno for "No such file or directory" */
    CORRADE_COMPARE_AS(out.str(),
        "Utility::Path::copy(): can't open nonexistent for reading: error 44 (",
        TestSuite::Compare::StringHasPrefix);
    #else
    CORRADE_COMPARE_AS(out.str(),
        "Utility::Path::copy(): can't open nonexistent for reading: error 2 (",
        TestSuite::Compare::StringHasPrefix);
    #endif
}

void PathTest::copyWriteNoPermission() {
    #ifdef CORRADE_TARGET_EMSCRIPTEN
    CORRADE_SKIP("Everything is writable under Emscripten.");
    #else
    #ifdef CORRADE_TARGET_APPLE
    /* Assuming there's no real possibility to run as root on Apple */
    Containers::StringView filename = "/var/root/writtenFile";
    #elif defined(CORRADE_TARGET_ANDROID)
    /* Same here, would need a rooted device */
    Containers::StringView filename = "/data/local/writtenFile";
    #elif defined(CORRADE_TARGET_UNIX)
    Containers::StringView filename = "/root/writtenFile";
    if(Path::homeDirectory() == "/root")
        CORRADE_SKIP("Running under root, can't test for permissions.");
    #elif defined(CORRADE_TARGET_WINDOWS)
    /* Only the TrustedInstaller system user is supposed to have access into
       WindowsApps */
    Containers::StringView filename = "C:/Program Files/WindowsApps/writtenFile";
    #else
    Containers::StringView filename;
    CORRADE_SKIP("Not sure how to test on this system.");
    #endif

    std::ostringstream out;
    Error redirectError{&out};
    CORRADE_VERIFY(!Path::copy(Path::join(_testDir, "dir/dummy"), filename));
    CORRADE_COMPARE_AS(out.str(),
        formatString("Utility::Path::copy(): can't open {} for writing: error 13 (", filename),
        TestSuite::Compare::StringHasPrefix);
    #endif
}

void PathTest::copyNonNullTerminated() {
    CORRADE_VERIFY(Path::make(_writeTestDir));

    Containers::String source = Path::join(_writeTestDir, "copySource.dat");
    CORRADE_VERIFY(Path::exists(source));

    Containers::String destination = Path::join(_writeTestDir, "copyDestination.dat");
    if(Path::exists(destination))
        CORRADE_VERIFY(Path::remove(destination));

    CORRADE_VERIFY(Path::copy(Path::join(_writeTestDir, "copySource.datX").exceptSuffix(1), Path::join(_writeTestDir, "copyDestination.datX").exceptSuffix(1)));
    CORRADE_COMPARE_AS(source, destination, TestSuite::Compare::File);
}

void PathTest::copyUtf8() {
    CORRADE_VERIFY(Path::make(_writeTestDir));

    Containers::String output = Path::join(_writeTestDir, "hýždě");
    if(Path::exists(output))
        CORRADE_VERIFY(Path::remove(output));

    CORRADE_VERIFY(Path::copy(Path::join(_testDirUtf8, "hýždě"), output));
    CORRADE_COMPARE_AS(Path::join(_writeTestDir, "hýždě"),
        Path::join(_testDirUtf8, "hýždě"),
        TestSuite::Compare::File);
}

#ifndef CORRADE_TARGET_EMSCRIPTEN
void PathTest::prepareFileToBenchmarkCopy() {
    Path::make(_writeTestDir);

    if(Path::exists(Path::join(_writeTestDir, "copyBenchmarkSource.dat")))
        return;

    /* Append a megabyte file 50 times to create a 50MB file */
    Containers::Array<int> data{ValueInit, 256*1024};
    for(std::size_t i = 0; i != data.size(); ++i) data[i] = 4678641 + i;

    for(std::size_t i = 0; i != 50; ++i)
        Path::append(Path::join(_writeTestDir, "copyBenchmarkSource.dat"), data);
}

void PathTest::copy100MReadWrite() {
    Containers::String input = Path::join(_writeTestDir, "copyBenchmarkSource.dat");
    CORRADE_VERIFY(Path::exists(input));

    Containers::String output = Path::join(_writeTestDir, "copyDestination.dat");
    if(Path::exists(output))
        CORRADE_VERIFY(Path::remove(output));

    CORRADE_BENCHMARK(1)
        Path::write(output, *Path::read(input));
}

void PathTest::copy100MReadWriteString() {
    Containers::String input = Path::join(_writeTestDir, "copyBenchmarkSource.dat");
    CORRADE_VERIFY(Path::exists(input));

    Containers::String output = Path::join(_writeTestDir, "copyDestination.dat");
    if(Path::exists(output))
        CORRADE_VERIFY(Path::remove(output));

    CORRADE_BENCHMARK(1)
        Path::write(output, *Path::readString(input));
}

void PathTest::copy100MCopy() {
    Containers::String input = Path::join(_writeTestDir, "copyBenchmarkSource.dat");
    CORRADE_VERIFY(Path::exists(input));

    Containers::String output = Path::join(_writeTestDir, "copyDestination.dat");
    if(Path::exists(output))
        CORRADE_VERIFY(Path::remove(output));

    CORRADE_BENCHMARK(1)
        Path::copy(input, output);
}

#if defined(DOXYGEN_GENERATING_OUTPUT) || defined(CORRADE_TARGET_UNIX) || (defined(CORRADE_TARGET_WINDOWS) && !defined(CORRADE_TARGET_WINDOWS_RT))
void PathTest::copy100MMap() {
    Containers::String input = Path::join(_writeTestDir, "copyBenchmarkSource.dat");
    CORRADE_VERIFY(Path::exists(input));

    Containers::String output = Path::join(_writeTestDir, "copyDestination.dat");
    if(Path::exists(output))
        CORRADE_VERIFY(Path::remove(output));

    CORRADE_BENCHMARK(1)
        Path::write(output, *Path::mapRead(input));
}
#endif
#endif

void PathTest::map() {
    #if defined(CORRADE_TARGET_UNIX) || (defined(CORRADE_TARGET_WINDOWS) && !defined(CORRADE_TARGET_WINDOWS_RT))
    CORRADE_VERIFY(Path::make(_writeTestDir));
    Containers::String file = Path::join(_writeTestDir, "mappedFile");
    CORRADE_VERIFY(Path::write(file, Containers::arrayView(Data)));

    {
        Containers::Optional<Containers::Array<char, Path::MapDeleter>> mappedFile = Path::map(file);
        CORRADE_VERIFY(mappedFile);
        CORRADE_COMPARE_AS(*mappedFile,
            Containers::arrayView(Data),
            TestSuite::Compare::Container);

        /* Write a thing there */
        (*mappedFile)[2] = '\xCA';
        (*mappedFile)[3] = '\xFE';

        /* Implicit unmap */
    }

    /* Here --------------------vv--vv- the file should be changed */
    CORRADE_COMPARE_AS(file,
        (std::string{"\xCA\xFE\xCA\xFE\x0D\x0A\x00\xDE\xAD\xBE\xEF", 11}),
        TestSuite::Compare::FileToString);
    #else
    CORRADE_SKIP("Not implemented on this platform.");
    #endif
}

void PathTest::mapEmpty() {
    #if defined(CORRADE_TARGET_UNIX) || (defined(CORRADE_TARGET_WINDOWS) && !defined(CORRADE_TARGET_WINDOWS_RT))
    CORRADE_VERIFY(Path::make(_writeTestDir));
    Containers::String file = Path::join(_writeTestDir, "mappedEmpty");
    CORRADE_VERIFY(Path::write(file, nullptr));

    {
        Containers::Optional<Containers::Array<char, Path::MapDeleter>> mappedFile = Path::map(file);
        CORRADE_VERIFY(mappedFile);
        CORRADE_COMPARE_AS(*mappedFile,
            Containers::arrayView<char>({}),
            TestSuite::Compare::Container);

        /* Implicit unmap */
    }

    /* The file should be still as empty as before */
    CORRADE_COMPARE_AS(file,
        "",
        TestSuite::Compare::FileToString);
    #else
    CORRADE_SKIP("Not implemented on this platform.");
    #endif
}

void PathTest::mapDirectory() {
    #if defined(CORRADE_TARGET_UNIX) || (defined(CORRADE_TARGET_WINDOWS) && !defined(CORRADE_TARGET_WINDOWS_RT))
    CORRADE_VERIFY(Path::make(_writeTestDir));

    std::ostringstream out;
    Error redirectError{&out};
    CORRADE_VERIFY(!Path::map(_writeTestDir));
    /* Opening a directory for R+W fails on its own, so there's no need for a
       special message */
    #ifdef CORRADE_TARGET_WINDOWS
    /* Windows APIs fill GetLastError() instead of errno, leading to a
       different code ("Access denied") */
    CORRADE_COMPARE_AS(out.str(),
        formatString("Utility::Path::map(): can't open {}: error 5 (", _writeTestDir),
        TestSuite::Compare::StringHasPrefix);
    #else
    CORRADE_COMPARE_AS(out.str(),
        formatString("Utility::Path::map(): can't open {}: error 21 (", _writeTestDir),
        TestSuite::Compare::StringHasPrefix);
    #endif
    #else
    CORRADE_SKIP("Not implemented on this platform.");
    #endif
}

void PathTest::mapNonexistent() {
    #if defined(CORRADE_TARGET_UNIX) || (defined(CORRADE_TARGET_WINDOWS) && !defined(CORRADE_TARGET_WINDOWS_RT))
    std::ostringstream out;
    Error redirectError{&out};
    CORRADE_VERIFY(!Path::map("nonexistent"));
    CORRADE_COMPARE_AS(out.str(),
        "Utility::Path::map(): can't open nonexistent: error 2 (",
        TestSuite::Compare::StringHasPrefix);
    #else
    CORRADE_SKIP("Not implemented on this platform.");
    #endif
}

void PathTest::mapNonNullTerminated() {
    #if defined(CORRADE_TARGET_UNIX) || (defined(CORRADE_TARGET_WINDOWS) && !defined(CORRADE_TARGET_WINDOWS_RT))
    CORRADE_VERIFY(Path::make(_writeTestDir));

    Containers::String file = Path::join(_writeTestDir, "mappedFile");
    CORRADE_VERIFY(Path::write(file, Containers::arrayView(Data)));

    /* Enough to just verify that the file got read, no need to test writing as
       well */
    Containers::Optional<Containers::Array<char, Path::MapDeleter>> mappedFile = Path::map(Path::join(_writeTestDir, "mappedFileX").exceptSuffix(1));
    CORRADE_VERIFY(mappedFile);
    CORRADE_COMPARE_AS(*mappedFile,
        Containers::arrayView(Data),
        TestSuite::Compare::Container);
    #else
    CORRADE_SKIP("Not implemented on this platform.");
    #endif
}

void PathTest::mapUtf8() {
    #if defined(CORRADE_TARGET_UNIX) || (defined(CORRADE_TARGET_WINDOWS) && !defined(CORRADE_TARGET_WINDOWS_RT))
    Containers::Optional<Containers::Array<char, Path::MapDeleter>> mappedFile = Path::map(Path::join(_testDirUtf8, "hýždě"));
    CORRADE_VERIFY(mappedFile);
    CORRADE_COMPARE_AS(*mappedFile,
        Containers::arrayView(Data),
        TestSuite::Compare::Container);
    #else
    CORRADE_SKIP("Not implemented on this platform.");
    #endif
}

void PathTest::mapRead() {
    #if defined(CORRADE_TARGET_UNIX) || (defined(CORRADE_TARGET_WINDOWS) && !defined(CORRADE_TARGET_WINDOWS_RT))
    Containers::Optional<Containers::Array<const char, Path::MapDeleter>> mappedFile = Path::mapRead(Path::join(_testDir, "file"));
    CORRADE_VERIFY(mappedFile);
    CORRADE_COMPARE_AS(*mappedFile,
        Containers::arrayView(Data),
        TestSuite::Compare::Container);
    #else
    CORRADE_SKIP("Not implemented on this platform.");
    #endif
}

void PathTest::mapReadEmpty() {
    #if defined(CORRADE_TARGET_UNIX) || (defined(CORRADE_TARGET_WINDOWS) && !defined(CORRADE_TARGET_WINDOWS_RT))
    Containers::Optional<Containers::Array<const char, Path::MapDeleter>> mappedFile = Path::mapRead(Path::join(_testDir, "dir/dummy"));
    CORRADE_VERIFY(mappedFile);
    CORRADE_COMPARE_AS(*mappedFile,
        Containers::arrayView<char>({}),
        TestSuite::Compare::Container);
    #else
    CORRADE_SKIP("Not implemented on this platform.");
    #endif
}

void PathTest::mapReadDirectory() {
    #if defined(CORRADE_TARGET_IOS) && defined(CORRADE_TESTSUITE_TARGET_XCTEST)
    if(!std::getenv("SIMULATOR_UDID"))
        CORRADE_SKIP("iOS (in a simulator) has no idea about file types, can't test.");
    #endif

    #if defined(CORRADE_TARGET_UNIX) || (defined(CORRADE_TARGET_WINDOWS) && !defined(CORRADE_TARGET_WINDOWS_RT))
    CORRADE_VERIFY(Path::make(_writeTestDir));

    std::ostringstream out;
    Error redirectError{&out};
    CORRADE_VERIFY(!Path::mapRead(_writeTestDir));
    /* On Windows the opening itself fails, on Unix we have an explicit check */
    #ifdef CORRADE_TARGET_WINDOWS
    /* Windows APIs fill GetLastError() instead of errno, leading to a
       different code ("Access denied") */
    CORRADE_COMPARE_AS(out.str(),
        formatString("Utility::Path::mapRead(): can't open {}: error 5 (", _writeTestDir),
        TestSuite::Compare::StringHasPrefix);
    #else
    CORRADE_COMPARE(out.str(), formatString("Utility::Path::mapRead(): {} is a directory\n", _writeTestDir));
    #endif
    #else
    CORRADE_SKIP("Not implemented on this platform.");
    #endif
}

void PathTest::mapReadNonexistent() {
    #if defined(CORRADE_TARGET_UNIX) || (defined(CORRADE_TARGET_WINDOWS) && !defined(CORRADE_TARGET_WINDOWS_RT))
    std::ostringstream out;
    Error redirectError{&out};
    CORRADE_VERIFY(!Path::mapRead("nonexistent"));
    CORRADE_COMPARE_AS(out.str(),
        "Utility::Path::mapRead(): can't open nonexistent: error 2 (",
        TestSuite::Compare::StringHasPrefix);
    #else
    CORRADE_SKIP("Not implemented on this platform.");
    #endif
}

void PathTest::mapReadNonNullTerminated() {
    #if defined(CORRADE_TARGET_UNIX) || (defined(CORRADE_TARGET_WINDOWS) && !defined(CORRADE_TARGET_WINDOWS_RT))
    Containers::Optional<Containers::Array<const char, Path::MapDeleter>> mappedFile = Path::mapRead(Path::join(_testDir, "fileX").exceptSuffix(1));
    CORRADE_VERIFY(mappedFile);
    CORRADE_COMPARE_AS(*mappedFile,
        Containers::arrayView(Data),
        TestSuite::Compare::Container);
    #else
    CORRADE_SKIP("Not implemented on this platform.");
    #endif
}

void PathTest::mapReadUtf8() {
    #if defined(CORRADE_TARGET_UNIX) || (defined(CORRADE_TARGET_WINDOWS) && !defined(CORRADE_TARGET_WINDOWS_RT))
    Containers::Optional<Containers::Array<const char, Path::MapDeleter>> mappedFile = Path::mapRead(Path::join(_testDirUtf8, "hýždě"));
    CORRADE_VERIFY(mappedFile);
    CORRADE_COMPARE_AS(*mappedFile,
        Containers::arrayView(Data),
        TestSuite::Compare::Container);
    #else
    CORRADE_SKIP("Not implemented on this platform.");
    #endif
}

void PathTest::mapWrite() {
    #if defined(CORRADE_TARGET_UNIX) || (defined(CORRADE_TARGET_WINDOWS) && !defined(CORRADE_TARGET_WINDOWS_RT))
    CORRADE_VERIFY(Path::make(_writeTestDir));

    {
        Containers::Optional<Containers::Array<char, Path::MapDeleter>> mappedFile = Path::mapWrite(Path::join(_writeTestDir, "mappedWriteFile"), Containers::arraySize(Data));
        CORRADE_VERIFY(mappedFile);
        CORRADE_COMPARE(mappedFile->size(), Containers::arraySize(Data));
        Utility::copy(Data, *mappedFile);
    }
    CORRADE_COMPARE_AS(Path::join(_writeTestDir, "mappedWriteFile"),
        (std::string{Data, Containers::arraySize(Data)}),
        TestSuite::Compare::FileToString);
    #else
    CORRADE_SKIP("Not implemented on this platform.");
    #endif
}

void PathTest::mapWriteEmpty() {
    #if defined(CORRADE_TARGET_UNIX) || (defined(CORRADE_TARGET_WINDOWS) && !defined(CORRADE_TARGET_WINDOWS_RT))
    CORRADE_VERIFY(Path::make(_writeTestDir));

    {
        Containers::Optional<Containers::Array<char, Path::MapDeleter>> mappedFile = Path::mapWrite(Path::join(_writeTestDir, "mappedWriteEmpty"), 0);
        CORRADE_VERIFY(mappedFile);
        CORRADE_COMPARE(mappedFile->size(), 0);
    }
    CORRADE_COMPARE_AS(Path::join(_writeTestDir, "mappedWriteEmpty"),
        "",
        TestSuite::Compare::FileToString);
    #else
    CORRADE_SKIP("Not implemented on this platform.");
    #endif
}

void PathTest::mapWriteDirectory() {
    #if defined(CORRADE_TARGET_UNIX) || (defined(CORRADE_TARGET_WINDOWS) && !defined(CORRADE_TARGET_WINDOWS_RT))
    CORRADE_VERIFY(Path::make(_writeTestDir));

    std::ostringstream out;
    Error redirectError{&out};
    CORRADE_VERIFY(!Path::mapWrite(_writeTestDir, 64));
    /* Opening a directory for R+W fails on its own, so there's no need for a
       special message */
    #ifdef CORRADE_TARGET_WINDOWS
    /* Windows APIs fill GetLastError() instead of errno, leading to a
       different code ("Access denied") */
    CORRADE_COMPARE_AS(out.str(),
        formatString("Utility::Path::mapWrite(): can't open {}: error 5 (", _writeTestDir),
        TestSuite::Compare::StringHasPrefix);
    #else
    CORRADE_COMPARE_AS(out.str(),
        formatString("Utility::Path::mapWrite(): can't open {}: error 21 (", _writeTestDir),
        TestSuite::Compare::StringHasPrefix);
    #endif
    #else
    CORRADE_SKIP("Not implemented on this platform.");
    #endif
}

void PathTest::mapWriteNoPermission() {
    #if defined(CORRADE_TARGET_UNIX) || (defined(CORRADE_TARGET_WINDOWS) && !defined(CORRADE_TARGET_WINDOWS_RT))
    #ifdef CORRADE_TARGET_APPLE
    /* Assuming there's no real possibility to run as root on Apple */
    Containers::StringView filename = "/var/root/mappedFile";
    #elif defined(CORRADE_TARGET_ANDROID)
    /* Same here, would need a rooted device */
    Containers::StringView filename = "/data/local/mappedFile";
    #elif defined(CORRADE_TARGET_UNIX)
    Containers::StringView filename = "/root/mappedFile";
    if(Path::homeDirectory() == "/root")
        CORRADE_SKIP("Running under root, can't test for permissions.");
    #elif defined(CORRADE_TARGET_WINDOWS)
    /* Only the TrustedInstaller system user is supposed to have access into
       WindowsApps */
    Containers::StringView filename = "C:/Program Files/WindowsApps/mappedFile";
    #else
    #error
    #endif

    std::ostringstream out;
    Error redirectError{&out};
    CORRADE_VERIFY(!Path::mapWrite(filename, 64));
    #ifdef CORRADE_TARGET_WINDOWS
    /* Windows APIs fill GetLastError() instead of errno, leading to a
       different code */
    CORRADE_COMPARE_AS(out.str(),
        formatString("Utility::Path::mapWrite(): can't open {}: error 5 (", filename),
        TestSuite::Compare::StringHasPrefix);
    #else
    CORRADE_COMPARE_AS(out.str(),
        formatString("Utility::Path::mapWrite(): can't open {}: error 13 (", filename),
        TestSuite::Compare::StringHasPrefix);
    #endif
    #else
    CORRADE_SKIP("Not implemented on this platform.");
    #endif
}

void PathTest::mapWriteNonNullTerminated() {
    #if defined(CORRADE_TARGET_UNIX) || (defined(CORRADE_TARGET_WINDOWS) && !defined(CORRADE_TARGET_WINDOWS_RT))
    CORRADE_VERIFY(Path::make(_writeTestDir));

    {
        Containers::Optional<Containers::Array<char, Path::MapDeleter>> mappedFile = Path::mapWrite(Path::join(_writeTestDir, "mappedWriteFileX").exceptSuffix(1), Containers::arraySize(Data));
        CORRADE_VERIFY(mappedFile);
        CORRADE_COMPARE(mappedFile->size(), Containers::arraySize(Data));
        Utility::copy(Data, *mappedFile);
    }
    CORRADE_COMPARE_AS(Path::join(_writeTestDir, "mappedWriteFile"),
        (std::string{Data, Containers::arraySize(Data)}),
        TestSuite::Compare::FileToString);
    #else
    CORRADE_SKIP("Not implemented on this platform.");
    #endif
}

void PathTest::mapWriteUtf8() {
    #if defined(CORRADE_TARGET_UNIX) || (defined(CORRADE_TARGET_WINDOWS) && !defined(CORRADE_TARGET_WINDOWS_RT))
    CORRADE_VERIFY(Path::make(_writeTestDir));

    {
        Containers::Optional<Containers::Array<char, Path::MapDeleter>> mappedFile = Path::mapWrite(Path::join(_writeTestDir, "hýždě chlípníka"), Containers::arraySize(Data));
        CORRADE_VERIFY(mappedFile);
        CORRADE_COMPARE(mappedFile->size(), Containers::arraySize(Data));
        Utility::copy(Data, *mappedFile);
    }
    CORRADE_COMPARE_AS(Path::join(_writeTestDir, "hýždě chlípníka"),
        (std::string{Data, Containers::arraySize(Data)}),
        TestSuite::Compare::FileToString);
    #else
    CORRADE_SKIP("Not implemented on this platform.");
    #endif
}

}}}}

CORRADE_TEST_MAIN(Corrade::Utility::Test::PathTest)
