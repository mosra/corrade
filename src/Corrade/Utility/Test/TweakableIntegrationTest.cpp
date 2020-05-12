/*
    This file is part of Corrade.

    Copyright © 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016,
                2017, 2018, 2019, 2020 Vladimír Vondruš <mosra@centrum.cz>

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

#include <sstream>

#include "Corrade/TestSuite/Tester.h"
#include "Corrade/Utility/Directory.h"
#include "Corrade/Utility/DebugStl.h" /** @todo remove when <sstream> is gone */
#include "Corrade/Utility/Format.h"
#include "Corrade/Utility/FormatStl.h"
#include "Corrade/Utility/String.h"
#include "Corrade/Utility/System.h"
#include "Corrade/Utility/Tweakable.h"

#include "configure.h"

#define _ CORRADE_TWEAKABLE

namespace Corrade { namespace Utility { namespace Test { namespace {

struct TweakableIntegrationTest: TestSuite::Tester {
    explicit TweakableIntegrationTest();

    void setup();
    void teardown();

    void variable();
    void scopeTemplated();
    void scopeVoid();
    void updateNoChange();
    void updateUnexpectedLine();
    void updateDifferentType();
    void updateParseError();
    void updateNoAlias();

    private:
        std::string _thisWriteableFile, _thisReadablePath;
};

struct {
    const char* name;
    bool enabled;
} EnabledData[] {
    {"disabled", false},
    {"enabled", true}
};

TweakableIntegrationTest::TweakableIntegrationTest() {
    addInstancedTests({&TweakableIntegrationTest::variable,
                       &TweakableIntegrationTest::scopeTemplated,
                       &TweakableIntegrationTest::scopeVoid},
                      Containers::arraySize(EnabledData),
                      &TweakableIntegrationTest::setup,
                      &TweakableIntegrationTest::teardown);

    addTests({&TweakableIntegrationTest::updateNoChange,
              &TweakableIntegrationTest::updateUnexpectedLine,
              &TweakableIntegrationTest::updateDifferentType,
              &TweakableIntegrationTest::updateParseError,
              &TweakableIntegrationTest::updateNoAlias},
             &TweakableIntegrationTest::setup,
             &TweakableIntegrationTest::teardown);

    Directory::mkpath(TWEAKABLE_WRITE_TEST_DIR);
    _thisWriteableFile = Directory::join(TWEAKABLE_WRITE_TEST_DIR, "TweakableIntegrationTest.cpp");
    _thisReadablePath = Directory::path(Directory::fromNativeSeparators(__FILE__));
}

void TweakableIntegrationTest::setup() {
    Directory::writeString(_thisWriteableFile,
        Directory::readString(
            Directory::join(TWEAKABLE_TEST_DIR, "TweakableIntegrationTest.cpp")));
}

void TweakableIntegrationTest::teardown() {
    Directory::rm(_thisWriteableFile);
}

/* This is outside so we can trigger the updates from other functions later */
char foo() { return _('a'); /* now this */ }

void TweakableIntegrationTest::variable() {
    auto&& data = EnabledData[testCaseInstanceId()];
    setTestCaseDescription(data.name);

    CORRADE_VERIFY(Directory::exists(_thisWriteableFile));

    Tweakable tweakable;
    if(data.enabled)
        tweakable.enable(_thisReadablePath, TWEAKABLE_WRITE_TEST_DIR);

    CORRADE_COMPARE(tweakable.isEnabled(), data.enabled);

    char c;

    {
        std::ostringstream out;
        Debug redirectOutput{&out};
        c = foo();
        if(data.enabled)
            CORRADE_COMPARE(out.str(), formatString("Utility::Tweakable: watching for changes in {}\n", _thisWriteableFile));
        else
            CORRADE_COMPARE(out.str(), "");
    }

    CORRADE_COMPARE(tweakable.update(), TweakableState::NoChange);

    /* No change yet */
    CORRADE_COMPARE(c, 'a');

    /* FileWatcher crutch. See its test for more info. */
    /** @todo get rid of this once proper FS inode etc. watching is implemented */
    #if defined(CORRADE_TARGET_APPLE) || defined(CORRADE_TARGET_WINDOWS) || defined(CORRADE_TARGET_EMSCRIPTEN)
    Utility::System::sleep(1100);
    #else
    Utility::System::sleep(10);
    #endif

    /* Replace the above line with a different number */
    CORRADE_VERIFY(Directory::writeString(_thisWriteableFile,
        String::replaceFirst(Directory::readString(_thisWriteableFile),
            "_('a'); /* now this */",
            "_('X'); /* now this */")));

    /* Now it changes, if enabled */
    {
        std::ostringstream out;
        Debug redirectOutput{&out};
        Warning redirectWarning{&out};
        TweakableState state = tweakable.update();

        if(data.enabled) {
            CORRADE_COMPARE(out.str(), formatString(
"Utility::Tweakable::update(): looking for updated _() macros in {0}\n"
"Utility::Tweakable::update(): updating _('X') in {0}:102\n"
"Utility::Tweakable::update(): ignoring unknown new value _(42.0f) in {0}:185\n"
"Utility::Tweakable::update(): ignoring unknown new value _(22.7f) in {0}:251\n", __FILE__));
            CORRADE_COMPARE(state, TweakableState::Success);
        } else {
            CORRADE_COMPARE(out.str(), "");
            CORRADE_COMPARE(state, TweakableState::NoChange);
        }
    }

    CORRADE_COMPARE(foo(), data.enabled ? 'X' : 'a');
}

void TweakableIntegrationTest::scopeTemplated() {
    auto&& data = EnabledData[testCaseInstanceId()];
    setTestCaseDescription(data.name);

    CORRADE_VERIFY(Directory::exists(_thisWriteableFile));

    Tweakable tweakable;
    if(data.enabled)
        tweakable.enable(_thisReadablePath, TWEAKABLE_WRITE_TEST_DIR);

    float f = 0;
    {
        std::ostringstream out;
        Debug redirectOutput{&out};
        tweakable.scope([](float& f){
            f = _(42.0f); /* yes this */
        }, f);
        if(data.enabled)
            CORRADE_COMPARE(out.str(), formatString("Utility::Tweakable: watching for changes in {}\n", _thisWriteableFile));
        else
            CORRADE_COMPARE(out.str(), "");
    }

    CORRADE_COMPARE(tweakable.update(), TweakableState::NoChange);

    /* No change yet */
    CORRADE_COMPARE(f, 42.0f);

    /* FileWatcher crutch. See its test for more info. */
    /** @todo get rid of this once proper FS inode etc. watching is implemented */
    #if defined(CORRADE_TARGET_APPLE) || defined(CORRADE_TARGET_WINDOWS) || defined(CORRADE_TARGET_EMSCRIPTEN)
    Utility::System::sleep(1100);
    #else
    Utility::System::sleep(10);
    #endif

    /* Replace the above line with a different number */
    CORRADE_VERIFY(Directory::writeString(_thisWriteableFile,
        String::replaceFirst(Directory::readString(_thisWriteableFile),
            "_(42.0f); /* yes this */",
            "_(133.7f); /* yes this */")));

    /* Now it changes, if enabled */
    {
        std::ostringstream out;
        Debug redirectOutput{&out};
        Warning redirectWarning{&out};
        TweakableState state = tweakable.update();

        if(data.enabled) {
            CORRADE_COMPARE(out.str(), formatString(
"Utility::Tweakable::update(): looking for updated _() macros in {0}\n"
"Utility::Tweakable::update(): ignoring unknown new value _('a') in {0}:102\n"
"Utility::Tweakable::update(): updating _(133.7f) in {0}:185\n"
"Utility::Tweakable::update(): ignoring unknown new value _(22.7f) in {0}:251\n"
"Utility::Tweakable::update(): 1 scopes affected\n", __FILE__));
            CORRADE_COMPARE(state, TweakableState::Success);
        } else {
            CORRADE_COMPARE(out.str(), "");
            CORRADE_COMPARE(state, TweakableState::NoChange);
        }
    }

    CORRADE_COMPARE(f, data.enabled ? 133.7f : 42.0f);
}

void TweakableIntegrationTest::scopeVoid() {
    auto&& data = EnabledData[testCaseInstanceId()];
    setTestCaseDescription(data.name);

    CORRADE_VERIFY(Directory::exists(_thisWriteableFile));

    Tweakable tweakable;
    if(data.enabled)
        tweakable.enable(_thisReadablePath, TWEAKABLE_WRITE_TEST_DIR);

    float f = 0.0f;
    {
        std::ostringstream out;
        Debug redirectOutput{&out};
        tweakable.scope([](void* f){
            *static_cast<float*>(f) = _(22.7f); /* and finally */
        }, &f);
        if(data.enabled)
            CORRADE_COMPARE(out.str(), formatString("Utility::Tweakable: watching for changes in {}\n", _thisWriteableFile));
        else
            CORRADE_COMPARE(out.str(), "");
    }

    CORRADE_COMPARE(tweakable.update(), TweakableState::NoChange);

    /* No change yet */
    CORRADE_COMPARE(f, 22.7f);

    /* FileWatcher crutch. See its test for more info. */
    /** @todo get rid of this once proper FS inode etc. watching is implemented */
    #if defined(CORRADE_TARGET_APPLE) || defined(CORRADE_TARGET_WINDOWS) || defined(CORRADE_TARGET_EMSCRIPTEN)
    Utility::System::sleep(1100);
    #else
    Utility::System::sleep(10);
    #endif

    /* Replace the above line with a different number */
    CORRADE_VERIFY(Directory::writeString(_thisWriteableFile,
        String::replaceFirst(Directory::readString(_thisWriteableFile),
            "_(22.7f); /* and finally */",
            "_(-1.44f); /* and finally */")));

    /* Now it changes, if enabled */
    {
        std::ostringstream out;
        Debug redirectOutput{&out};
        Warning redirectWarning{&out};
        TweakableState state = tweakable.update();

        if(data.enabled) {
            CORRADE_COMPARE(out.str(), formatString(
"Utility::Tweakable::update(): looking for updated _() macros in {0}\n"
"Utility::Tweakable::update(): ignoring unknown new value _('a') in {0}:102\n"
"Utility::Tweakable::update(): ignoring unknown new value _(42.0f) in {0}:185\n"
"Utility::Tweakable::update(): updating _(-1.44f) in {0}:251\n"
"Utility::Tweakable::update(): 1 scopes affected\n", __FILE__));
            CORRADE_COMPARE(state, TweakableState::Success);
        } else {
            CORRADE_COMPARE(out.str(), "");
            CORRADE_COMPARE(state, TweakableState::NoChange);
        }
    }

    CORRADE_COMPARE(f, data.enabled ? -1.44f : 22.7f);
}

void TweakableIntegrationTest::updateNoChange() {
    CORRADE_VERIFY(Directory::exists(_thisWriteableFile));

    Tweakable tweakable;
    tweakable.enable(_thisReadablePath, TWEAKABLE_WRITE_TEST_DIR);

    /* Trigger watching of this file by executing annotated literal */
    foo();

    /* FileWatcher crutch. See its test for more info. */
    /** @todo get rid of this once proper FS inode etc. watching is implemented */
    #if defined(CORRADE_TARGET_APPLE) || defined(CORRADE_TARGET_WINDOWS) || defined(CORRADE_TARGET_EMSCRIPTEN)
    Utility::System::sleep(1100);
    #else
    Utility::System::sleep(10);
    #endif

    /* Replace without changing the literal itself */
    CORRADE_VERIFY(Directory::writeString(_thisWriteableFile,
        String::replaceFirst(Directory::readString(_thisWriteableFile),
            "_('a'); /* now this */",
            "_('a'); /* now that */")));

    std::ostringstream out;
    Debug redirectOutput{&out};
    Warning redirectWarning{&out};
    TweakableState state = tweakable.update();

    CORRADE_COMPARE(out.str(), formatString(
"Utility::Tweakable::update(): looking for updated _() macros in {0}\n"
"Utility::Tweakable::update(): ignoring unknown new value _(42.0f) in {0}:185\n"
"Utility::Tweakable::update(): ignoring unknown new value _(22.7f) in {0}:251\n", __FILE__));
    CORRADE_COMPARE(state, TweakableState::NoChange);
}

void TweakableIntegrationTest::updateUnexpectedLine() {
    CORRADE_VERIFY(Directory::exists(_thisWriteableFile));

    Tweakable tweakable;
    tweakable.enable(_thisReadablePath, TWEAKABLE_WRITE_TEST_DIR);

    /* Trigger watching of this file by executing annotated literal */
    foo();

    /* FileWatcher crutch. See its test for more info. */
    /** @todo get rid of this once proper FS inode etc. watching is implemented */
    #if defined(CORRADE_TARGET_APPLE) || defined(CORRADE_TARGET_WINDOWS) || defined(CORRADE_TARGET_EMSCRIPTEN)
    Utility::System::sleep(1100);
    #else
    Utility::System::sleep(10);
    #endif

    /* Replace without changing the literal itself */
    CORRADE_VERIFY(Directory::writeString(_thisWriteableFile,
        String::replaceFirst(Directory::readString(_thisWriteableFile),
            "_('a'); /* now this */",
            "\n_('a'); /* now this */")));

    std::ostringstream out;
    Warning redirectWarning{&out};
    TweakableState state = tweakable.update();

    CORRADE_COMPARE(out.str(), formatString(
"Utility::Tweakable::update(): code changed around _('a') in {0}:103, requesting a recompile\n", __FILE__));
    CORRADE_COMPARE(state, TweakableState::Recompile);
}

void TweakableIntegrationTest::updateDifferentType() {
    CORRADE_VERIFY(Directory::exists(_thisWriteableFile));

    Tweakable tweakable;
    tweakable.enable(_thisReadablePath, TWEAKABLE_WRITE_TEST_DIR);

    /* Trigger watching of this file by executing annotated literal */
    foo();

    /* FileWatcher crutch. See its test for more info. */
    /** @todo get rid of this once proper FS inode etc. watching is implemented */
    #if defined(CORRADE_TARGET_APPLE) || defined(CORRADE_TARGET_WINDOWS) || defined(CORRADE_TARGET_EMSCRIPTEN)
    Utility::System::sleep(1100);
    #else
    Utility::System::sleep(10);
    #endif

    /* Replace literal to a different type */
    CORRADE_VERIFY(Directory::writeString(_thisWriteableFile,
        String::replaceFirst(Directory::readString(_thisWriteableFile),
            "_('a'); /* now this */",
            "_(14.4f); /* now this */")));

    /* Now it changes, if enabled */
    std::ostringstream out;
    Warning redirectWarning{&out};
    TweakableState state = tweakable.update();

    CORRADE_COMPARE(out.str(), formatString(
"Utility::TweakableParser: 14.4f is not a character literal\n"
"Utility::Tweakable::update(): change of _(14.4f) in {0}:102 requested a recompile\n", __FILE__));
    CORRADE_COMPARE(state, TweakableState::Recompile);
}

void TweakableIntegrationTest::updateParseError() {
    CORRADE_VERIFY(Directory::exists(_thisWriteableFile));

    Tweakable tweakable;
    tweakable.enable(_thisReadablePath, TWEAKABLE_WRITE_TEST_DIR);

    /* Trigger watching of this file by executing annotated literal */
    foo();

    /* FileWatcher crutch. See its test for more info. */
    /** @todo get rid of this once proper FS inode etc. watching is implemented */
    #if defined(CORRADE_TARGET_APPLE) || defined(CORRADE_TARGET_WINDOWS) || defined(CORRADE_TARGET_EMSCRIPTEN)
    Utility::System::sleep(1100);
    #else
    Utility::System::sleep(10);
    #endif

    /* Replace literal to a broken value */
    CORRADE_VERIFY(Directory::writeString(_thisWriteableFile,
        String::replaceFirst(Directory::readString(_thisWriteableFile),
            "_('a'); /* now this */",
            "_('\\X'); /* now this */")));

    /* Now it changes, if enabled */
    std::ostringstream out;
    Error redirectError{&out};
    TweakableState state = tweakable.update();

    CORRADE_COMPARE(out.str(), formatString(
"Utility::TweakableParser: escape sequences in char literals are not implemented, sorry\n"
"Utility::Tweakable::update(): error parsing _('\\X') in {0}:102\n", __FILE__));
    CORRADE_COMPARE(state, TweakableState::Error);
}

void TweakableIntegrationTest::updateNoAlias() {
    CORRADE_VERIFY(Directory::exists(_thisWriteableFile));

    Tweakable tweakable;
    tweakable.enable(_thisReadablePath, TWEAKABLE_WRITE_TEST_DIR);

    /* Trigger watching of this file by executing annotated literal */
    foo();

    /* FileWatcher crutch. See its test for more info. */
    /** @todo get rid of this once proper FS inode etc. watching is implemented */
    #if defined(CORRADE_TARGET_APPLE) || defined(CORRADE_TARGET_WINDOWS) || defined(CORRADE_TARGET_EMSCRIPTEN)
    Utility::System::sleep(1100);
    #else
    Utility::System::sleep(10);
    #endif

    /* Comment out the alias definition */
    CORRADE_VERIFY(Directory::writeString(_thisWriteableFile,
        String::replaceFirst(Directory::readString(_thisWriteableFile),
            "#define _ CORRADE_TWEAKABLE",
            "// #define _ CORRADE_TWEAKABLE")));

    /* Now it changes, if enabled */
    std::ostringstream out;
    Warning redirectWarning{&out};
    TweakableState state = tweakable.update();

    CORRADE_COMPARE(out.str(), formatString(
"Utility::Tweakable::update(): no alias found in {0}, fallback to looking for CORRADE_TWEAKABLE()\n", __FILE__));
    CORRADE_COMPARE(state, TweakableState::NoChange);
}

}}}}

CORRADE_TEST_MAIN(Corrade::Utility::Test::TweakableIntegrationTest)
