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

#include "TestSuite/Tester.h"
#include "Utility/Configuration.h"
#include "Utility/Directory.h"
#include "Utility/Translator.h"

#include "testConfigure.h"

namespace Corrade { namespace Utility { namespace Test {

class TranslatorTest: public TestSuite::Tester {
    public:
        TranslatorTest();

        void file();
        void group();
        void dynamic();
};

TranslatorTest::TranslatorTest() {
    addTests({&TranslatorTest::file,
              &TranslatorTest::group,
              &TranslatorTest::dynamic});
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
    #ifndef CORRADE_GCC44_COMPATIBILITY
    CORRADE_COMPARE(*s, u8"primárně přeloženo");
    #else
    CORRADE_COMPARE(*s, "primárně přeloženo");
    #endif
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

    #ifndef CORRADE_GCC44_COMPATIBILITY
    CORRADE_COMPARE(*s1, u8"primárně přeloženo");
    CORRADE_COMPARE(*s2, u8"primárně přeloženo");
    #else
    CORRADE_COMPARE(*s1, "primárně přeloženo");
    CORRADE_COMPARE(*s2, "primárně přeloženo");
    #endif

    /* Fixed translations, not affected with setLocale() */
    t1.setPrimary(Directory::join(TRANSLATOR_TEST_DIR, "cs_CZ.conf"));
    t2.setPrimary(c.group("cs_CZ"));

    Translator::setLocale("en_US");

    #ifndef CORRADE_GCC44_COMPATIBILITY
    CORRADE_COMPARE(*s1, u8"primárně přeloženo");
    CORRADE_COMPARE(*s2, u8"primárně přeloženo");
    #else
    CORRADE_COMPARE(*s1, "primárně přeloženo");
    CORRADE_COMPARE(*s2, "primárně přeloženo");
    #endif
}

}}}

CORRADE_TEST_MAIN(Corrade::Utility::Test::TranslatorTest)
