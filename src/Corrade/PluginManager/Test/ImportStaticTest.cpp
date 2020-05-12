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
#include <vector>

#include "Corrade/PluginManager/Manager.h"
#include "Corrade/TestSuite/Tester.h"
#include "Corrade/Utility/DebugStl.h" /** @todo remove when <sstream> is gone */

#include "AbstractAnimal.h"

static void importPlugin() {
    CORRADE_PLUGIN_IMPORT(Canary)
}

static void importPluginAnother() {
    CORRADE_PLUGIN_IMPORT(Dird)
}

static void ejectPlugin() {
    CORRADE_PLUGIN_EJECT(Canary)
}

static void ejectPluginAnother() {
    CORRADE_PLUGIN_EJECT(Dird)
}

namespace Corrade { namespace PluginManager { namespace Test { namespace {

struct ImportStaticTest: TestSuite::Tester {
    explicit ImportStaticTest();

    /* These test that the linked list isn't getting overwritten in a bad way
       (such as clearing the `next` pointer before adding an item, which was
       the case before). Thorough tests for the linked list operations are in
       Containers/Test/RawForwardListTest.cpp */
    void importOnce();
    void importTwice();
    void importTwiceMixedWithAnother();
};

ImportStaticTest::ImportStaticTest() {
    addTests({&ImportStaticTest::importOnce,
              &ImportStaticTest::importTwice,
              &ImportStaticTest::importTwiceMixedWithAnother});
}

void ImportStaticTest::importOnce() {
    /* Nothing initialized yet so the plugin list is empty */
    {
        /* Avoid importing all dynamic plugins as well */
        PluginManager::Manager<AbstractAnimal> manager{"nonexistent"};
        CORRADE_COMPARE(manager.pluginList(), std::vector<std::string>{});
    }

    importPlugin();

    std::ostringstream out;
    Error redirectError{&out};

    /* This shouldn't report any error and list the plugin just once */
    {
        PluginManager::Manager<AbstractAnimal> manager{"nonexistent"};
        CORRADE_COMPARE(out.str(), "");
        CORRADE_COMPARE(manager.pluginList(), std::vector<std::string>{"Canary"});
    }

    ejectPlugin();

    /* Plugin list is empty again */
    {
        PluginManager::Manager<AbstractAnimal> manager{"nonexistent"};
        CORRADE_COMPARE(manager.pluginList(), std::vector<std::string>{});
    }
}

void ImportStaticTest::importTwice() {
    /* Nothing initialized yet so the plugin is empty */
    {
        /* Avoid importing all dynamic plugins as well */
        PluginManager::Manager<AbstractAnimal> manager{"nonexistent"};
        CORRADE_COMPARE(manager.pluginList(), std::vector<std::string>{});
    }

    importPlugin();
    importPlugin();

    std::ostringstream out;
    Error redirectError{&out};

    /* This shouldn't report any error and list the plugin just once */
    {
        PluginManager::Manager<AbstractAnimal> manager{"nonexistent"};
        CORRADE_COMPARE(out.str(), "");
        CORRADE_COMPARE(manager.pluginList(), std::vector<std::string>{"Canary"});

    /* And instantiating everything second time should have no issues either */
    } {
        PluginManager::Manager<AbstractAnimal> manager{"nonexistent"};
        CORRADE_COMPARE(out.str(), "");
        CORRADE_COMPARE(manager.pluginList(), std::vector<std::string>{"Canary"});
    }

    ejectPlugin();
    ejectPlugin();

    /* Plugin list is empty again */
    {
        PluginManager::Manager<AbstractAnimal> manager{"nonexistent"};
        CORRADE_COMPARE(manager.pluginList(), std::vector<std::string>{});
    }
}

void ImportStaticTest::importTwiceMixedWithAnother() {
    /* Nothing initialized yet so the plugin is empty */
    {
        /* Avoid importing all dynamic plugins as well */
        PluginManager::Manager<AbstractAnimal> manager{"nonexistent"};
        CORRADE_COMPARE(manager.pluginList(), std::vector<std::string>{});
    }

    importPlugin();
    importPluginAnother();
    importPlugin();

    std::ostringstream out;
    Error redirectError{&out};

    /* This shouldn't report any error and list the plugin just once */
    {
        PluginManager::Manager<AbstractAnimal> manager{"nonexistent"};
        CORRADE_COMPARE(out.str(), "");
        CORRADE_COMPARE(manager.pluginList(), (std::vector<std::string>{"Canary", "Dird"}));

    /* And instantiating everything second time should have no issues either */
    } {
        PluginManager::Manager<AbstractAnimal> manager{"nonexistent"};
        CORRADE_COMPARE(out.str(), "");
        CORRADE_COMPARE(manager.pluginList(), (std::vector<std::string>{"Canary", "Dird"}));
    }

    ejectPlugin();
    ejectPluginAnother();
    ejectPlugin();

    /* Plugin list is empty again */
    {
        PluginManager::Manager<AbstractAnimal> manager{"nonexistent"};
        CORRADE_COMPARE(manager.pluginList(), std::vector<std::string>{});
    }
}

}}}}

CORRADE_TEST_MAIN(Corrade::PluginManager::Test::ImportStaticTest)
