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

#include "TestSuite/Tester.h"
#include "TestSuite/Compare/Container.h"
#include "Utility/Directory.h"

#include "testConfigure.h"

namespace Corrade { namespace Utility { namespace Test {

class DirectoryTest: public Corrade::TestSuite::Tester {
    public:
        DirectoryTest();

        void path();
        void filename();
        void join();
        #ifdef _WIN32
        void joinWindows();
        #endif
        void fileExists();
        void remove();
        void moveFile();
        void moveDirectory();
        void mkpath();
        void home();
        void configurationDir();
        void list();
        void listSortPrecedence();
};

DirectoryTest::DirectoryTest() {
    addTests({&DirectoryTest::path,
              &DirectoryTest::filename,
              &DirectoryTest::join,
              #ifdef _WIN32
              &DirectoryTest::joinWindows,
              #endif
              &DirectoryTest::fileExists,
              &DirectoryTest::remove,
              &DirectoryTest::moveFile,
              &DirectoryTest::moveDirectory,
              &DirectoryTest::mkpath,
              &DirectoryTest::home,
              &DirectoryTest::configurationDir,
              &DirectoryTest::list,
              &DirectoryTest::listSortPrecedence});
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

#ifdef _WIN32
void DirectoryTest::joinWindows() {
    /* Drive letter */
    CORRADE_COMPARE(Directory::join("/foo/bar", "X:/path/file.txt"), "X:/path/file.txt");
}
#endif

void DirectoryTest::fileExists() {
    /* File */
    CORRADE_VERIFY(Directory::fileExists(Directory::join(DIRECTORY_TEST_DIR, "file")));

    /* Directory */
    CORRADE_VERIFY(Directory::fileExists(DIRECTORY_TEST_DIR));

    /* Nonexistent file */
    CORRADE_VERIFY(!Directory::fileExists(Directory::join(DIRECTORY_TEST_DIR, "nonexistentFile")));
}

void DirectoryTest::remove() {
    /* Directory */
    std::string directory = Directory::join(DIRECTORY_WRITE_TEST_DIR, "directory");
    CORRADE_VERIFY(Directory::mkpath(directory));
    CORRADE_VERIFY(Directory::rm(directory));
    CORRADE_VERIFY(!Directory::fileExists(directory));

    /* File */
    std::string file = Directory::join(DIRECTORY_WRITE_TEST_DIR, "file.txt");
    {
        std::ofstream out(file);
        CORRADE_VERIFY(out.good());
        out.put('a');
    }
    CORRADE_VERIFY(Directory::rm(file));
    CORRADE_VERIFY(!Directory::fileExists(file));

    /* Nonexistent file */
    std::string nonexistent = Directory::join(DIRECTORY_WRITE_TEST_DIR, "nonexistent");
    CORRADE_VERIFY(!Directory::fileExists(nonexistent));
    CORRADE_VERIFY(!Directory::rm(nonexistent));
}

void DirectoryTest::moveFile() {
    /* Old file, create if not exists */
    std::string oldFile = Directory::join(DIRECTORY_WRITE_TEST_DIR, "oldFile.txt");
    if(!Directory::fileExists(oldFile)) {
        std::ofstream out(oldFile);
        CORRADE_VERIFY(out.good());
        out.put('a');
    }

    /* New file, remove if exists */
    std::string newFile = Directory::join(DIRECTORY_WRITE_TEST_DIR, "newFile.txt");
    if(Directory::fileExists(newFile))
        CORRADE_VERIFY(Directory::rm(newFile));

    CORRADE_VERIFY(Directory::move(oldFile, newFile));
    CORRADE_VERIFY(!Directory::fileExists(oldFile));
    CORRADE_VERIFY(Directory::fileExists(newFile));
}

void DirectoryTest::moveDirectory() {
    /* Old directory, create if not exists */
    std::string oldDirectory = Directory::join(DIRECTORY_WRITE_TEST_DIR, "oldDirectory");
    if(!Directory::fileExists(oldDirectory))
        CORRADE_VERIFY(Directory::mkpath(oldDirectory));

    /* New directory, remove if exists */
    std::string newDirectory = Directory::join(DIRECTORY_WRITE_TEST_DIR, "newDirectory");
    if(Directory::fileExists(newDirectory))
        CORRADE_VERIFY(Directory::rm(newDirectory));

    CORRADE_VERIFY(Directory::move(oldDirectory, newDirectory));
    CORRADE_VERIFY(!Directory::fileExists(oldDirectory));
    CORRADE_VERIFY(Directory::fileExists(newDirectory));
}

void DirectoryTest::mkpath() {
    /* Existing */
    CORRADE_VERIFY(Directory::fileExists(DIRECTORY_WRITE_TEST_DIR));
    CORRADE_VERIFY(Directory::mkpath(DIRECTORY_WRITE_TEST_DIR));

    /* Leaf */
    std::string leaf = Directory::join(DIRECTORY_WRITE_TEST_DIR, "leaf");
    if(Directory::fileExists(leaf)) CORRADE_VERIFY(Directory::rm(leaf));
    CORRADE_VERIFY(Directory::mkpath(leaf));
    CORRADE_VERIFY(Directory::fileExists(leaf));

    /* Path */
    std::string path = Directory::join(DIRECTORY_WRITE_TEST_DIR, "path/to/new/dir");
    if(Directory::fileExists(path)) CORRADE_VERIFY(Directory::rm(path));
    if(Directory::fileExists(Directory::join(DIRECTORY_WRITE_TEST_DIR, "path/to/new")))
        CORRADE_VERIFY(Directory::rm(Directory::join(DIRECTORY_WRITE_TEST_DIR, "path/to/new")));
    if(Directory::fileExists(Directory::join(DIRECTORY_WRITE_TEST_DIR, "path/to")))
        CORRADE_VERIFY(Directory::rm(Directory::join(DIRECTORY_WRITE_TEST_DIR, "path/to")));
    if(Directory::fileExists(Directory::join(DIRECTORY_WRITE_TEST_DIR, "path")))
        CORRADE_VERIFY(Directory::rm(Directory::join(DIRECTORY_WRITE_TEST_DIR, "path")));

    CORRADE_VERIFY(Directory::mkpath(leaf));
    CORRADE_VERIFY(Directory::fileExists(leaf));
}

void DirectoryTest::home() {
    CORRADE_EXPECT_FAIL("Don't know how to properly test without another well tested framework.");
    CORRADE_COMPARE(Directory::home(), ""/*QDir::homePath()*/);
}

void DirectoryTest::configurationDir() {
    CORRADE_EXPECT_FAIL("Don't know how to properly test without another well tested framework.");
    CORRADE_COMPARE(Directory::configurationDir("Corrade", false), ""/*QDir::home().filePath(".corrade")*/);
}

void DirectoryTest::list() {
    /* All */
    CORRADE_COMPARE_AS(Directory::list(DIRECTORY_TEST_DIR),
        (std::vector<std::string>{".", "..", "dir", "file"}),
        TestSuite::Compare::SortedContainer);

    /* Skip special */
    CORRADE_COMPARE_AS(Directory::list(DIRECTORY_TEST_DIR, Directory::Flag::SkipSpecial),
        (std::vector<std::string>{".", "..", "dir", "file"}),
        TestSuite::Compare::SortedContainer);

    /* All, sorted ascending */
    CORRADE_COMPARE_AS(Directory::list(DIRECTORY_TEST_DIR, Directory::Flag::SortAscending),
        (std::vector<std::string>{".", "..", "dir", "file"}),
        TestSuite::Compare::Container);

    /* All, sorted descending */
    CORRADE_COMPARE_AS(Directory::list(DIRECTORY_TEST_DIR, Directory::Flag::SortDescending),
        (std::vector<std::string>{"file", "dir", "..", "."}),
        TestSuite::Compare::Container);

    /* Skip . and .. */
    CORRADE_COMPARE_AS(Directory::list(DIRECTORY_TEST_DIR, Directory::Flag::SkipDotAndDotDot),
        (std::vector<std::string>{"dir", "file"}),
        TestSuite::Compare::SortedContainer);

    /* Skip directories */
    CORRADE_COMPARE_AS(Directory::list(DIRECTORY_TEST_DIR, Directory::Flag::SkipDirectories),
        std::vector<std::string>{"file"},
        TestSuite::Compare::SortedContainer);

    /* Skip files */
    CORRADE_COMPARE_AS(Directory::list(DIRECTORY_TEST_DIR, Directory::Flag::SkipFiles),
        (std::vector<std::string>{".", "..", "dir"}),
        TestSuite::Compare::SortedContainer);

}

void DirectoryTest::listSortPrecedence() {
    CORRADE_VERIFY((Directory::Flag::SortAscending|Directory::Flag::SortDescending) == Directory::Flag::SortAscending);
}

}}}

CORRADE_TEST_MAIN(Corrade::Utility::Test::DirectoryTest)
