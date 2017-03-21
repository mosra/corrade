/*
    This file is part of Corrade.

    Copyright © 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016,
                2017 Vladimír Vondruš <mosra@centrum.cz>

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

#include <Corrade/PluginManager/Manager.h>
#include <Corrade/Utility/Arguments.h>
#include <Corrade/Utility/Debug.h>

#include "AbstractAnimal.h"

using namespace Corrade;

int main(int argc, char** argv) {
    /* Import static plugin using the same name as in Canary.cpp */
    CORRADE_PLUGIN_IMPORT(Canary);

    Utility::Arguments args;
    args.addArgument("plugin").setHelp("plugin", "animal plugin name")
        .addOption("plugin-dir", ".").setHelp("plugin-dir", "plugin directory to use", "DIR")
        .setHelp("Displays info about given animal.")
        .parse(argc, argv);

    /* Initialize plugin manager with given directory */
    PluginManager::Manager<Examples::AbstractAnimal> manager(args.value("plugin-dir"));

    /* Try to load a plugin */
    if(!(manager.load(args.value("plugin")) & PluginManager::LoadState::Loaded)) {
        Utility::Error() << "The requested plugin" << args.value("plugin") << "cannot be loaded.";
        return 2;
    }

    /* Instance of an animal */
    std::unique_ptr<Examples::AbstractAnimal> animal = manager.instance(args.value("plugin"));

    Utility::Debug() << "Using plugin" << '\'' + animal->metadata()->data().value("name") + '\''
                     << "...\n";

    Utility::Debug() << "Name:     " << animal->name();
    Utility::Debug() << "Leg count:" << animal->legCount();
    Utility::Debug() << "Has tail: " << (animal->hasTail() ? "yes" : "no");

    return 0;
}
