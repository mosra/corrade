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

#include "Corrade/PluginManager/Manager.h"
#include "Corrade/TestSuite/Tester.h"
#include "Corrade/Utility/DebugStl.h" /** @todo remove when <sstream> is gone */

#include "init-fini/InitFini.h"

static void importPlugin() {
    CORRADE_PLUGIN_IMPORT(InitFiniStatic)
}

namespace Corrade { namespace PluginManager { namespace Test { namespace {

struct ManagerInitFiniTest: TestSuite::Tester {
    explicit ManagerInitFiniTest();

    void staticPlugin();
    #ifndef CORRADE_PLUGINMANAGER_NO_DYNAMIC_PLUGIN_SUPPORT
    void dynamicPlugin();
    #endif
};

ManagerInitFiniTest::ManagerInitFiniTest() {
    addTests({&ManagerInitFiniTest::staticPlugin,
              #ifndef CORRADE_PLUGINMANAGER_NO_DYNAMIC_PLUGIN_SUPPORT
              &ManagerInitFiniTest::dynamicPlugin,
              #endif
              });

    importPlugin();
}

void ManagerInitFiniTest::staticPlugin() {
    std::ostringstream out;
    Debug redirectDebug{&out};

    {
        /* Initialization is right after manager assigns them to itself */
        PluginManager::Manager<InitFini> manager;
        CORRADE_COMPARE(out.str(), "Static plugin initialized\n");

        /* Finalization is right before manager frees them */
        out.str({});
    }

    CORRADE_COMPARE(out.str(), "Static plugin finalized\n");
}

#ifndef CORRADE_PLUGINMANAGER_NO_DYNAMIC_PLUGIN_SUPPORT
void ManagerInitFiniTest::dynamicPlugin() {
    std::ostringstream out;
    Debug redirectDebug{&out};

    {
        PluginManager::Manager<InitFini> manager;
        CORRADE_COMPARE(out.str(), "Static plugin initialized\n");

        /* Initialization is right after manager loads them. Base
           initialization is not called again. */
        out.str({});
        CORRADE_COMPARE(manager.load("InitFiniDynamic"), LoadState::Loaded);
        CORRADE_COMPARE(out.str(), "Dynamic plugin initialized\n");

        /* Finalization is right before manager unloads them. Base finalization
           is not called yet. */
        out.str({});
        CORRADE_COMPARE(manager.unload("InitFiniDynamic"), LoadState::NotLoaded);
        CORRADE_COMPARE(out.str(), "Dynamic plugin finalized\n");

        out.str({});
    }

    /* Static plugin (a dependency of the dynamic one) is called on
       destruction */
    CORRADE_COMPARE(out.str(), "Static plugin finalized\n");
}
#endif

}}}}

CORRADE_TEST_MAIN(Corrade::PluginManager::Test::ManagerInitFiniTest)
