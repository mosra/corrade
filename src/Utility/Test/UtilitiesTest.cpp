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

#include "UtilitiesTest.h"

#include <string>
#include <QtTest/QTest>

#include "Utility/utilities.h"

using namespace std;

QTEST_APPLESS_MAIN(Kompas::Utility::Test::UtilitiesTest)

namespace Kompas { namespace Utility { namespace Test {

void UtilitiesTest::pow2() {
    QVERIFY(Utility::pow2(10) == 1024);
}

void UtilitiesTest::trim_data() {
    QTest::addColumn<QString>("in");
    QTest::addColumn<QString>("out");

    QTest::newRow("spaces at the end") << "abc  " << "abc";
    QTest::newRow("spaces at beginning") << "  abc" << "abc";
    QTest::newRow("no spaces") << "abc" << "abc";
    QTest::newRow("all spaces") << "    " << "";
    QTest::newRow("different kinds of spaces") << "\t\r\n\f\v " << "";
}

void UtilitiesTest::trim() {
    QFETCH(QString, in);
    QFETCH(QString, out);

    QCOMPARE(QString::fromStdString(Utility::trim(in.toStdString())), out);
}

void UtilitiesTest::split_data() {
    QTest::addColumn<QString>("in");
    QTest::addColumn<bool>("keepEmptyParts");
    QTest::addColumn<QStringList>("out");

    QStringList list;
    list << "abcdef";
    QTest::newRow("noDelimiter") << "abcdef" << true << list;

    list.clear();
    list << "ab" << "c" << "def";
    QTest::newRow("delimiters") << "ab/c/def" << true << list;

    list.clear();
    list << "ab" << "" << "c" << "def" << "" << "";
    QTest::newRow("emptyParts") << "ab//c/def//" << true << list;

    list.clear();
    list << "ab" << "c" << "def";
    QTest::newRow("skipEmptyParts") << "ab//c/def//" << false << list;
}

void UtilitiesTest::split() {
    QFETCH(QString, in);
    QFETCH(bool, keepEmptyParts);
    QFETCH(QStringList, out);

    vector<string> o = Utility::split(in.toStdString(), '/', keepEmptyParts);

    QStringList o_;
    for(vector<string>::const_iterator it = o.begin(); it != o.end(); ++it)
        o_.append(QString::fromStdString(*it));

    QCOMPARE(o_, out);
}

void UtilitiesTest::lowercase_data() {
    QTest::addColumn<QString>("in");
    QTest::addColumn<QString>("out");

    QTest::newRow("lowercase") << "hello" << "hello";
    QTest::newRow("uppercase") << "QWERTZUIOP" << "qwertzuiop";
    QTest::newRow("special chars") << ".,?- \"!/(98765%" << ".,?- \"!/(98765%";
    QTest::newRow("UTF-8") << "ĚŠČŘŽÝÁÍÉÚŮĎŤŇ" << "ěščřžýáíéúůďťň";
}

void UtilitiesTest::lowercase() {
    QFETCH(QString, in);
    QFETCH(QString, out);

    QString actual = QString::fromStdString(Utility::lowercase(in.toStdString()));

    QEXPECT_FAIL("UTF-8", "UTF-8 lowercasing is not supported", Continue);
    QCOMPARE(out, actual);
}

}}}
