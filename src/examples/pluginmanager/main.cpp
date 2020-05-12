/*
    This file is part of Corrade.

    Original authors — credit is appreciated but not required:

        2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016,
        2017, 2018, 2019, 2020 — Vladimír Vondruš <mosra@centrum.cz>

    This is free and unencumbered software released into the public domain.

    Anyone is free to copy, modify, publish, use, compile, sell, or distribute
    this software, either in source code form or as a compiled binary, for any
    purpose, commercial or non-commercial, and by any means.

    In jurisdictions that recognize copyright laws, the author or authors of
    this software dedicate any and all copyright interest in the software to
    the public domain. We make this dedication for the benefit of the public
    at large and to the detriment of our heirs and successors. We intend this
    dedication to be an overt act of relinquishment in perpetuity of all
    present and future rights to this software under copyright law.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
    THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
    IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
    CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include <Corrade/PluginManager/Manager.h>
#include <Corrade/PluginManager/PluginMetadata.h>
#include <Corrade/Utility/Arguments.h>
#include <Corrade/Utility/ConfigurationGroup.h>
#include <Corrade/Utility/Debug.h>
#include <Corrade/Utility/DebugStl.h>

#include "AbstractAnimal.h"

using namespace Corrade;

int main(int argc, char** argv) {
    /* Import static plugin using the same name as in Canary.cpp */
    CORRADE_PLUGIN_IMPORT(Canary)

    Utility::Arguments args;
    args.addArgument("plugin").setHelp("plugin", "animal plugin name")
        .setGlobalHelp("Displays info about given animal.")
        .parse(argc, argv);

    /* Initialize plugin manager with given directory */
    PluginManager::Manager<Examples::AbstractAnimal> manager;

    /* Try to load a plugin */
    if(!(manager.load(args.value("plugin")) & PluginManager::LoadState::Loaded)) {
        Utility::Error{} << "The requested plugin" << args.value("plugin") << "cannot be loaded.";
        return 2;
    }

    /* Instance of an animal */
    Containers::Pointer<Examples::AbstractAnimal> animal = manager.instantiate(args.value("plugin"));

    Utility::Debug{} << "Using plugin" << '\'' + animal->metadata()->data().value("name") + '\''
                     << "...\n";

    Utility::Debug{} << "Name:     " << animal->name();
    Utility::Debug{} << "Leg count:" << animal->legCount();
    Utility::Debug{} << "Has tail: " << (animal->hasTail() ? "yes" : "no");

    return 0;
}
