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

#include "ResourceTest.h"

#include <QtTest/QTest>
#include <QtCore/QFile>

#include "Utility/Resource.h"
#include "ResourceTestConfigure.h"

QTEST_APPLESS_MAIN(Map2X::Utility::Test::ResourceTest)

using namespace std;

namespace Map2X { namespace Utility { namespace Test {

ResourceTest::ResourceTest(QObject* parent): QObject(parent) {
    /* Testing also null bytes and signed overflow, don't change binaries */

    QFile _predisposition(TESTFILES_DIR + QString("predisposition.bin"));
    QFile _consequence(TESTFILES_DIR + QString("consequence.bin"));

    _predisposition.open(QFile::ReadOnly);
    _consequence.open(QFile::ReadOnly);

    predisposition = _predisposition.readAll();
    consequence = _consequence.readAll();
}

void ResourceTest::compile() {
    Resource r("test");

    QFile compiled(TESTFILES_DIR + QString("compiled.cpp"));
    compiled.open(QFile::Text|QFile::ReadOnly);

    map<string, string> input;
    input.insert(pair<string, string>("predisposition.bin", string(predisposition.data(), predisposition.size())));
    input.insert(pair<string, string>("consequence.bin", string(consequence.data(), consequence.size())));

    QCOMPARE(QString::fromStdString(r.compile("ResourceTestData", input)), QString::fromUtf8(compiled.readAll()));
}

void ResourceTest::get() {
    Resource r("test");

    string pd = r.get("predisposition.bin");
    string cd = r.get("consequence.bin");

    QCOMPARE(QByteArray(pd.c_str(), pd.size()), predisposition);
    QCOMPARE(QByteArray(cd.c_str(), cd.size()), consequence);
}

}}}
