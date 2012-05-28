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

#include "ResourceTest.h"

#include <sstream>
#include <QtCore/QFile>
#include <QtTest/QTest>

#include "Utility/Debug.h"
#include "Utility/Resource.h"
#include "testConfigure.h"

QTEST_APPLESS_MAIN(Corrade::Utility::Test::ResourceTest)

using namespace std;

namespace Corrade { namespace Utility { namespace Test {

ResourceTest::ResourceTest(QObject* parent): QObject(parent) {
    /* Testing also null bytes and signed overflow, don't change binaries */

    QFile _predisposition(RESOURCE_TEST_DIR + QString("predisposition.bin"));
    QFile _consequence(RESOURCE_TEST_DIR + QString("consequence.bin"));

    _predisposition.open(QFile::ReadOnly);
    _consequence.open(QFile::ReadOnly);

    predisposition = _predisposition.readAll();
    consequence = _consequence.readAll();
}

void ResourceTest::compile() {
    Resource r("test");

    QFile compiled(RESOURCE_TEST_DIR + QString("compiled.cpp"));
    compiled.open(QFile::Text|QFile::ReadOnly);

    map<string, string> input;
    input.insert(make_pair("predisposition.bin", string(predisposition.data(), predisposition.size())));
    input.insert(make_pair("consequence.bin", string(consequence.data(), consequence.size())));

    QCOMPARE(QString::fromStdString(r.compile("ResourceTestData", input)), QString::fromUtf8(compiled.readAll()));
}

void ResourceTest::get() {
    Resource r("test");

    string pd = r.get("predisposition.bin");
    string cd = r.get("consequence.bin");

    QCOMPARE(QByteArray(pd.c_str(), pd.size()), predisposition);
    QCOMPARE(QByteArray(cd.c_str(), cd.size()), consequence);
}

void ResourceTest::getInexistent() {
    ostringstream out;
    Error::setOutput(&out);

    {
        Resource r("inexistentGroup");
        QVERIFY(r.get("inexistentFile").empty());
        QVERIFY(out.str() == "Resource: group 'inexistentGroup' was not found\n");
    }

    out.str("");

    {
        Resource r("test");
        QVERIFY(r.get("inexistentFile").empty());
        QVERIFY(out.str() == "Resource: file 'inexistentFile' was not found in group 'test'\n");
    }
}

}}}
