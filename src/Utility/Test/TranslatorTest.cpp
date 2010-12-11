/*
    Copyright © 2007, 2008, 2009, 2010 Vladimír Vondruš <mosra@centrum.cz>

    This file is part of Kompas.

    Kompas is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License version 3
    only, as published by the Free Software Foundation.

    Kompas is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU Lesser General Public License version 3 for more details.
*/

#include "TranslatorTest.h"

#include <QtTest/QTest>

#include "Utility/Translator.h"
#include "Utility/Directory.h"
#include "testConfigure.h"

using namespace std;

QTEST_APPLESS_MAIN(Kompas::Utility::Test::TranslatorTest)

namespace Kompas { namespace Utility { namespace Test {

void TranslatorTest::file() {
    Translator t(Directory::join(TRANSLATOR_TEST_DIR, "primary.conf"), Directory::join(TRANSLATOR_TEST_DIR, "fallback.conf"));
    const string* s = t.get("string");

    QVERIFY(*s == "primarily default translated");

    /* Load another primary localization */
    t.setPrimary(Directory::join(TRANSLATOR_TEST_DIR, "en_US.conf"));

    QVERIFY(*s == "primarily translated");

    /* Cleanup primary localization */
    t.setPrimary(0);
    QVERIFY(*s == "fallback translation");

    /* Load inexistent primary localization */
    t.setPrimary(Directory::join(TRANSLATOR_TEST_DIR, "inexistent.conf"));
    QVERIFY(*s == "fallback translation");

    /* Load another fallback localization */
    t.setFallback(Directory::join(TRANSLATOR_TEST_DIR, "fallback2.conf"));
    QVERIFY(*s == "other fallback translation");

    /* Cleanup fallback localization */
    t.setFallback(0);
    QVERIFY(*s == "");
}

void TranslatorTest::group() {
    Configuration c(Directory::join(TRANSLATOR_TEST_DIR, "primary.conf"), Configuration::ReadOnly);

    Translator t(&c);

    const string* s = t.get("string");

    QVERIFY(*s == "primarily default translated");

    /* Load another group */
    t.setPrimary(c.group("cs_CZ"));
    QVERIFY(*s == "primárně přeloženo");
}

void TranslatorTest::dynamic() {
    Configuration c(Directory::join(TRANSLATOR_TEST_DIR, "primary.conf"), Configuration::ReadOnly);
    Translator t1(Directory::join(TRANSLATOR_TEST_DIR, "#.conf"));
    Translator t2;
    t2.setPrimary(&c, true);

    const string* s1 = t1.get("string");
    const string* s2 = t2.get("string");

    Translator::setLocale("en_US");

    QVERIFY(*s1 == "primarily translated");
    QVERIFY(*s2 == "primarily translated");

    Translator::setLocale("cs_CZ");

    QVERIFY(*s1 == "primárně přeloženo");
    QVERIFY(*s2 == "primárně přeloženo");

    /* Fixed translations, not affected with setLocale() */
    t1.setPrimary(Directory::join(TRANSLATOR_TEST_DIR, "cs_CZ.conf"));
    t2.setPrimary(c.group("cs_CZ"));

    Translator::setLocale("en_US");

    QVERIFY(*s1 == "primárně přeloženo");
    QVERIFY(*s2 == "primárně přeloženo");
}

}}}
