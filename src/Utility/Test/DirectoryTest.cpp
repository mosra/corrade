/*
    Copyright © 2007, 2008, 2009, 2010 Vladimír Vondruš <mosra@centrum.cz>

    This file is part of Map2X.

    Map2X is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License version 3
    only, as published by the Free Software Foundation.

    Map2X is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU Lesser General Public License version 3 for more details.
*/

#include "DirectoryTest.h"

#include <QtCore/QList>
#include <QtTest/QTest>

#include "Utility/Directory.h"
#include "DirectoryTestConfigure.h"

using namespace std;

QTEST_APPLESS_MAIN(Map2X::Utility::Test::DirectoryTest)

namespace Map2X { namespace Utility { namespace Test {

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

    Directory d(TESTFILES_DIR, flags);
    QVERIFY(d.isLoaded());

    QStringList actual;
    for(Directory::const_iterator i = d.begin(); i != d.end(); ++i)
        actual << QString::fromStdString(*i);

    if(!(flags & (Directory::SortAscending|Directory::SortDescending)))
        actual.sort();

    QCOMPARE(actual, data);
}

}}}
