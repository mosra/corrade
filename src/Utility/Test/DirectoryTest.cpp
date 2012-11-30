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

#include "DirectoryTest.h"

#include <fstream>

#include "TestSuite/Compare/Container.h"
#include "Utility/Directory.h"

#include "testConfigure.h"

using namespace std;

CORRADE_TEST_MAIN(Corrade::Utility::Test::DirectoryTest)

namespace Corrade { namespace Utility { namespace Test {

DirectoryTest::DirectoryTest() {
    addTests(&DirectoryTest::path,
             &DirectoryTest::filename,
             &DirectoryTest::join,
             &DirectoryTest::fileExists,
             &DirectoryTest::remove,
             &DirectoryTest::mkpath,
             &DirectoryTest::home,
             &DirectoryTest::configurationDir,
             &DirectoryTest::list);
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

void DirectoryTest::fileExists() {
    /* File */
    CORRADE_VERIFY(Directory::fileExists(Directory::join(DIRECTORY_TEST_DIR, "file")));

    /* Directory */
    CORRADE_VERIFY(Directory::fileExists(DIRECTORY_TEST_DIR));

    /* Inexistent file */
    CORRADE_VERIFY(!Directory::fileExists(Directory::join(DIRECTORY_TEST_DIR, "inexistentFile")));
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

    /* Inexistent file */
    std::string inexistent = Directory::join(DIRECTORY_WRITE_TEST_DIR, "inexistent");
    CORRADE_VERIFY(!Directory::fileExists(inexistent));
    CORRADE_VERIFY(!Directory::rm(inexistent));
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
    {
        /* All */
        Directory d(DIRECTORY_TEST_DIR);
        CORRADE_VERIFY(d.isLoaded());
        CORRADE_COMPARE_WITH(d, (std::vector<std::string>{".", "..", "dir", "file"}),
            TestSuite::Compare::Container<std::vector<std::string>>(TestSuite::Compare::ContainerMethod::Sorted));
    } {
        /* Skip special */
        Directory d(DIRECTORY_TEST_DIR, Directory::SkipSpecial);
        CORRADE_VERIFY(d.isLoaded());
        CORRADE_COMPARE_WITH(d, (std::vector<std::string>{".", "..", "dir", "file"}),
            TestSuite::Compare::Container<std::vector<std::string>>(TestSuite::Compare::ContainerMethod::Sorted));
    } {
        /* All, sorted ascending */
        Directory d(DIRECTORY_TEST_DIR, Directory::SortAscending);
        CORRADE_VERIFY(d.isLoaded());
        CORRADE_COMPARE_AS(d, (std::vector<std::string>{".", "..", "dir", "file"}),
            TestSuite::Compare::Container);
    } {
        /* All, sorted descending */
        Directory d(DIRECTORY_TEST_DIR, Directory::SortDescending);
        CORRADE_VERIFY(d.isLoaded());
        CORRADE_COMPARE_AS(d, (std::vector<std::string>{"file", "dir", "..", "."}),
            TestSuite::Compare::Container);
    } {
        /* Skip . and .. */
        Directory d(DIRECTORY_TEST_DIR, Directory::SkipDotAndDotDot);
        CORRADE_VERIFY(d.isLoaded());
        CORRADE_COMPARE_WITH(d, (std::vector<std::string>{"dir", "file"}),
            TestSuite::Compare::Container<std::vector<std::string>>(TestSuite::Compare::ContainerMethod::Sorted));
    } {
        /* Skip directories */
        Directory d(DIRECTORY_TEST_DIR, Directory::SkipDirectories);
        CORRADE_VERIFY(d.isLoaded());
        CORRADE_COMPARE_WITH(d, std::vector<std::string>{"file"},
            TestSuite::Compare::Container<std::vector<std::string>>(TestSuite::Compare::ContainerMethod::Sorted));
    } {
        /* Skip files */
        Directory d(DIRECTORY_TEST_DIR, Directory::SkipFiles);
        CORRADE_VERIFY(d.isLoaded());
        CORRADE_COMPARE_WITH(d, (std::vector<std::string>{".", "..", "dir"}),
            TestSuite::Compare::Container<std::vector<std::string>>(TestSuite::Compare::ContainerMethod::Sorted));
    }
}

}}}
