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

#include "TranslatorTest.h"

#include "Utility/Translator.h"
#include "Utility/Directory.h"

#include "testConfigure.h"

CORRADE_TEST_MAIN(Corrade::Utility::Test::TranslatorTest)

namespace Corrade { namespace Utility { namespace Test {

TranslatorTest::TranslatorTest() {
    addTests(&TranslatorTest::file,
             &TranslatorTest::group,
             &TranslatorTest::dynamic);
}

void TranslatorTest::file() {
    Translator t(Directory::join(TRANSLATOR_TEST_DIR, "primary.conf"), Directory::join(TRANSLATOR_TEST_DIR, "fallback.conf"));
    const std::string* s = t.get("string");

    CORRADE_COMPARE(*s, "primarily default translated");

    /* Load another primary localization */
    t.setPrimary(Directory::join(TRANSLATOR_TEST_DIR, "en_US.conf"));

    CORRADE_COMPARE(*s, "primarily translated");

    /* Cleanup primary localization */
    t.setPrimary(nullptr);
    CORRADE_COMPARE(*s, "fallback translation");

    /* Load inexistent primary localization */
    t.setPrimary(Directory::join(TRANSLATOR_TEST_DIR, "inexistent.conf"));
    CORRADE_COMPARE(*s, "fallback translation");

    /* Load another fallback localization */
    t.setFallback(Directory::join(TRANSLATOR_TEST_DIR, "fallback2.conf"));
    CORRADE_COMPARE(*s, "other fallback translation");

    /* Cleanup fallback localization */
    t.setFallback(nullptr);
    CORRADE_VERIFY(s->empty());
}

void TranslatorTest::group() {
    Configuration c(Directory::join(TRANSLATOR_TEST_DIR, "primary.conf"), Configuration::Flag::ReadOnly);

    Translator t(&c);

    const std::string* s = t.get("string");

    CORRADE_COMPARE(*s, "primarily default translated");

    /* Load another group */
    t.setPrimary(c.group("cs_CZ"));
    CORRADE_COMPARE(*s, u8"primárně přeloženo");
}

void TranslatorTest::dynamic() {
    Configuration c(Directory::join(TRANSLATOR_TEST_DIR, "primary.conf"), Configuration::Flag::ReadOnly);
    Translator t1(Directory::join(TRANSLATOR_TEST_DIR, "#.conf"));
    Translator t2;
    t2.setPrimary(&c, true);

    const std::string* s1 = t1.get("string");
    const std::string* s2 = t2.get("string");

    Translator::setLocale("en_US");

    CORRADE_COMPARE(*s1, "primarily translated");
    CORRADE_COMPARE(*s2, "primarily translated");

    Translator::setLocale("cs_CZ");

    CORRADE_COMPARE(*s1, u8"primárně přeloženo");
    CORRADE_COMPARE(*s2, u8"primárně přeloženo");

    /* Fixed translations, not affected with setLocale() */
    t1.setPrimary(Directory::join(TRANSLATOR_TEST_DIR, "cs_CZ.conf"));
    t2.setPrimary(c.group("cs_CZ"));

    Translator::setLocale("en_US");

    CORRADE_COMPARE(*s1, u8"primárně přeloženo");
    CORRADE_COMPARE(*s2, u8"primárně přeloženo");
}

}}}
