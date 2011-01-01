/*
    Copyright © 2007, 2008, 2009, 2010, 2011 Vladimír Vondruš <mosra@centrum.cz>

    This file is part of Kompas.

    Kompas is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License version 3
    only, as published by the Free Software Foundation.

    Kompas is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU Lesser General Public License version 3 for more details.
*/

#include "DirectoryTest.h"

#include <QtCore/QList>
#include <QtCore/QDir>
#include <QtTest/QTest>

#include "Utility/Directory.h"
#include "testConfigure.h"

using namespace std;

QTEST_APPLESS_MAIN(Kompas::Utility::Test::DirectoryTest)

namespace Kompas { namespace Utility { namespace Test {

void DirectoryTest::path_data() {
    QTest::addColumn<QString>("input");
    QTest::addColumn<QString>("expected");

    QTest::newRow("noPath") << "foo.txt" << "";
    QTest::newRow("noFilename") << ".kompas/configuration/" << ".kompas/configuration";
    QTest::newRow("regular") << "package/map.conf" << "package";
}

void DirectoryTest::path() {
    QFETCH(QString, input);
    QFETCH(QString, expected);

    string actual = Directory::path(input.toStdString());

    QCOMPARE(QString::fromStdString(actual), expected);
}

void DirectoryTest::filename_data() {
    QTest::addColumn<QString>("input");
    QTest::addColumn<QString>("expected");

    QTest::newRow("pathOnly") << "foo/bar/" << "";
    QTest::newRow("fileOnly") << "file.txt" << "file.txt";
    QTest::newRow("regular") << "foo/bar/map.conf" << "map.conf";
}

void DirectoryTest::filename() {
    QFETCH(QString, input);
    QFETCH(QString, expected);

    string actual = Directory::filename(input.toStdString());

    QCOMPARE(QString::fromStdString(actual), expected);
}

void DirectoryTest::join_data() {
    QTest::addColumn<QString>("path");
    QTest::addColumn<QString>("filename");
    QTest::addColumn<QString>("expected");

    QTest::newRow("emptyPath") << "" << "/foo.txt" << "/foo.txt";
    QTest::newRow("emptyAll") << "" << "" << "";
    QTest::newRow("absoluteFilename") << "/foo/bar" << "/file.txt" << "/file.txt";
    QTest::newRow("trailingSlash") << "/foo/bar/" << "file.txt" << "/foo/bar/file.txt";
    QTest::newRow("regular") << "/foo/bar" << "file.txt" << "/foo/bar/file.txt";
}

void DirectoryTest::join() {
    QFETCH(QString, path);
    QFETCH(QString, filename);
    QFETCH(QString, expected);

    std::string actual = Directory::join(path.toStdString(), filename.toStdString());

    QCOMPARE(QString::fromStdString(actual), expected);
}

void DirectoryTest::mkpath_data() {
    QTest::addColumn<QString>("path");

    QDir dir;
    dir.rmpath(QString::fromStdString(Directory::join(DIRECTORY_WRITE_TEST_DIR, "leaf")));
    dir.rmpath(QString::fromStdString(Directory::join(DIRECTORY_WRITE_TEST_DIR, "path/to/new/dir")));

    QTest::newRow("leaf") << QString::fromStdString(Directory::join(DIRECTORY_WRITE_TEST_DIR, "leaf"));
    QTest::newRow("path") << QString::fromStdString(Directory::join(DIRECTORY_WRITE_TEST_DIR, "path/to/new/dir"));
    QTest::newRow("existing") << DIRECTORY_WRITE_TEST_DIR;
}

void DirectoryTest::mkpath() {
    QFETCH(QString, path);

    QVERIFY(Directory::mkpath(path.toStdString()));

    QDir dir;
    QVERIFY(dir.exists(path));
}

void DirectoryTest::fileExists() {
    QVERIFY(Directory::fileExists(Directory::join(DIRECTORY_TEST_DIR, "file")));
    QVERIFY(!Directory::fileExists(Directory::join(DIRECTORY_TEST_DIR, "inexistentFile")));
}

void DirectoryTest::home() {
    QCOMPARE(QString::fromStdString(Directory::home()), QDir::homePath());
}

void DirectoryTest::configurationDir() {
    QCOMPARE(QString::fromStdString(Directory::configurationDir("Kompas", false)), QDir::home().filePath(".kompas"));
}

void DirectoryTest::list_data() {
    QTest::addColumn<int>("flags");
    QTest::addColumn<QStringList>("data");

    QStringList data;
    data << "." << ".." << "dir" << "file";
    QTest::newRow("all") << 0 << data;
    QTest::newRow("SkipSpecial") << (int) Directory::SkipSpecial << data;
    QTest::newRow("allSortedAsc") << (int) Directory::SortAscending << data;

    data.clear();
    data << "file" << "dir" << ".." << ".";
    QTest::newRow("allSortedDesc") << (int) Directory::SortDescending << data;

    data.clear();
    data << "dir" << "file";
    QTest::newRow("SkipDotAndDotDot") << (int) Directory::SkipDotAndDotDot << data;

    data.clear();
    data << "file";
    QTest::newRow("SkipDirectories") << (int) Directory::SkipDirectories << data;

    data.clear();
    data << "." << ".." << "dir";
    QTest::newRow("SkipFiles") << (int) Directory::SkipFiles << data;
}

void DirectoryTest::list() {
    QFETCH(int, flags);
    QFETCH(QStringList, data);

    Directory d(DIRECTORY_TEST_DIR, flags);
    QVERIFY(d.isLoaded());

    QStringList actual;
    for(Directory::const_iterator i = d.begin(); i != d.end(); ++i)
        actual << QString::fromStdString(*i);

    if(!(flags & (Directory::SortAscending|Directory::SortDescending)))
        actual.sort();

    QCOMPARE(actual, data);
}

}}}
