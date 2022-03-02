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
#include <vector>

/* There's no better way to disable file deprecation warnings */
#define _CORRADE_NO_DEPRECATED_DIRECTORY

#include "Corrade/Containers/Array.h"
#include "Corrade/Containers/Optional.h"
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
#include "Corrade/Utility/Directory.h"
#include "Corrade/Utility/FormatStl.h"

#include "configure.h"

#ifdef CORRADE_TARGET_UNIX
/* Needed for chdir() in currentInvalid() */
#include <unistd.h>
#endif

#ifdef CORRADE_TARGET_APPLE
#include "Corrade/Utility/System.h" /* isSandboxed() */
#endif

namespace Corrade { namespace Utility { namespace Test { namespace {

struct DirectoryTest: TestSuite::Tester {
    explicit DirectoryTest();

    void fromNativeSeparators();
    void toNativeSeparators();
    void path();
    void filename();
    void splitExtension();
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
    void existsUtf8();

    void isDirectory();
    void isDirectorySymlink();
    void isDirectoryNoPermission();
    void isDirectoryUtf8();

    void removeFile();
    void removeDirectory();
    void removeFileNonexistent();
    void removeDirectoryNonEmpty();
    void removeFileUtf8();
    void removeDirectoryUtf8();

    void moveFile();
    void moveDirectory();
    void moveSourceNonexistent();
    void moveDestinationNoPermission();
    void moveUtf8();

    void mkpath();
    void mkpathDotDotDot();
    void mkpathNoPermission();
    void mkpathUtf8();

    void current();
    void currentNonexistent();
    void currentUtf8();

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

    void home();
    void homeInvalid();
    void homeUtf8();

    void configurationDir();
    void configurationDirInvalid();
    void configurationDirUtf8();

    void tmp();
    void tmpInvalid();
    void tmpUtf8();

    void list();
    void listSkipDirectories();
    void listSkipDirectoriesSymlinks();
    void listSkipFiles();
    void listSkipFilesSymlinks();
    void listSkipSpecial();
    void listSkipSpecialSymlink();
    void listSkipDotAndDotDot();
    void listSort();
    void listSortPrecedence();
    void listNonexistent();
    void listUtf8();

    void fileSize();
    void fileSizeEmpty();
    void fileSizeNonSeekable();
    void fileSizeEarlyEof();
    void fileSizeDirectory();
    void fileSizeNonexistent();
    void fileSizeUtf8();

    void read();
    void readEmpty();
    void readNonSeekable();
    void readEarlyEof();
    void readDirectory();
    void readNonexistent();
    void readUtf8();

    void write();
    void writeEmpty();
    void writeDirectory();
    void writeNoPermission();
    void writeUtf8();

    void append();
    void appendToNonexistent();
    void appendEmpty();
    void appendDirectory();
    void appendNoPermission();
    void appendUtf8();

    void prepareFileToCopy();
    void copy();
    void copyEmpty();
    void copyDirectory();
    void copyReadNonexistent();
    void copyWriteNoPermission();
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
    void mapUtf8();

    void mapRead();
    void mapReadEmpty();
    void mapReadDirectory();
    void mapReadNonexistent();
    void mapReadUtf8();

    void mapWrite();
    void mapWriteEmpty();
    void mapWriteDirectory();
    void mapWriteNoPermission();
    void mapWriteUtf8();

    std::string _testDir,
        _testDirSymlink,
        _testDirUtf8,
        _writeTestDir;
};

CORRADE_IGNORE_DEPRECATED_PUSH

DirectoryTest::DirectoryTest() {
    addTests({&DirectoryTest::fromNativeSeparators,
              &DirectoryTest::toNativeSeparators,
              &DirectoryTest::path,
              &DirectoryTest::filename,
              &DirectoryTest::splitExtension,
              &DirectoryTest::join,
              #ifdef CORRADE_TARGET_WINDOWS
              &DirectoryTest::joinWindows,
              #endif
              &DirectoryTest::joinMultiple,
              &DirectoryTest::joinMultipleAbsolute,
              &DirectoryTest::joinMultipleOneEmpty,
              &DirectoryTest::joinMultipleJustOne,
              &DirectoryTest::joinMultipleNone,

              &DirectoryTest::exists,
              &DirectoryTest::existsNoPermission,
              &DirectoryTest::existsUtf8,

              &DirectoryTest::isDirectory,
              &DirectoryTest::isDirectorySymlink,
              &DirectoryTest::isDirectoryNoPermission,
              &DirectoryTest::isDirectoryUtf8,

              &DirectoryTest::removeFile,
              &DirectoryTest::removeDirectory,
              &DirectoryTest::removeFileNonexistent,
              &DirectoryTest::removeDirectoryNonEmpty,
              &DirectoryTest::removeFileUtf8,
              &DirectoryTest::removeDirectoryUtf8,

              &DirectoryTest::moveFile,
              &DirectoryTest::moveDirectory,
              &DirectoryTest::moveSourceNonexistent,
              &DirectoryTest::moveDestinationNoPermission,
              &DirectoryTest::moveUtf8,

              &DirectoryTest::mkpath,
              &DirectoryTest::mkpathDotDotDot,
              &DirectoryTest::mkpathNoPermission,
              &DirectoryTest::mkpathUtf8,

              &DirectoryTest::current,
              &DirectoryTest::currentNonexistent,
              &DirectoryTest::currentUtf8,

              #ifndef CORRADE_BUILD_STATIC
              &DirectoryTest::libraryLocation,
              #else
              &DirectoryTest::libraryLocationStatic,
              #endif
              &DirectoryTest::libraryLocationNull,
              &DirectoryTest::libraryLocationInvalid,
              &DirectoryTest::libraryLocationUtf8,

              &DirectoryTest::executableLocation,
              &DirectoryTest::executableLocationInvalid,
              &DirectoryTest::executableLocationUtf8,

              &DirectoryTest::home,
              &DirectoryTest::homeInvalid,
              &DirectoryTest::homeUtf8,

              &DirectoryTest::configurationDir,
              &DirectoryTest::configurationDirInvalid,
              &DirectoryTest::configurationDirUtf8,

              &DirectoryTest::tmp,
              &DirectoryTest::tmpInvalid,
              &DirectoryTest::tmpUtf8,

              &DirectoryTest::list,
              &DirectoryTest::listSkipDirectories,
              &DirectoryTest::listSkipDirectoriesSymlinks,
              &DirectoryTest::listSkipFiles,
              &DirectoryTest::listSkipFilesSymlinks,
              &DirectoryTest::listSkipSpecial,
              &DirectoryTest::listSkipSpecialSymlink,
              &DirectoryTest::listSkipDotAndDotDot,
              &DirectoryTest::listSort,
              &DirectoryTest::listSortPrecedence,
              &DirectoryTest::listNonexistent,
              &DirectoryTest::listUtf8,

              &DirectoryTest::fileSize,
              &DirectoryTest::fileSizeEmpty,
              &DirectoryTest::fileSizeNonSeekable,
              &DirectoryTest::fileSizeEarlyEof,
              &DirectoryTest::fileSizeDirectory,
              &DirectoryTest::fileSizeNonexistent,
              &DirectoryTest::fileSizeUtf8,

              &DirectoryTest::read,
              &DirectoryTest::readEmpty,
              &DirectoryTest::readNonSeekable,
              &DirectoryTest::readEarlyEof,
              &DirectoryTest::readDirectory,
              &DirectoryTest::readNonexistent,
              &DirectoryTest::readUtf8,

              &DirectoryTest::write,
              &DirectoryTest::writeEmpty,
              &DirectoryTest::writeDirectory,
              &DirectoryTest::writeNoPermission,
              &DirectoryTest::writeUtf8,

              &DirectoryTest::append,
              &DirectoryTest::appendToNonexistent,
              &DirectoryTest::appendEmpty,
              &DirectoryTest::appendDirectory,
              &DirectoryTest::appendNoPermission,
              &DirectoryTest::appendUtf8});

    addTests({&DirectoryTest::copy},
             &DirectoryTest::prepareFileToCopy,
             &DirectoryTest::prepareFileToCopy);

    addTests({&DirectoryTest::copyEmpty,
              &DirectoryTest::copyDirectory,
              &DirectoryTest::copyReadNonexistent,
              &DirectoryTest::copyWriteNoPermission,
              &DirectoryTest::copyUtf8});

    #ifndef CORRADE_TARGET_EMSCRIPTEN
    addBenchmarks({
        &DirectoryTest::copy100MReadWrite,
        &DirectoryTest::copy100MReadWriteString,
        &DirectoryTest::copy100MCopy,
        #if defined(DOXYGEN_GENERATING_OUTPUT) || defined(CORRADE_TARGET_UNIX) || (defined(CORRADE_TARGET_WINDOWS) && !defined(CORRADE_TARGET_WINDOWS_RT))
        &DirectoryTest::copy100MMap
        #endif
        }, 5,
        &DirectoryTest::prepareFileToBenchmarkCopy,
        &DirectoryTest::prepareFileToBenchmarkCopy);
    #endif

    addTests({&DirectoryTest::map,
              &DirectoryTest::mapEmpty,
              &DirectoryTest::mapDirectory,
              &DirectoryTest::mapNonexistent,
              &DirectoryTest::mapUtf8,

              &DirectoryTest::mapRead,
              &DirectoryTest::mapReadEmpty,
              &DirectoryTest::mapReadDirectory,
              &DirectoryTest::mapReadNonexistent,
              &DirectoryTest::mapReadUtf8,

              &DirectoryTest::mapWrite,
              &DirectoryTest::mapWriteEmpty,
              &DirectoryTest::mapWriteDirectory,
              &DirectoryTest::mapWriteNoPermission,
              &DirectoryTest::mapWriteUtf8});

    #ifdef CORRADE_TARGET_APPLE
    if(System::isSandboxed()
        #if defined(CORRADE_TARGET_IOS) && defined(CORRADE_TESTSUITE_TARGET_XCTEST)
        /** @todo Fix this once I persuade CMake to run XCTest tests properly */
        && std::getenv("SIMULATOR_UDID")
        #endif
    ) {
        _testDir = Directory::join(Directory::path(Directory::executableLocation()), "PathTestFiles");
        _testDirSymlink = Directory::join(Directory::path(Directory::executableLocation()), "PathTestFilesSymlink");
        _testDirUtf8 = Directory::join(Directory::path(Directory::executableLocation()), "PathTestFilesUtf8");
        _writeTestDir = Directory::join(Directory::home(), "Library/Caches");
    } else
    #endif
    {
        _testDir = PATH_TEST_DIR;
        _testDirSymlink = PATH_TEST_DIR_SYMLINK;
        _testDirUtf8 = PATH_TEST_DIR_UTF8;
        _writeTestDir = DIRECTORY_WRITE_TEST_DIR;
    }

    /* Delete the file for copy tests to avoid using a stale version */
    if(Directory::exists(Directory::join(_writeTestDir, "copySource.dat")))
        Directory::rm(Directory::join(_writeTestDir, "copySource.dat"));
    if(Directory::exists(Directory::join(_writeTestDir, "copyBenchmarkSource.dat")))
        Directory::rm(Directory::join(_writeTestDir, "copyBenchmarkSource.dat"));
}

void DirectoryTest::fromNativeSeparators() {
    const std::string nativeSeparators = Directory::fromNativeSeparators("put\\ that/somewhere\\ else");
    #ifdef CORRADE_TARGET_WINDOWS
    CORRADE_COMPARE(nativeSeparators, "put/ that/somewhere/ else");
    #else
    CORRADE_COMPARE(nativeSeparators, "put\\ that/somewhere\\ else");
    #endif
}

void DirectoryTest::toNativeSeparators() {
    const std::string nativeSeparators = Directory::toNativeSeparators("this\\is a weird/system\\right");
    #ifdef CORRADE_TARGET_WINDOWS
    CORRADE_COMPARE(nativeSeparators, "this\\is a weird\\system\\right");
    #else
    CORRADE_COMPARE(nativeSeparators, "this\\is a weird/system\\right");
    #endif
}

void DirectoryTest::path() {
    /* No path */
    CORRADE_COMPARE(Directory::path("foo.txt"), "");

    /* No filename */
    CORRADE_COMPARE(Directory::path(".corrade/configuration/"), ".corrade/configuration");

    /* Common case */
    CORRADE_COMPARE(Directory::path("package/map.conf"), "package");
}

void DirectoryTest::filename() {
    /* Path only */
    CORRADE_COMPARE(Directory::filename("foo/bar/"), "");

    /* File only */
    CORRADE_COMPARE(Directory::filename("file.txt"), "file.txt");

    /* Common case */
    CORRADE_COMPARE(Directory::filename("foo/bar/map.conf"), "map.conf");
}

void DirectoryTest::splitExtension() {
    /* In case you're not sure about the behavior, cross-check with Python's
       os.path.splitext(). */

    /* Empty */
    CORRADE_COMPARE(Directory::splitExtension(""), std::make_pair("", ""));

    /* Usual case */
    CORRADE_COMPARE(Directory::splitExtension("file.txt"), std::make_pair("file", ".txt"));

    /* Double extension */
    CORRADE_COMPARE(Directory::splitExtension("file.tar.gz"), std::make_pair("file.tar", ".gz"));

    /* No extension */
    CORRADE_COMPARE(Directory::splitExtension("/etc/passwd"), std::make_pair("/etc/passwd", ""));

    /* Dot not a part of the file */
    CORRADE_COMPARE(Directory::splitExtension("/etc/rc.conf/file"), std::make_pair("/etc/rc.conf/file", ""));

    /* Dot at the end */
    CORRADE_COMPARE(Directory::splitExtension("/home/no."), std::make_pair("/home/no", "."));

    /* Dotfile, prefixed or not */
    CORRADE_COMPARE(Directory::splitExtension("/home/mosra/.bashrc"), std::make_pair("/home/mosra/.bashrc", ""));
    CORRADE_COMPARE(Directory::splitExtension(".bashrc"), std::make_pair(".bashrc", ""));

    /* One level up, prefixed or not */
    CORRADE_COMPARE(Directory::splitExtension("/home/mosra/Code/.."), std::make_pair("/home/mosra/Code/..", ""));
    CORRADE_COMPARE(Directory::splitExtension(".."), std::make_pair("..", ""));

    /* This directory */
    CORRADE_COMPARE(Directory::splitExtension("/home/mosra/."), std::make_pair("/home/mosra/.", ""));
    CORRADE_COMPARE(Directory::splitExtension("."), std::make_pair(".", ""));
}

void DirectoryTest::join() {
    /* Empty path */
    CORRADE_COMPARE(Directory::join("", "/foo.txt"), "/foo.txt");

    /* Empty all */
    CORRADE_COMPARE(Directory::join("", ""), "");

    /* Absolute filename */
    CORRADE_COMPARE(Directory::join("/foo/bar", "/file.txt"), "/file.txt");

    /* Trailing slash */
    CORRADE_COMPARE(Directory::join("/foo/bar/", "file.txt"), "/foo/bar/file.txt");

    /* Common case */
    CORRADE_COMPARE(Directory::join("/foo/bar", "file.txt"), "/foo/bar/file.txt");
}

#ifdef CORRADE_TARGET_WINDOWS
void DirectoryTest::joinWindows() {
    /* Drive letter */
    CORRADE_COMPARE(Directory::join("/foo/bar", "X:/path/file.txt"), "X:/path/file.txt");
}
#endif

void DirectoryTest::joinMultiple() {
    CORRADE_COMPARE(Directory::join({"foo", "bar", "file.txt"}), "foo/bar/file.txt");
}

void DirectoryTest::joinMultipleAbsolute() {
    CORRADE_COMPARE(Directory::join({"foo", "/bar", "file.txt"}), "/bar/file.txt");
}

void DirectoryTest::joinMultipleOneEmpty() {
    CORRADE_COMPARE(Directory::join({"foo", "", "file.txt"}), "foo/file.txt");
}

void DirectoryTest::joinMultipleJustOne() {
    CORRADE_COMPARE(Directory::join({"file.txt"}), "file.txt");
}

void DirectoryTest::joinMultipleNone() {
    CORRADE_COMPARE(Directory::join({}), "");
}

void DirectoryTest::exists() {
    /* File */
    CORRADE_VERIFY(Directory::exists(Directory::join(_testDir, "file")));

    /* Directory */
    CORRADE_VERIFY(Directory::exists(_testDir));

    /* Nonexistent file */
    CORRADE_VERIFY(!Directory::exists(Directory::join(_testDir, "nonexistentFile")));

    /* Current directory, empty */
    CORRADE_VERIFY(Directory::exists("."));
    CORRADE_VERIFY(!Directory::exists(""));
}

void DirectoryTest::existsNoPermission() {
    #ifdef CORRADE_TARGET_EMSCRIPTEN
    CORRADE_SKIP("Everything is accessible under Emscripten.");
    #else
    /* macOS or BSD doesn't have /proc */
    #if defined(__unix__) && !defined(CORRADE_TARGET_EMSCRIPTEN) && \
        !defined(__FreeBSD__) && !defined(__OpenBSD__) && !defined(__bsdi__) && \
        !defined(__NetBSD__) && !defined(__DragonFly__)
    /* Assuming there's no real possibility to run as root on Apple so this
       checks only other Unix systems */
    if(Directory::home() == "/root")
        CORRADE_SKIP("Running under root, can't test for permissions.");

    /* /proc/self/fd exists, PID 1 is the "root" process and should always
       exist -- thus exists() reporting true, but any attempts to read it
       should fail. */
    CORRADE_VERIFY(Directory::exists("/proc/self/mem"));
    CORRADE_VERIFY(Directory::exists("/proc/1"));
    CORRADE_VERIFY(Directory::exists("/proc/1/mem"));
    /* Just to be sure we're not giving back bullshit -- a random file in the
       same inaccessible directory should fail, opening that inacessible file
       should fail */
    CORRADE_VERIFY(!Directory::exists("/proc/1/nonexistent"));
    CORRADE_VERIFY(!Directory::fileSize("/proc/1/mem"));
    #else
    /** @todo find something that actually exists, to test the same case as on
        Unix. No idea what in C:/Program Files/WindowsApps could be guaranteed
        to exist. */
    CORRADE_SKIP("Not sure how to test this.");
    #endif
    #endif
}

void DirectoryTest::existsUtf8() {
    CORRADE_VERIFY(Directory::exists(Directory::join(_testDirUtf8, "hýždě")));
}

void DirectoryTest::isDirectory() {
    {
        #if defined(CORRADE_TARGET_IOS) && defined(CORRADE_TESTSUITE_TARGET_XCTEST)
        CORRADE_EXPECT_FAIL_IF(!std::getenv("SIMULATOR_UDID"),
            "iOS (in a simulator) thinks all paths are files.");
        #endif
        CORRADE_VERIFY(Directory::isDirectory(Directory::join(_testDir, "dir")));
    }

    CORRADE_VERIFY(!Directory::isDirectory(Directory::join(_testDir, "file")));

    /* Nonexistent file */
    CORRADE_VERIFY(!Directory::isDirectory(Directory::join(_testDir, "nonexistentFile")));
}

void DirectoryTest::isDirectorySymlink() {
    CORRADE_VERIFY(Directory::exists(Directory::join(_testDirSymlink, "file-symlink")));
    CORRADE_VERIFY(!Directory::isDirectory(Directory::join(_testDirSymlink, "file-symlink")));

    CORRADE_VERIFY(Directory::exists(Directory::join(_testDirSymlink, "dir-symlink")));
    {
        #if !defined(CORRADE_TARGET_UNIX) && !defined(CORRADE_TARGET_EMSCRIPTEN)
        /* Possible on Windows too, but there we'd need to first detect if the
           Git clone has the symlinks preserved */
        CORRADE_EXPECT_FAIL("Symlink support is implemented on Unix systems and Emscripten only.");
        #endif
        #if defined(CORRADE_TARGET_IOS) && defined(CORRADE_TESTSUITE_TARGET_XCTEST)
        CORRADE_EXPECT_FAIL_IF(!std::getenv("SIMULATOR_UDID"),
            "iOS (in a simulator) thinks all paths are files.");
        #endif
        CORRADE_VERIFY(Directory::isDirectory(Directory::join(_testDirSymlink, "dir-symlink")));
    }
}

void DirectoryTest::isDirectoryNoPermission() {
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
    if(Directory::home() == "/root")
        CORRADE_SKIP("Running under root, can't test for permissions.");

    /* /proc/self/fd exists and is a directory, PID 1 is the "root" process and
       should always exist, but we shouldn't be able to peek into it */
    CORRADE_VERIFY(Directory::isDirectory("/proc/self/fd"));
    CORRADE_VERIFY(Directory::isDirectory("/proc/1"));
    CORRADE_VERIFY(Directory::isDirectory("/proc/1/fd"));
    /* Just to be sure we're not giving back bullshit -- a random file in the
       same inaccessible directory should fail, opening that inacessible file
       should fail */
    CORRADE_VERIFY(!Directory::exists("/proc/1/nonexistent"));
    CORRADE_VERIFY(!Directory::fileSize("/proc/1/fd"));
    #elif defined(CORRADE_TARGET_WINDOWS)
    /* This file doesn't exist, but the whole contents of the WindowsApps
       directory is accessible only by the TrustedInstaller system user so this
       should fail */
    CORRADE_VERIFY(Directory::isDirectory("C:/Program Files/WindowsApps"));
    CORRADE_VERIFY(!Directory::isDirectory("C:/Program Files/WindowsApps/someDir"));
    #else
    CORRADE_SKIP("Not sure how to test this.");
    #endif
    #endif
}

void DirectoryTest::isDirectoryUtf8() {
    {
        #if defined(CORRADE_TARGET_IOS) && defined(CORRADE_TESTSUITE_TARGET_XCTEST)
        CORRADE_EXPECT_FAIL_IF(!std::getenv("SIMULATOR_UDID"),
            "iOS (in a simulator) thinks all paths are files.");
        #endif
        CORRADE_VERIFY(Directory::isDirectory(Directory::join(_testDirUtf8, "šňůra")));
    }
    CORRADE_VERIFY(!Directory::isDirectory(Directory::join(_testDirUtf8, "hýždě")));
}

void DirectoryTest::removeFile() {
    std::string file = Directory::join(_writeTestDir, "file.txt");
    CORRADE_VERIFY(Directory::mkpath(_writeTestDir));
    CORRADE_VERIFY(Directory::writeString(file, "a"));
    CORRADE_VERIFY(Directory::exists(file));
    CORRADE_VERIFY(Directory::rm(file));
    CORRADE_VERIFY(!Directory::exists(file));
}

void DirectoryTest::removeDirectory() {
    std::string directory = Directory::join(_writeTestDir, "directory");
    CORRADE_VERIFY(Directory::mkpath(directory));
    CORRADE_VERIFY(Directory::exists(directory));
    CORRADE_VERIFY(Directory::rm(directory));
    CORRADE_VERIFY(!Directory::exists(directory));
}

void DirectoryTest::removeFileNonexistent() {
    CORRADE_VERIFY(!Directory::exists("nonexistent"));

    std::ostringstream out;
    Error redirectError{&out};
    CORRADE_VERIFY(!Directory::rm("nonexistent"));
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

void DirectoryTest::removeDirectoryNonEmpty() {
    std::string directory = Directory::join(_writeTestDir, "nonEmptyDirectory");
    CORRADE_VERIFY(Directory::mkpath(directory));
    CORRADE_VERIFY(Directory::writeString(Directory::join(directory, "file.txt"), "a"));

    std::ostringstream out;
    Error redirectError{&out};
    CORRADE_VERIFY(!Directory::rm(directory));
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

void DirectoryTest::removeFileUtf8() {
    std::string file = Directory::join(_writeTestDir, "hýždě.txt");
    CORRADE_VERIFY(Directory::mkpath(_writeTestDir));
    CORRADE_VERIFY(Directory::writeString(file, "a"));
    CORRADE_VERIFY(Directory::exists(file));
    CORRADE_VERIFY(Directory::rm(file));
    CORRADE_VERIFY(!Directory::exists(file));
}

void DirectoryTest::removeDirectoryUtf8() {
    std::string directory = Directory::join(_writeTestDir, "složka");
    CORRADE_VERIFY(Directory::mkpath(directory));
    CORRADE_VERIFY(Directory::exists(directory));
    CORRADE_VERIFY(Directory::rm(directory));
    CORRADE_VERIFY(!Directory::exists(directory));
}

void DirectoryTest::moveFile() {
    /* Old file */
    std::string oldFile = Directory::join(_writeTestDir, "oldFile.txt");
    CORRADE_VERIFY(Directory::writeString(oldFile, "a"));

    /* New file, remove if exists */
    std::string newFile = Directory::join(_writeTestDir, "newFile.txt");
    if(Directory::exists(newFile)) Directory::rm(newFile);

    CORRADE_VERIFY(Directory::exists(oldFile));
    CORRADE_VERIFY(!Directory::exists(newFile));
    CORRADE_VERIFY(Directory::move(oldFile, newFile));
    CORRADE_VERIFY(!Directory::exists(oldFile));
    CORRADE_VERIFY(Directory::exists(newFile));
}

void DirectoryTest::moveDirectory() {
    /* Old directory, create if not exists */
    std::string oldDirectory = Directory::join(_writeTestDir, "oldDirectory");
    if(!Directory::exists(oldDirectory))
        CORRADE_VERIFY(Directory::mkpath(oldDirectory));

    /* New directory, remove if exists */
    std::string newDirectory = Directory::join(_writeTestDir, "newDirectory");
    if(Directory::exists(newDirectory))
        CORRADE_VERIFY(Directory::rm(newDirectory));

    CORRADE_VERIFY(Directory::move(oldDirectory, newDirectory));
    CORRADE_VERIFY(!Directory::exists(oldDirectory));
    CORRADE_VERIFY(Directory::exists(newDirectory));
}

void DirectoryTest::moveSourceNonexistent() {
    std::string to = Directory::join(_writeTestDir, "empty");

    std::ostringstream out;
    Error redirectError{&out};
    CORRADE_VERIFY(!Directory::move("nonexistent", to));
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

void DirectoryTest::moveDestinationNoPermission() {
    #ifdef CORRADE_TARGET_EMSCRIPTEN
    CORRADE_SKIP("Everything is writable under Emscripten.");
    #else
    std::string from = Directory::join(_testDir, "dir/dummy");
    #ifdef CORRADE_TARGET_APPLE
    /* Assuming there's no real possibility to run as root on Apple */
    std::string to = "/var/root/writtenFile";
    #elif defined(CORRADE_TARGET_ANDROID)
    /* Same here, would need a rooted device */
    std::string to = "/data/local/writtenFile";
    #elif defined(CORRADE_TARGET_UNIX)
    std::string to = "/root/writtenFile";
    if(Directory::home() == "/root")
        CORRADE_SKIP("Running under root, can't test for permissions.");
    #elif defined(CORRADE_TARGET_WINDOWS)
    /* Only the TrustedInstaller system user is supposed to have access into
       WindowsApps */
    std::string to = "C:/Program Files/WindowsApps/writtenFile";
    #else
    std::string to;
    CORRADE_SKIP("Not sure how to test on this system.");
    #endif

    std::ostringstream out;
    Error redirectError{&out};
    CORRADE_VERIFY(!Directory::move(from, to));
    CORRADE_COMPARE_AS(out.str(),
        formatString("Utility::Path::move(): can't move {} to {}: error 13 (", from, to),
        TestSuite::Compare::StringHasPrefix);
    #endif
}

void DirectoryTest::moveUtf8() {
    /* Old file */
    std::string oldFile = Directory::join(_writeTestDir, "starý hýždě.txt");
    CORRADE_VERIFY(Directory::writeString(oldFile, "a"));

    /* New file, remove if exists */
    std::string newFile = Directory::join(_writeTestDir, "nový hýždě.txt");
    if(Directory::exists(newFile)) CORRADE_VERIFY(Directory::rm(newFile));

    CORRADE_VERIFY(Directory::exists(oldFile));
    CORRADE_VERIFY(!Directory::exists(newFile));
    CORRADE_VERIFY(Directory::move(oldFile, newFile));
    CORRADE_VERIFY(!Directory::exists(oldFile));
    CORRADE_VERIFY(Directory::exists(newFile));
}

void DirectoryTest::mkpath() {
    /* Existing */
    CORRADE_VERIFY(Directory::exists(_writeTestDir));
    CORRADE_VERIFY(Directory::mkpath(_writeTestDir));

    /* Leaf */
    std::string leaf = Directory::join(_writeTestDir, "leaf");
    if(Directory::exists(leaf)) CORRADE_VERIFY(Directory::rm(leaf));
    CORRADE_VERIFY(Directory::mkpath(leaf));
    CORRADE_VERIFY(Directory::exists(leaf));

    /* Path */
    std::string path = Directory::join(_writeTestDir, "path/to/new/dir");
    if(Directory::exists(path)) CORRADE_VERIFY(Directory::rm(path));
    if(Directory::exists(Directory::join(_writeTestDir, "path/to/new")))
        CORRADE_VERIFY(Directory::rm(Directory::join(_writeTestDir, "path/to/new")));
    if(Directory::exists(Directory::join(_writeTestDir, "path/to")))
        CORRADE_VERIFY(Directory::rm(Directory::join(_writeTestDir, "path/to")));
    if(Directory::exists(Directory::join(_writeTestDir, "path")))
        CORRADE_VERIFY(Directory::rm(Directory::join(_writeTestDir, "path")));

    CORRADE_VERIFY(Directory::mkpath(leaf));
    CORRADE_VERIFY(Directory::exists(leaf));

    /* Empty should be just a no-op without checking anything. Not like
       in Python, where `os.makedirs('', exist_ok=True)` stupidly fails with
        FileNotFoundError: [Errno 2] No such file or directory: '' */
    CORRADE_VERIFY(Directory::mkpath(""));
}

void DirectoryTest::mkpathDotDotDot() {
    /* Creating current directory should be a no-op because it exists */
    CORRADE_VERIFY(Directory::exists("."));
    {
        #ifdef CORRADE_TARGET_EMSCRIPTEN
        CORRADE_EXPECT_FAIL("Emscripten doesn't return EEXIST on mdkir(\".\") but fails instead.");
        #endif
        CORRADE_VERIFY(Directory::mkpath("."));
    }

    /* Parent as well */
    CORRADE_VERIFY(Directory::exists(".."));
    {
        #ifdef CORRADE_TARGET_EMSCRIPTEN
        CORRADE_EXPECT_FAIL("Emscripten doesn't return EEXIST on mdkir(\"..\") but fails instead.");
        #endif
        CORRADE_VERIFY(Directory::mkpath(".."));
    }
}

void DirectoryTest::mkpathNoPermission() {
    #ifdef CORRADE_TARGET_EMSCRIPTEN
    CORRADE_SKIP("Everything is writable under Emscripten.");
    #else
    #ifdef CORRADE_TARGET_APPLE
    /* Assuming there's no real possibility to run as root on Apple */
    std::string prefix = "/var/root";
    #elif defined(CORRADE_TARGET_ANDROID)
    /* Same here, would need a rooted device */
    std::string prefix = "/data/local";
    #elif defined(CORRADE_TARGET_UNIX)
    std::string prefix = "/root";
    if(Directory::home() == "/root")
        CORRADE_SKIP("Running under root, can't test for permissions.");
    #elif defined(CORRADE_TARGET_WINDOWS)
    /* Only the TrustedInstaller system user is supposed to have access into
       WindowsApps */
    std::string prefix = "C:/Program Files/WindowsApps";
    #else
    std::string prefix;
    CORRADE_SKIP("Not sure how to test on this system.");
    #endif

    std::ostringstream out;
    Error redirectError{&out};
    CORRADE_VERIFY(!Directory::mkpath(Directory::join(prefix, "nope/never")));
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

void DirectoryTest::mkpathUtf8() {
    std::string leaf = Directory::join(_writeTestDir, "šňůra");
    if(Directory::exists(leaf)) CORRADE_VERIFY(Directory::rm(leaf));
    CORRADE_VERIFY(Directory::mkpath(leaf));
    CORRADE_VERIFY(Directory::exists(leaf));
}

void DirectoryTest::current() {
    const std::string current = Directory::current();
    CORRADE_INFO("Current directory found as:" << current);

    /* Ensure the test is not accidentally false positive due to stale files */
    if(Directory::exists("currentDirectoryTestDir.mark"))
        CORRADE_VERIFY(Directory::rm("currentDirectoryTestDir.mark"));
    CORRADE_VERIFY(!Directory::exists("currentDirectoryTestDir.mark"));

    /* Create a file on a relative path. If current directory is correctly
       queried, it should exist there */
    CORRADE_VERIFY(Directory::write("currentDirectoryTestDir.mark",
        "hi, i'm testing Utility::Directory::current()"));
    CORRADE_VERIFY(Directory::exists(Directory::join(current, "currentDirectoryTestDir.mark")));

    /* Clean up after ourselves */
    CORRADE_VERIFY(Directory::rm("currentDirectoryTestDir.mark"));

    /* It shouldn't contain null bytes anywhere, especially not at the end */
    CORRADE_COMPARE(current.find('\0'), std::string::npos);

    /* On Windows it shouldn't contain backslashes */
    #ifdef CORRADE_TARGET_WINDOWS
    CORRADE_COMPARE(current.find('\\'), std::string::npos);
    #endif
}

void DirectoryTest::currentNonexistent() {
    #ifdef CORRADE_TARGET_UNIX
    std::string current = Directory::current();
    CORRADE_VERIFY(!current.empty());

    /* On Android the write dir is relative, so append it to current */
    std::string newCurrent = Directory::join({current, _writeTestDir, "cwd"});
    CORRADE_VERIFY(Directory::mkpath(newCurrent));

    {
        /** @todo remove (and <unistd.h>) once Directory::setCurrent() exists */
        Containers::ScopeGuard resetCurrent{&current, [](std::string* current) {
            chdir(current->data());
        }};
        chdir(newCurrent.data());
        CORRADE_COMPARE(Directory::current(), newCurrent);

        CORRADE_VERIFY(Directory::exists("."));
        CORRADE_VERIFY(Directory::rm(newCurrent));

        /* Interestingly, this doesn't fail */
        CORRADE_VERIFY(Directory::exists("."));

        std::ostringstream out;
        Error redirectError{&out};
        CORRADE_COMPARE(Directory::current(), "");
        CORRADE_COMPARE_AS(out.str(),
            "Utility::Path::currentDirectory(): error 2 (",
            TestSuite::Compare::StringHasPrefix);
    }

    /* Verify that we're back to the original directory so other tests relying
       on it keep working. Should be also done in case anything above fails. */
    CORRADE_COMPARE(Directory::current(), current);
    #else
    CORRADE_SKIP("Known to fail only on UNIX, not sure how to test elsewhere.");
    #endif
}

void DirectoryTest::currentUtf8() {
    /** @todo test once setCurrent() exists -- otherwise we'd have to implement
        UTF-16 conversion for Windows by hand here to pass to
        SetCurrentDirectory() and ... ew */
    CORRADE_SKIP("Not sure how to test this.");
}

#ifndef CORRADE_BUILD_STATIC
void DirectoryTest::libraryLocation() {
    #if defined(CORRADE_TARGET_UNIX) || (defined(CORRADE_TARGET_WINDOWS) && !defined(CORRADE_TARGET_WINDOWS_RT))
    const std::string libraryLocation = Directory::libraryLocation(&Utility::Directory::rm);

    CORRADE_INFO("Corrade::Utility library location found as:" << libraryLocation);

    /* Shouldn't be empty */
    CORRADE_VERIFY(!libraryLocation.empty());

    {
        /* https://sourceware.org/bugzilla/show_bug.cgi?id=20292 probably?
           doesn't seem like that, but couldn't find anything else in the
           changelog that would be relevant */
        #ifdef __GLIBC__
        #if __GLIBC__*100 + __GLIBC_MINOR__ < 225
        CORRADE_EXPECT_FAIL("glibc < 2.25 returns executable location from dladdr()");
        #endif
        CORRADE_VERIFY(libraryLocation != Directory::executableLocation());
        #endif

        /* There should be a TestSuite library next to this one */
        const std::string testSuiteLibraryName =
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
        CORRADE_VERIFY(Directory::exists(Directory::join(Directory::path(libraryLocation), testSuiteLibraryName)));
    }

    /* It shouldn't contain null bytes anywhere, especially not at the end */
    CORRADE_COMPARE(libraryLocation.find('\0'), std::string::npos);

    /* On Windows it shouldn't contain backslashes */
    #ifdef CORRADE_TARGET_WINDOWS
    CORRADE_COMPARE(libraryLocation.find('\\'), std::string::npos);
    #endif
    #else
    CORRADE_SKIP("Not implemented on this platform.");
    #endif
}
#else
void DirectoryTest::libraryLocationStatic() {
    #if defined(CORRADE_TARGET_UNIX) || (defined(CORRADE_TARGET_WINDOWS) && !defined(CORRADE_TARGET_WINDOWS_RT))
    const std::string libraryLocation = Directory::libraryLocation(&Utility::Directory::rm);

    CORRADE_INFO("Corrade::Utility library location found as:" << libraryLocation);

    /* No libraries in a static build, so this will print the final executable
       instead */
    #ifdef CORRADE_TARGET_WINDOWS
    CORRADE_COMPARE_AS(libraryLocation,
        "UtilityDirectoryTest.exe",
        TestSuite::Compare::StringHasSuffix);
    #else
    CORRADE_COMPARE_AS(libraryLocation,
        "UtilityDirectoryTest",
        TestSuite::Compare::StringHasSuffix);
    #endif
    #else
    CORRADE_SKIP("Not implemented on this platform.");
    #endif
}
#endif

void DirectoryTest::libraryLocationNull() {
    #if defined(CORRADE_TARGET_UNIX) || (defined(CORRADE_TARGET_WINDOWS) && !defined(CORRADE_TARGET_WINDOWS_RT))
    std::ostringstream out;
    Error redirectError{&out};
    CORRADE_COMPARE(Directory::libraryLocation(nullptr), "");
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

void DirectoryTest::libraryLocationInvalid() {
    #if defined(CORRADE_TARGET_UNIX) || (defined(CORRADE_TARGET_WINDOWS) && !defined(CORRADE_TARGET_WINDOWS_RT))
    std::ostringstream out;
    Error redirectError{&out};
    CORRADE_COMPARE(Directory::libraryLocation(reinterpret_cast<const void*>(0xbadcafe)), "");
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

void DirectoryTest::libraryLocationUtf8() {
    #if defined(CORRADE_TARGET_UNIX) || (defined(CORRADE_TARGET_WINDOWS) && !defined(CORRADE_TARGET_WINDOWS_RT))
    CORRADE_SKIP("Not sure how to test this.");
    #else
    CORRADE_SKIP("Not implemented on this platform.");
    #endif
}

void DirectoryTest::executableLocation() {
    const std::string executableLocation = Directory::executableLocation();
    CORRADE_INFO("Executable location found as:" << executableLocation);

    /* On sandboxed macOS and iOS verify that the directory contains Info.plist
       file */
    #ifdef CORRADE_TARGET_APPLE
    if(System::isSandboxed()) {
        #if defined(CORRADE_TARGET_IOS) && defined(CORRADE_TESTSUITE_TARGET_XCTEST)
        CORRADE_EXPECT_FAIL_IF(!std::getenv("SIMULATOR_UDID"),
            "CTest is not able to run XCTest executables properly in the simulator.");
        #endif

        CORRADE_VERIFY(Directory::exists(Directory::join(Directory::path(executableLocation), "Info.plist")));
    } else
    #endif

    /* On Emscripten we should have access to the bundled files */
    #ifdef CORRADE_TARGET_EMSCRIPTEN
    CORRADE_VERIFY(Directory::exists(Directory::join(Directory::path(executableLocation), "PathTestFiles")));

    /* On Android we can't be sure about anything, so just test that the
       executable exists and it has access to the bundled files */
    #elif defined(CORRADE_TARGET_ANDROID)
    CORRADE_VERIFY(Directory::exists(executableLocation));
    CORRADE_VERIFY(executableLocation.find("UtilityDirectoryTest") != std::string::npos);
    CORRADE_VERIFY(Directory::exists(Directory::join(Directory::path(executableLocation), "PathTestFiles")));

    /* Otherwise it should contain other executables and libraries as we put
       all together */
    #else
    {
        #ifndef CORRADE_TARGET_WINDOWS
        CORRADE_VERIFY(Directory::exists(Directory::join(Directory::path(executableLocation), "corrade-rc")));
        #else
        CORRADE_VERIFY(Directory::exists(Directory::join(Directory::path(executableLocation), "corrade-rc.exe")));
        #endif
    }
    #endif

    /* It shouldn't contain null bytes anywhere, especially not at the end */
    CORRADE_COMPARE(executableLocation.find('\0'), std::string::npos);

    /* On Windows it shouldn't contain backslashes */
    #ifdef CORRADE_TARGET_WINDOWS
    CORRADE_COMPARE(executableLocation.find('\\'), std::string::npos);
    #endif
}

void DirectoryTest::executableLocationInvalid() {
    CORRADE_SKIP("Not sure how to test this.");
}

void DirectoryTest::executableLocationUtf8() {
    CORRADE_SKIP("Not sure how to test this.");
}

void DirectoryTest::home() {
    const std::string home = Directory::home();
    CORRADE_INFO("Home dir found as:" << home);

    /* On macOS and iOS verify that the home dir contains `Library` directory */
    #ifdef CORRADE_TARGET_APPLE
    CORRADE_VERIFY(Directory::exists(Directory::join(home, "Library")));

    /* On other Unixes (except Android, which is shit) verify that the home dir
       contains `.local` directory or is /root. Ugly and hacky, but it's the
       best I came up with. Can't test for e.g. `/home/` substring, as that can
       be overridden. */
    #elif defined(CORRADE_TARGET_UNIX) && !defined(CORRADE_TARGET_ANDROID)
    CORRADE_VERIFY(Directory::exists(home));
    CORRADE_VERIFY(Directory::exists(Directory::join(home, ".local")) || home == "/root");

    /* On Emscripten verify that the directory exists (it's empty by default) */
    #elif defined(CORRADE_TARGET_EMSCRIPTEN)
    CORRADE_VERIFY(Directory::exists(home));

    /* On Windows verify that the home dir contains `desktop.ini` file. Ugly
       and hacky, but it's the best I came up with. Can't test for e.g.
       `/Users/` substring, as that can be overridden. */
    #elif defined(CORRADE_TARGET_WINDOWS)
    CORRADE_VERIFY(Directory::exists(Directory::join(home, "desktop.ini")));

    /* No idea elsewhere */
    #else
    {
        CORRADE_EXPECT_FAIL("Not implemented yet.");
        CORRADE_COMPARE(home, "(not implemented)");
    }
    #endif

    /* It shouldn't contain null bytes anywhere, especially not at the end */
    CORRADE_COMPARE(home.find('\0'), std::string::npos);

    /* On Windows it shouldn't contain backslashes */
    #ifdef CORRADE_TARGET_WINDOWS
    CORRADE_COMPARE(home.find('\\'), std::string::npos);
    #endif
}

void DirectoryTest::homeInvalid() {
    /* Could be tested by temporarily removing $HOME, but ... ahem */
    CORRADE_SKIP("Not sure how to test this.");
}

void DirectoryTest::homeUtf8() {
    CORRADE_SKIP("Not sure how to test this.");
}

void DirectoryTest::configurationDir() {
    const std::string dir = Directory::configurationDir("Corrade");
    CORRADE_INFO("Configuration dir found as:" << dir);

    #ifdef CORRADE_TARGET_APPLE
    CORRADE_COMPARE(dir.substr(dir.size() - 7), "Corrade");
    if(System::isSandboxed())
        CORRADE_VERIFY(Directory::exists(Directory::join(Directory::path(Directory::path(dir)), "Caches")));
    else
        /* App Store is not present on *some* Travis VMs since 2018-08-05.
           CrashReporter is. */
        CORRADE_VERIFY(
            Directory::exists(Directory::join(Directory::path(dir), "App Store")) ||
            Directory::exists(Directory::join(Directory::path(dir), "CrashReporter")));

    /* On Linux verify that the parent dir contains `autostart` directory,
       something from GTK or something from Qt. Ugly and hacky, but it's the
       best I could come up with. Can't test for e.g. `/home/` substring, as
       that can be overridden. */
    #elif defined(__linux__) && !defined(CORRADE_TARGET_ANDROID)
    CORRADE_COMPARE(dir.substr(dir.size()-7), "corrade");
    CORRADE_VERIFY(Directory::exists(Directory::join(Directory::path(dir), "autostart")) ||
                   Directory::exists(Directory::join(Directory::path(dir), "dconf")) ||
                   Directory::exists(Directory::join(Directory::path(dir), "Trolltech.conf")));

    /* Emscripten -- just compare to hardcoded value */
    #elif defined(CORRADE_TARGET_EMSCRIPTEN)
    CORRADE_COMPARE(Directory::path(dir), "/home/web_user/.config");

    /* On Windows verify that the parent dir contains `Microsoft` subdirectory.
       Ugly and hacky, but it's the best I came up with. Can't test for e.g.
       `/Users/` substring, as that can be overridden. */
    #elif defined(CORRADE_TARGET_WINDOWS)
    CORRADE_COMPARE(dir.substr(dir.size()-7), "Corrade");
    CORRADE_VERIFY(Directory::exists(Directory::join(Directory::path(dir), "Microsoft")));

    /* No idea elsewhere */
    #else
    {
        CORRADE_EXPECT_FAIL("Not implemented yet.");
        CORRADE_COMPARE(dir, "(not implemented)");
    }
    #endif

    /* It shouldn't contain null bytes anywhere, especially not at the end */
    CORRADE_COMPARE(dir.find('\0'), std::string::npos);

    /* On Windows it shouldn't contain backslashes */
    #ifdef CORRADE_TARGET_WINDOWS
    CORRADE_COMPARE(dir.find('\\'), std::string::npos);
    #endif
}

void DirectoryTest::configurationDirInvalid() {
    /* Could be tested by temporarily removing $XDG_CONFIG_HOME and $HOME, but
       ... ahem */
    CORRADE_SKIP("Not sure how to test this.");
}

void DirectoryTest::configurationDirUtf8() {
    CORRADE_SKIP("Not sure how to test this.");
}

void DirectoryTest::tmp() {
    const std::string dir = Directory::tmp();
    CORRADE_INFO("Temporary dir found as:" << dir);

    #if defined(CORRADE_TARGET_UNIX) || defined(CORRADE_TARGET_EMSCRIPTEN)
    {
        #if defined(CORRADE_TARGET_IOS) && defined(CORRADE_TESTSUITE_TARGET_XCTEST)
        CORRADE_EXPECT_FAIL_IF(!std::getenv("SIMULATOR_UDID"),
            "CTest is not able to run XCTest executables properly in the simulator.");
        #endif
        CORRADE_VERIFY(Directory::exists(dir));
    }
    CORRADE_VERIFY(dir.find("tmp") != std::string::npos);

    #elif defined(CORRADE_TARGET_WINDOWS) && !defined(CORRADE_TARGET_WINDOWS_RT)
    CORRADE_VERIFY(Directory::exists(dir));
    #ifndef __MINGW32__
    CORRADE_VERIFY(dir.find("Temp") != std::string::npos);
    #else
    /* MinGW shell maps temp to a different directory, e.g. C:/msys64/tmp, so
       check for both */
    CORRADE_VERIFY(dir.find("Temp") != std::string::npos || dir.find("tmp") != std::string::npos);
    #endif

    /* No idea elsewhere */
    #else
    {
        CORRADE_EXPECT_FAIL("Not implemented yet.");
        CORRADE_COMPARE(dir, "(not implemented)");
    }
    #endif

    /* It shouldn't contain null bytes anywhere, especially not at the end */
    CORRADE_COMPARE(dir.find('\0'), std::string::npos);

    /* On Windows it shouldn't contain backslashes */
    #ifdef CORRADE_TARGET_WINDOWS
    CORRADE_COMPARE(dir.find('\\'), std::string::npos);
    #endif

    /* Verify that it's possible to write stuff there */
    {
        #if defined(CORRADE_TARGET_IOS) && defined(CORRADE_TESTSUITE_TARGET_XCTEST)
        CORRADE_EXPECT_FAIL_IF(!std::getenv("SIMULATOR_UDID"),
            "CTest is not able to run XCTest executables properly in the simulator.");
        #endif
        CORRADE_VERIFY(Directory::writeString(Directory::join(Directory::tmp(), "a"), "hello"));
        CORRADE_VERIFY(Directory::rm(Directory::join(Directory::tmp(), "a")));
    }
}

void DirectoryTest::tmpInvalid() {
    CORRADE_SKIP("Not known to fail on any known system.");
}

void DirectoryTest::tmpUtf8() {
    CORRADE_SKIP("Not sure how to test this.");
}

void DirectoryTest::list() {
    #if defined(CORRADE_TARGET_IOS) && defined(CORRADE_TESTSUITE_TARGET_XCTEST)
    CORRADE_EXPECT_FAIL_IF(!std::getenv("SIMULATOR_UDID"),
        "CTest is not able to run XCTest executables properly in the simulator.");
    #endif

    CORRADE_COMPARE_AS(Directory::list(_testDir),
        (std::vector<std::string>{".", "..", "dir", "file"}),
        TestSuite::Compare::SortedContainer);
}

void DirectoryTest::listSkipDirectories() {
    #if defined(CORRADE_TARGET_IOS) && defined(CORRADE_TESTSUITE_TARGET_XCTEST)
    CORRADE_EXPECT_FAIL_IF(!std::getenv("SIMULATOR_UDID"),
        "CTest is not able to run XCTest executables properly in the simulator.");
    #endif

    CORRADE_COMPARE_AS(Directory::list(_testDir, Directory::Flag::SkipDirectories),
        std::vector<std::string>{"file"},
        TestSuite::Compare::SortedContainer);
}

void DirectoryTest::listSkipDirectoriesSymlinks() {
    #if defined(CORRADE_TARGET_IOS) && defined(CORRADE_TESTSUITE_TARGET_XCTEST)
    CORRADE_EXPECT_FAIL_IF(!std::getenv("SIMULATOR_UDID"),
        "CTest is not able to run XCTest executables properly in the simulator.");
    #endif

    #if !defined(CORRADE_TARGET_UNIX) && !defined(CORRADE_TARGET_EMSCRIPTEN)
    /* Possible on Windows too, but there we'd need to first detect if the
       Git clone has the symlinks preserved */
    CORRADE_EXPECT_FAIL("Symlink support is implemented on Unix systems and Emscripten only.");
    #endif
    CORRADE_COMPARE_AS(Directory::list(_testDirSymlink, Directory::Flag::SkipDirectories),
        (std::vector<std::string>{"file", "file-symlink"}),
        TestSuite::Compare::SortedContainer);
}

void DirectoryTest::listSkipFiles() {
    #if defined(CORRADE_TARGET_IOS) && defined(CORRADE_TESTSUITE_TARGET_XCTEST)
    CORRADE_EXPECT_FAIL_IF(!std::getenv("SIMULATOR_UDID"),
        "CTest is not able to run XCTest executables properly in the simulator.");
    #endif

    CORRADE_COMPARE_AS(Directory::list(_testDir, Directory::Flag::SkipFiles),
        (std::vector<std::string>{".", "..", "dir"}),
        TestSuite::Compare::SortedContainer);
}

void DirectoryTest::listSkipFilesSymlinks() {
    #if defined(CORRADE_TARGET_IOS) && defined(CORRADE_TESTSUITE_TARGET_XCTEST)
    CORRADE_EXPECT_FAIL_IF(!std::getenv("SIMULATOR_UDID"),
        "CTest is not able to run XCTest executables properly in the simulator.");
    #endif

    #if !defined(CORRADE_TARGET_UNIX) && !defined(CORRADE_TARGET_EMSCRIPTEN)
    /* Possible on Windows too, but there we'd need to first detect if the
       Git clone has the symlinks preserved */
    CORRADE_EXPECT_FAIL("Symlink support is implemented on Unix systems and Emscripten only.");
    #endif
    CORRADE_COMPARE_AS(Directory::list(_testDirSymlink, Directory::Flag::SkipFiles),
        (std::vector<std::string>{".", "..", "dir", "dir-symlink"}),
        TestSuite::Compare::SortedContainer);
}

void DirectoryTest::listSkipSpecial() {
    #if defined(CORRADE_TARGET_IOS) && defined(CORRADE_TESTSUITE_TARGET_XCTEST)
    CORRADE_EXPECT_FAIL_IF(!std::getenv("SIMULATOR_UDID"),
        "CTest is not able to run XCTest executables properly in the simulator.");
    #endif

    CORRADE_COMPARE_AS(Directory::list(_testDir, Directory::Flag::SkipSpecial),
        (std::vector<std::string>{".", "..", "dir", "file"}),
        TestSuite::Compare::SortedContainer);
}

void DirectoryTest::listSkipSpecialSymlink() {
    #if defined(CORRADE_TARGET_IOS) && defined(CORRADE_TESTSUITE_TARGET_XCTEST)
    CORRADE_EXPECT_FAIL_IF(!std::getenv("SIMULATOR_UDID"),
        "CTest is not able to run XCTest executables properly in the simulator.");
    #endif

    /* Symlinks should not be treated as special files, so they're shown */
    CORRADE_COMPARE_AS(Directory::list(_testDirSymlink, Directory::Flag::SkipSpecial),
        (std::vector<std::string>{".", "..", "dir", "dir-symlink", "file", "file-symlink"}),
        TestSuite::Compare::SortedContainer);
}

void DirectoryTest::listSkipDotAndDotDot() {
    #if defined(CORRADE_TARGET_IOS) && defined(CORRADE_TESTSUITE_TARGET_XCTEST)
    CORRADE_EXPECT_FAIL_IF(!std::getenv("SIMULATOR_UDID"),
        "CTest is not able to run XCTest executables properly in the simulator.");
    #endif

    CORRADE_COMPARE_AS(Directory::list(_testDir, Directory::Flag::SkipDotAndDotDot),
        (std::vector<std::string>{"dir", "file"}),
        TestSuite::Compare::SortedContainer);
}

void DirectoryTest::listSort() {
    #if defined(CORRADE_TARGET_IOS) && defined(CORRADE_TESTSUITE_TARGET_XCTEST)
    CORRADE_EXPECT_FAIL_IF(!std::getenv("SIMULATOR_UDID"),
        "CTest is not able to run XCTest executables properly in the simulator.");
    #endif

    CORRADE_COMPARE_AS(Directory::list(_testDir, Directory::Flag::SortAscending),
        (std::vector<std::string>{".", "..", "dir", "file"}),
        TestSuite::Compare::Container);

    CORRADE_COMPARE_AS(Directory::list(_testDir, Directory::Flag::SortDescending),
        (std::vector<std::string>{"file", "dir", "..", "."}),
        TestSuite::Compare::Container);
}

void DirectoryTest::listSortPrecedence() {
    CORRADE_VERIFY((Directory::Flag::SortAscending|Directory::Flag::SortDescending) == Directory::Flag::SortAscending);
}

void DirectoryTest::listNonexistent() {
    std::ostringstream out;
    Error redirectError{&out};
    CORRADE_COMPARE(Directory::list("nonexistent"), std::vector<std::string>{});
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

void DirectoryTest::listUtf8() {
    std::vector<std::string> list{".", "..", "hýždě", "šňůra"};

    std::vector<std::string> actual = Directory::list(_testDirUtf8, Directory::Flag::SortAscending);

    #ifdef CORRADE_TARGET_APPLE
    /* Apple HFS+ stores filenames in a decomposed normalized form to avoid
       e.g. `e` + `ˇ` and `ě` being treated differently. That makes sense. I
       wonder why neither Linux nor Windows do this. */
    std::vector<std::string> listDecomposedUnicode{".", "..", "hýždě", "šňůra"};
    CORRADE_VERIFY(list[3] != listDecomposedUnicode[3]);
    #endif

    #ifdef CORRADE_TARGET_APPLE
    #if defined(CORRADE_TARGET_IOS) && defined(CORRADE_TESTSUITE_TARGET_XCTEST)
    CORRADE_EXPECT_FAIL_IF(!std::getenv("SIMULATOR_UDID"),
        "CTest is not able to run XCTest executables properly in the simulator.");
    #endif

    /* However, Apple systems still can use filesystems other than HFS+, so
       be prepared that it can compare to either */
    if(actual[3] == listDecomposedUnicode[3]) {
        CORRADE_COMPARE_AS(actual, listDecomposedUnicode,
            TestSuite::Compare::Container);
    } else
    #endif
    {
        CORRADE_COMPARE_AS(actual, list, TestSuite::Compare::Container);
    }
}

/* Checks if we are reading it as binary (CR+LF is not converted to LF),
   nothing after \0 gets lost, and invalid UTF-8 chars (over 0x80) also cause
   no issues */
constexpr const char Data[]{'\xCA', '\xFE', '\xBA', '\xBE', '\x0D', '\x0A', '\x00', '\xDE', '\xAD', '\xBE', '\xEF'};

void DirectoryTest::fileSize() {
    /* Existing file, containing the above data */
    CORRADE_COMPARE(Directory::fileSize(Directory::join(_testDir, "file")),
        Containers::arraySize(Data));
}

void DirectoryTest::fileSizeEmpty() {
    const std::string empty = Directory::join(_testDir, "dir/dummy");
    CORRADE_VERIFY(Directory::exists(empty));
    CORRADE_COMPARE(Directory::fileSize(empty), 0);
}

void DirectoryTest::fileSizeNonSeekable() {
    /* macOS or BSD doesn't have /proc */
    #if defined(__unix__) && !defined(CORRADE_TARGET_EMSCRIPTEN) && \
        !defined(__FreeBSD__) && !defined(__OpenBSD__) && !defined(__bsdi__) && \
        !defined(__NetBSD__) && !defined(__DragonFly__)
    std::ostringstream out;
    Error redirectError{&out};
    /* /proc/loadavg works on Android Emulator but not on a real device;
       /proc/zoneinfo works everywhere */
    CORRADE_VERIFY(!Directory::fileSize("/proc/zoneinfo"));
    CORRADE_COMPARE(out.str(), "Utility::Path::size(): /proc/zoneinfo is not seekable\n");
    #else
    CORRADE_SKIP("Not implemented on this platform.");
    #endif
}

void DirectoryTest::fileSizeEarlyEof() {
    #ifdef __linux__
    constexpr const char* file = "/sys/devices/system/cpu/cpu0/cpufreq/scaling_governor";
    if(!Directory::exists(file))
        CORRADE_SKIP(file << "doesn't exist, can't test");
    Containers::Optional<std::size_t> size = Directory::fileSize(file);
    Containers::Array<char> data = Directory::read(file);
    CORRADE_VERIFY(size);
    CORRADE_COMPARE_AS(*size, data.size(), TestSuite::Compare::Greater);
    #else
    CORRADE_SKIP("Not sure how to test on this platform.");
    #endif
}

void DirectoryTest::fileSizeDirectory() {
    #if defined(CORRADE_TARGET_IOS) && defined(CORRADE_TESTSUITE_TARGET_XCTEST)
    if(!std::getenv("SIMULATOR_UDID"))
        CORRADE_SKIP("iOS (in a simulator) thinks all paths are files, can't test.");
    #endif

    std::ostringstream out;
    Error redirectError{&out};
    CORRADE_COMPARE(Directory::fileSize(_testDir), Containers::NullOpt);

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

void DirectoryTest::fileSizeNonexistent() {
    std::ostringstream out;
    Error redirectError{&out};
    CORRADE_COMPARE(Directory::fileSize("nonexistent"), Containers::NullOpt);
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

void DirectoryTest::fileSizeUtf8() {
    CORRADE_COMPARE(Directory::fileSize(Directory::join(_testDirUtf8, "hýždě")),
        Containers::arraySize(Data));
}

void DirectoryTest::read() {
    CORRADE_COMPARE_AS(Directory::read(Directory::join(_testDir, "file")),
        Containers::arrayView(Data),
        TestSuite::Compare::Container);

    /* Read into string */
    std::string string = Directory::readString(Directory::join(_testDir, "file"));
    CORRADE_COMPARE(string,
        std::string(Data, Containers::arraySize(Data)));
}

void DirectoryTest::readEmpty() {
    const std::string empty = Directory::join(_testDir, "dir/dummy");
    CORRADE_VERIFY(Directory::exists(empty));
    CORRADE_VERIFY(!Directory::read(empty));
}

void DirectoryTest::readNonSeekable() {
    /* macOS or BSD doesn't have /proc */
    #if defined(__unix__) && !defined(CORRADE_TARGET_EMSCRIPTEN) && \
        !defined(__FreeBSD__) && !defined(__OpenBSD__) && !defined(__bsdi__) && \
        !defined(__NetBSD__) && !defined(__DragonFly__)
    /** @todo Test more thoroughly than this */
    /* /proc/loadavg works on Android Emulator but not on a real device;
       /proc/zoneinfo works everywhere */
    const Containers::Array<char> data = Directory::read("/proc/zoneinfo");
    CORRADE_VERIFY(!data.empty());
    #else
    CORRADE_SKIP("Not implemented on this platform.");
    #endif
}

void DirectoryTest::readEarlyEof() {
    #ifdef __linux__
    if(!Directory::exists("/sys/devices/system/cpu/cpu0/cpufreq/scaling_governor"))
        CORRADE_SKIP("/sys/devices/system/cpu/cpu0/cpufreq/scaling_governor doesn't exist, can't test");
    const Containers::Array<char> data = Directory::read("/sys/devices/system/cpu/cpu0/cpufreq/scaling_governor");
    CORRADE_VERIFY(!data.empty());
    #else
    CORRADE_SKIP("Not sure how to test on this platform.");
    #endif
}

void DirectoryTest::readDirectory() {
    #if defined(CORRADE_TARGET_IOS) && defined(CORRADE_TESTSUITE_TARGET_XCTEST)
    if(!std::getenv("SIMULATOR_UDID"))
        CORRADE_SKIP("iOS (in a simulator) thinks all paths are files, can't test.");
    #endif

    std::ostringstream out;
    Error redirectError{&out};
    CORRADE_VERIFY(!Directory::read(_testDir));

    /* On Windows the opening itself fails, on Unix we have an explicit check.
       On other systems no idea, so let's say we expect the same message as on
       Unix. */
    #ifdef CORRADE_TARGET_WINDOWS
    CORRADE_COMPARE_AS(out.str(),
        formatString("Utility::Path::read(): can't open {}: error 13 (", _testDir),
        TestSuite::Compare::StringHasPrefix);
    #else
    CORRADE_COMPARE(out.str(), formatString("Utility::Path::read(): {} is a directory\n", _testDir));
    #endif

    /* Nonexistent file into string shouldn't throw on nullptr */
    CORRADE_VERIFY(Directory::readString(_testDir).empty());
}

void DirectoryTest::readNonexistent() {
    std::ostringstream out;
    Error redirectError{&out};
    CORRADE_VERIFY(!Directory::read("nonexistent"));
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

    /* Nonexistent file into string shouldn't throw on nullptr */
    CORRADE_VERIFY(Directory::readString("nonexistent").empty());
}

void DirectoryTest::readUtf8() {
    CORRADE_COMPARE_AS(Directory::read(Directory::join(_testDirUtf8, "hýždě")),
        Containers::arrayView(Data),
        TestSuite::Compare::Container);
}

void DirectoryTest::write() {
    std::string file = Directory::join(_writeTestDir, "file");

    if(Directory::exists(file)) CORRADE_VERIFY(Directory::rm(file));
    CORRADE_VERIFY(Directory::write(file, Data));
    CORRADE_COMPARE_AS(file, Directory::join(_testDir, "file"),
        TestSuite::Compare::File);

    CORRADE_VERIFY(Directory::rm(file));
    CORRADE_VERIFY(Directory::writeString(file, std::string(Data, 11)));
    CORRADE_COMPARE_AS(file, Directory::join(_testDir, "file"),
        TestSuite::Compare::File);
}

void DirectoryTest::writeEmpty() {
    std::string file = Directory::join(_writeTestDir, "empty");

    if(Directory::exists(file)) CORRADE_VERIFY(Directory::rm(file));
    CORRADE_VERIFY(Directory::write(file, nullptr));
    CORRADE_COMPARE_AS(file, "",
        TestSuite::Compare::FileToString);
}

void DirectoryTest::writeDirectory() {
    std::ostringstream out;
    Error redirectError{&out};
    CORRADE_VERIFY(!Directory::write(_writeTestDir, nullptr));
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

void DirectoryTest::writeNoPermission() {
    #ifdef CORRADE_TARGET_EMSCRIPTEN
    CORRADE_SKIP("Everything is writable under Emscripten.");
    #else
    #ifdef CORRADE_TARGET_APPLE
    /* Assuming there's no real possibility to run as root on Apple */
    std::string filename = "/var/root/writtenFile";
    #elif defined(CORRADE_TARGET_ANDROID)
    /* Same here, would need a rooted device */
    std::string filename = "/data/local/writtenFile";
    #elif defined(CORRADE_TARGET_UNIX)
    std::string filename = "/root/writtenFile";
    if(Directory::home() == "/root")
        CORRADE_SKIP("Running under root, can't test for permissions.");
    #elif defined(CORRADE_TARGET_WINDOWS)
    /* Only the TrustedInstaller system user is supposed to have access into
       WindowsApps */
    std::string filename = "C:/Program Files/WindowsApps/writtenFile";
    #elif defined(CORRADE_TARGET_EMSCRIPTEN)
    #else
    std::string filename;
    CORRADE_SKIP("Not sure how to test on this system.");
    #endif

    std::ostringstream out;
    Error redirectError{&out};
    CORRADE_VERIFY(!Directory::write(filename, nullptr));
    CORRADE_COMPARE_AS(out.str(),
        formatString("Utility::Path::write(): can't open {}: error 13 (", filename),
        TestSuite::Compare::StringHasPrefix);
    #endif
}

void DirectoryTest::writeUtf8() {
    std::string file = Directory::join(_writeTestDir, "hýždě");

    if(Directory::exists(file)) CORRADE_VERIFY(Directory::rm(file));
    CORRADE_VERIFY(Directory::write(file, Data));
    CORRADE_COMPARE_AS(file, Directory::join(_testDirUtf8, "hýždě"),
        TestSuite::Compare::File);
}

void DirectoryTest::append() {
    constexpr const char expected[]{'h', 'e', 'l', 'l', 'o', '\xCA', '\xFE', '\xBA', '\xBE', '\x0D', '\x0A', '\x00', '\xDE', '\xAD', '\xBE', '\xEF'};

    std::string file = Directory::join(_writeTestDir, "file");
    if(Directory::exists(file)) CORRADE_VERIFY(Directory::rm(file));
    CORRADE_VERIFY(Directory::writeString(file, "hello"));

    CORRADE_VERIFY(Directory::append(file, Data));
    CORRADE_COMPARE_AS(file, (std::string{expected, Containers::arraySize(expected)}),
        TestSuite::Compare::FileToString);

    CORRADE_VERIFY(Directory::writeString(file, "hello"));

    CORRADE_VERIFY(Directory::appendString(file, std::string{Data, Containers::arraySize(Data)}));
    CORRADE_COMPARE_AS(file, (std::string{expected, Containers::arraySize(expected)}),
        TestSuite::Compare::FileToString);
}

void DirectoryTest::appendToNonexistent() {
    std::string file = Directory::join(_writeTestDir, "empty");

    if(Directory::exists(file)) CORRADE_VERIFY(Directory::rm(file));

    CORRADE_VERIFY(Directory::appendString(file, "hello"));
    CORRADE_COMPARE_AS(file, "hello",
        TestSuite::Compare::FileToString);
}

void DirectoryTest::appendEmpty() {
    std::string file = Directory::join(_writeTestDir, "empty");

    if(Directory::exists(file)) CORRADE_VERIFY(Directory::rm(file));
    CORRADE_VERIFY(Directory::writeString(file, "hello"));

    CORRADE_VERIFY(Directory::append(file, nullptr));
    CORRADE_COMPARE_AS(file, "hello",
        TestSuite::Compare::FileToString);
}

void DirectoryTest::appendDirectory() {
    std::ostringstream out;
    Error redirectError{&out};
    CORRADE_VERIFY(!Directory::append(_writeTestDir, nullptr));
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

void DirectoryTest::appendNoPermission() {
    #ifdef CORRADE_TARGET_EMSCRIPTEN
    CORRADE_SKIP("Everything is writable under Emscripten.");
    #else
    #ifdef CORRADE_TARGET_APPLE
    /* Assuming there's no real possibility to run as root on Apple */
    std::string filename = "/var/root/writtenFile";
    #elif defined(CORRADE_TARGET_ANDROID)
    /* Same here, would need a rooted device */
    std::string filename = "/data/local/writtenFile";
    #elif defined(CORRADE_TARGET_UNIX)
    std::string filename = "/root/writtenFile";
    if(Directory::home() == "/root")
        CORRADE_SKIP("Running under root, can't test for permissions.");
    #elif defined(CORRADE_TARGET_WINDOWS)
    /* Only the TrustedInstaller system user is supposed to have access into
       WindowsApps */
    std::string filename = "C:/Program Files/WindowsApps/writtenFile";
    #else
    std::string filename;
    CORRADE_SKIP("Not sure how to test on this system.");
    #endif

    std::ostringstream out;
    Error redirectError{&out};
    CORRADE_VERIFY(!Directory::append(filename, nullptr));
    CORRADE_COMPARE_AS(out.str(),
        formatString("Utility::Path::append(): can't open {}: error 13 (", filename),
        TestSuite::Compare::StringHasPrefix);
    #endif
}

void DirectoryTest::appendUtf8() {
    std::string file = Directory::join(_writeTestDir, "hýždě");

    if(Directory::exists(file)) CORRADE_VERIFY(Directory::rm(file));
    CORRADE_VERIFY(Directory::append(file, Data));
    CORRADE_COMPARE_AS(file, Directory::join(_testDirUtf8, "hýždě"),
        TestSuite::Compare::File);
}

void DirectoryTest::prepareFileToCopy() {
    if(Directory::exists(Directory::join(_writeTestDir, "copySource.dat")))
        return;

    Containers::Array<int> data{NoInit, 150000};
    for(std::size_t i = 0; i != data.size(); ++i) data[i] = 4678641 + i;

    Directory::write(Directory::join(_writeTestDir, "copySource.dat"), data);
}

void DirectoryTest::copy() {
    const std::string source = Directory::join(_writeTestDir, "copySource.dat");
    const std::string destination = Directory::join(_writeTestDir, "copyDestination.dat");
    CORRADE_VERIFY(Directory::exists(source));
    CORRADE_VERIFY(Directory::copy(source, destination));
    CORRADE_COMPARE_AS(source, destination, TestSuite::Compare::File);
}

void DirectoryTest::copyEmpty() {
    std::string source = Directory::join(_testDir, "dir/dummy");
    CORRADE_VERIFY(Directory::exists(source));

    std::string destination = Directory::join(_writeTestDir, "empty");
    if(Directory::exists(destination)) CORRADE_VERIFY(Directory::rm(destination));

    CORRADE_VERIFY(Directory::copy(source, destination));
    CORRADE_COMPARE_AS(destination, "",
        TestSuite::Compare::FileToString);
}

void DirectoryTest::copyDirectory() {
    #if defined(CORRADE_TARGET_IOS) && defined(CORRADE_TESTSUITE_TARGET_XCTEST)
    if(!std::getenv("SIMULATOR_UDID"))
        CORRADE_SKIP("iOS (in a simulator) thinks all paths are files, can't test.");
    #endif

    const std::string source = Directory::join(_writeTestDir, "copySource.dat");
    const std::string destination = Directory::join(_writeTestDir, "copyDestination.dat");
    CORRADE_VERIFY(Directory::exists(source));

    {
        std::ostringstream out;
        Error redirectError{&out};
        CORRADE_VERIFY(!Directory::copy(source, _writeTestDir));
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
        CORRADE_VERIFY(!Directory::copy(_writeTestDir, destination));

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

void DirectoryTest::copyReadNonexistent() {
    std::ostringstream out;
    Error redirectError{&out};
    CORRADE_VERIFY(!Directory::copy("nonexistent", Directory::join(_writeTestDir, "empty")));
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

void DirectoryTest::copyWriteNoPermission() {
    #ifdef CORRADE_TARGET_EMSCRIPTEN
    CORRADE_SKIP("Everything is writable under Emscripten.");
    #else
    #ifdef CORRADE_TARGET_APPLE
    /* Assuming there's no real possibility to run as root on Apple */
    std::string filename = "/var/root/writtenFile";
    #elif defined(CORRADE_TARGET_ANDROID)
    /* Same here, would need a rooted device */
    std::string filename = "/data/local/writtenFile";
    #elif defined(CORRADE_TARGET_UNIX)
    std::string filename = "/root/writtenFile";
    if(Directory::home() == "/root")
        CORRADE_SKIP("Running under root, can't test for permissions.");
    #elif defined(CORRADE_TARGET_WINDOWS)
    /* Only the TrustedInstaller system user is supposed to have access into
       WindowsApps */
    std::string filename = "C:/Program Files/WindowsApps/writtenFile";
    #else
    std::string filename;
    CORRADE_SKIP("Not sure how to test on this system.");
    #endif

    std::ostringstream out;
    Error redirectError{&out};
    CORRADE_VERIFY(!Directory::copy(Directory::join(_testDir, "dir/dummy"), filename));
    CORRADE_COMPARE_AS(out.str(),
        formatString("Utility::Path::copy(): can't open {} for writing: error 13 (", filename),
        TestSuite::Compare::StringHasPrefix);
    #endif
}

void DirectoryTest::copyUtf8() {
    std::string output = Directory::join(_writeTestDir, "hýždě");

    if(Directory::exists(output)) CORRADE_VERIFY(Directory::rm(output));

    CORRADE_VERIFY(Directory::copy(Directory::join(_testDirUtf8, "hýždě"), output));
    CORRADE_COMPARE_AS(Directory::join(_writeTestDir, "hýždě"),
        Directory::join(_testDirUtf8, "hýždě"),
        TestSuite::Compare::File);
}

#ifndef CORRADE_TARGET_EMSCRIPTEN
void DirectoryTest::prepareFileToBenchmarkCopy() {
    if(Directory::exists(Directory::join(_writeTestDir, "copyBenchmarkSource.dat")))
        return;

    /* Append a megabyte file 50 times to create a 50MB file */
    Containers::Array<int> data{ValueInit, 256*1024};
    for(std::size_t i = 0; i != data.size(); ++i) data[i] = 4678641 + i;

    for(std::size_t i = 0; i != 50; ++i)
        Directory::append(Directory::join(_writeTestDir, "copyBenchmarkSource.dat"), data);
}

void DirectoryTest::copy100MReadWrite() {
    std::string input = Directory::join(_writeTestDir, "copyBenchmarkSource.dat");
    std::string output = Directory::join(_writeTestDir, "copyDestination.dat");
    CORRADE_VERIFY(Directory::exists(input));
    if(Directory::exists(output)) CORRADE_VERIFY(Directory::rm(output));

    CORRADE_BENCHMARK(1)
        Directory::write(output, Directory::read(input));
}

void DirectoryTest::copy100MReadWriteString() {
    std::string input = Directory::join(_writeTestDir, "copyBenchmarkSource.dat");
    std::string output = Directory::join(_writeTestDir, "copyDestination.dat");
    CORRADE_VERIFY(Directory::exists(input));
    if(Directory::exists(output)) CORRADE_VERIFY(Directory::rm(output));

    CORRADE_BENCHMARK(1)
        Directory::writeString(output, Directory::readString(input));
}

void DirectoryTest::copy100MCopy() {
    std::string input = Directory::join(_writeTestDir, "copyBenchmarkSource.dat");
    std::string output = Directory::join(_writeTestDir, "copyDestination.dat");
    CORRADE_VERIFY(Directory::exists(input));
    if(Directory::exists(output)) CORRADE_VERIFY(Directory::rm(output));

    CORRADE_BENCHMARK(1)
        Directory::copy(input, output);
}

#if defined(DOXYGEN_GENERATING_OUTPUT) || defined(CORRADE_TARGET_UNIX) || (defined(CORRADE_TARGET_WINDOWS) && !defined(CORRADE_TARGET_WINDOWS_RT))
void DirectoryTest::copy100MMap() {
    std::string input = Directory::join(_writeTestDir, "copyBenchmarkSource.dat");
    std::string output = Directory::join(_writeTestDir, "copyDestination.dat");
    CORRADE_VERIFY(Directory::exists(input));
    if(Directory::exists(output)) CORRADE_VERIFY(Directory::rm(output));

    CORRADE_BENCHMARK(1)
        Directory::write(output, Directory::mapRead(input));
}
#endif
#endif

void DirectoryTest::map() {
    #if defined(CORRADE_TARGET_UNIX) || (defined(CORRADE_TARGET_WINDOWS) && !defined(CORRADE_TARGET_WINDOWS_RT))
    std::string file = Directory::join(_writeTestDir, "mappedFile");
    if(Directory::exists(file)) CORRADE_VERIFY(Directory::rm(file));
    CORRADE_VERIFY(Directory::write(file, Data));

    {
        Containers::Array<char, Directory::MapDeleter> mappedFile = Directory::map(file);
        CORRADE_COMPARE_AS(mappedFile,
            Containers::arrayView(Data),
            TestSuite::Compare::Container);

        /* Write a thing there */
        mappedFile[2] = '\xCA';
        mappedFile[3] = '\xFE';

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

void DirectoryTest::mapEmpty() {
    #if defined(CORRADE_TARGET_UNIX) || (defined(CORRADE_TARGET_WINDOWS) && !defined(CORRADE_TARGET_WINDOWS_RT))
    std::string file = Directory::join(_writeTestDir, "mappedEmpty");
    if(Directory::exists(file)) CORRADE_VERIFY(Directory::rm(file));
    CORRADE_VERIFY(Directory::write(file, nullptr));

    {
        Containers::Array<char, Directory::MapDeleter> mappedFile = Directory::map(file);
        CORRADE_COMPARE_AS(mappedFile,
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

void DirectoryTest::mapDirectory() {
    #if defined(CORRADE_TARGET_UNIX) || (defined(CORRADE_TARGET_WINDOWS) && !defined(CORRADE_TARGET_WINDOWS_RT))
    std::ostringstream out;
    Error redirectError{&out};
    CORRADE_VERIFY(!Directory::map(_writeTestDir));
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

void DirectoryTest::mapNonexistent() {
    #if defined(CORRADE_TARGET_UNIX) || (defined(CORRADE_TARGET_WINDOWS) && !defined(CORRADE_TARGET_WINDOWS_RT))
    std::ostringstream out;
    Error redirectError{&out};
    CORRADE_VERIFY(!Directory::map("nonexistent"));
    CORRADE_COMPARE_AS(out.str(),
        "Utility::Path::map(): can't open nonexistent: error 2 (",
        TestSuite::Compare::StringHasPrefix);
    #else
    CORRADE_SKIP("Not implemented on this platform.");
    #endif
}

void DirectoryTest::mapUtf8() {
    #if defined(CORRADE_TARGET_UNIX) || (defined(CORRADE_TARGET_WINDOWS) && !defined(CORRADE_TARGET_WINDOWS_RT))
    const Containers::Array<char, Directory::MapDeleter> mappedFile = Directory::map(Directory::join(_testDirUtf8, "hýždě"));
    CORRADE_COMPARE_AS(mappedFile,
        Containers::arrayView(Data),
        TestSuite::Compare::Container);
    #else
    CORRADE_SKIP("Not implemented on this platform.");
    #endif
}

void DirectoryTest::mapRead() {
    #if defined(CORRADE_TARGET_UNIX) || (defined(CORRADE_TARGET_WINDOWS) && !defined(CORRADE_TARGET_WINDOWS_RT))
    const Containers::Array<const char, Directory::MapDeleter> mappedFile = Directory::mapRead(Directory::join(_testDir, "file"));
    CORRADE_COMPARE_AS(mappedFile,
        Containers::arrayView(Data),
        TestSuite::Compare::Container);
    #else
    CORRADE_SKIP("Not implemented on this platform.");
    #endif
}

void DirectoryTest::mapReadEmpty() {
    #if defined(CORRADE_TARGET_UNIX) || (defined(CORRADE_TARGET_WINDOWS) && !defined(CORRADE_TARGET_WINDOWS_RT))
    const Containers::Array<const char, Directory::MapDeleter> mappedFile = Directory::mapRead(Directory::join(_testDir, "dir/dummy"));
    CORRADE_COMPARE_AS(mappedFile,
        Containers::arrayView<char>({}),
        TestSuite::Compare::Container);
    #else
    CORRADE_SKIP("Not implemented on this platform.");
    #endif
}

void DirectoryTest::mapReadDirectory() {
    #if defined(CORRADE_TARGET_IOS) && defined(CORRADE_TESTSUITE_TARGET_XCTEST)
    if(!std::getenv("SIMULATOR_UDID"))
        CORRADE_SKIP("iOS (in a simulator) thinks all paths are files, can't test.");
    #endif

    #if defined(CORRADE_TARGET_UNIX) || (defined(CORRADE_TARGET_WINDOWS) && !defined(CORRADE_TARGET_WINDOWS_RT))
    std::ostringstream out;
    Error redirectError{&out};
    CORRADE_VERIFY(!Directory::mapRead(_writeTestDir));
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

void DirectoryTest::mapReadNonexistent() {
    #if defined(CORRADE_TARGET_UNIX) || (defined(CORRADE_TARGET_WINDOWS) && !defined(CORRADE_TARGET_WINDOWS_RT))
    std::ostringstream out;
    Error redirectError{&out};
    CORRADE_VERIFY(!Directory::mapRead("nonexistent"));
    CORRADE_COMPARE_AS(out.str(),
        "Utility::Path::mapRead(): can't open nonexistent: error 2 (",
        TestSuite::Compare::StringHasPrefix);
    #else
    CORRADE_SKIP("Not implemented on this platform.");
    #endif
}

void DirectoryTest::mapReadUtf8() {
    #if defined(CORRADE_TARGET_UNIX) || (defined(CORRADE_TARGET_WINDOWS) && !defined(CORRADE_TARGET_WINDOWS_RT))
    const Containers::Array<const char, Directory::MapDeleter> mappedFile = Directory::mapRead(Directory::join(_testDirUtf8, "hýždě"));
    CORRADE_COMPARE_AS(mappedFile,
        Containers::arrayView(Data),
        TestSuite::Compare::Container);
    #else
    CORRADE_SKIP("Not implemented on this platform.");
    #endif
}

void DirectoryTest::mapWrite() {
    #if defined(CORRADE_TARGET_UNIX) || (defined(CORRADE_TARGET_WINDOWS) && !defined(CORRADE_TARGET_WINDOWS_RT))
    {
        Containers::Array<char, Directory::MapDeleter> mappedFile = Directory::mapWrite(Directory::join(_writeTestDir, "mappedWriteFile"), Containers::arraySize(Data));
        CORRADE_VERIFY(mappedFile);
        CORRADE_COMPARE(mappedFile.size(), Containers::arraySize(Data));
        Utility::copy(Data, mappedFile);
    }
    CORRADE_COMPARE_AS(Directory::join(_writeTestDir, "mappedWriteFile"),
        (std::string{Data, Containers::arraySize(Data)}),
        TestSuite::Compare::FileToString);
    #else
    CORRADE_SKIP("Not implemented on this platform.");
    #endif
}

void DirectoryTest::mapWriteEmpty() {
    #if defined(CORRADE_TARGET_UNIX) || (defined(CORRADE_TARGET_WINDOWS) && !defined(CORRADE_TARGET_WINDOWS_RT))
    {
        Containers::Array<char, Directory::MapDeleter> mappedFile = Directory::mapWrite(Directory::join(_writeTestDir, "mappedWriteEmpty"), 0);
        CORRADE_COMPARE(mappedFile.size(), 0);
    }
    CORRADE_COMPARE_AS(Directory::join(_writeTestDir, "mappedWriteEmpty"),
        "",
        TestSuite::Compare::FileToString);
    #else
    CORRADE_SKIP("Not implemented on this platform.");
    #endif
}

void DirectoryTest::mapWriteDirectory() {
    #if defined(CORRADE_TARGET_UNIX) || (defined(CORRADE_TARGET_WINDOWS) && !defined(CORRADE_TARGET_WINDOWS_RT))
    std::ostringstream out;
    Error redirectError{&out};
    CORRADE_VERIFY(!Directory::mapWrite(_writeTestDir, 64));
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

void DirectoryTest::mapWriteNoPermission() {
    #if defined(CORRADE_TARGET_UNIX) || (defined(CORRADE_TARGET_WINDOWS) && !defined(CORRADE_TARGET_WINDOWS_RT))
    #ifdef CORRADE_TARGET_APPLE
    /* Assuming there's no real possibility to run as root on Apple */
    std::string filename = "/var/root/mappedFile";
    #elif defined(CORRADE_TARGET_ANDROID)
    /* Same here, would need a rooted device */
    std::string filename = "/data/local/mappedFile";
    #elif defined(CORRADE_TARGET_UNIX)
    std::string filename = "/root/mappedFile";
    if(Directory::home() == "/root")
        CORRADE_SKIP("Running under root, can't test for permissions.");
    #elif defined(CORRADE_TARGET_WINDOWS)
    /* Only the TrustedInstaller system user is supposed to have access into
       WindowsApps */
    std::string filename = "C:/Program Files/WindowsApps/mappedFile";
    #else
    #error
    #endif

    std::ostringstream out;
    Error redirectError{&out};
    CORRADE_VERIFY(!Directory::mapWrite(filename, 64));
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

void DirectoryTest::mapWriteUtf8() {
    #if defined(CORRADE_TARGET_UNIX) || (defined(CORRADE_TARGET_WINDOWS) && !defined(CORRADE_TARGET_WINDOWS_RT))
    {
        Containers::Array<char, Directory::MapDeleter> mappedFile = Directory::mapWrite(Directory::join(_writeTestDir, "hýždě chlípníka"), Containers::arraySize(Data));
        CORRADE_VERIFY(mappedFile);
        CORRADE_COMPARE(mappedFile.size(), Containers::arraySize(Data));
        Utility::copy(Data, mappedFile);
    }
    CORRADE_COMPARE_AS(Directory::join(_writeTestDir, "hýždě chlípníka"),
        (std::string{Data, Containers::arraySize(Data)}),
        TestSuite::Compare::FileToString);
    #else
    CORRADE_SKIP("Not implemented on this platform.");
    #endif
}

CORRADE_IGNORE_DEPRECATED_POP

}}}}

CORRADE_TEST_MAIN(Corrade::Utility::Test::DirectoryTest)
