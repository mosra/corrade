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

#include <PluginManager/PluginManager.h>

#include "AbstractAnimal.h"

using namespace Corrade;

int main(int argc, char** argv) {
    /* Import static plugin using the same name as in Canary.cpp */
    PLUGIN_IMPORT(Canary);

    if(argc != 2) {
        Utility::Debug() << "Usage:" << argv[0] << "animal_plugin_name";
        return 1;
    }

    /* Initialize plugin manager with given directory */
    PluginManager::PluginManager<Examples::AbstractAnimal> manager(".");

    /* Try to load a plugin */
    if(!(manager.load(argv[1]) & (PluginManager::LoadState::Loaded|PluginManager::LoadState::Static))) {
        Utility::Error() << "The requested plugin" << argv[1] << "cannot be loaded.";
        return 2;
    }

    Utility::Debug() << "Using plugin" << '\'' + *manager.metadata(argv[1])->name() + '\''
                     << "...\n";

    /* Instance of an animal */
    Examples::AbstractAnimal* animal = manager.instance(argv[1]);

    Utility::Debug() << "Name:     " << animal->name();
    Utility::Debug() << "Leg count:" << animal->legCount();
    Utility::Debug() << "Has tail: " << (animal->hasTail() ? "yes" : "no");

    /* Destruct the animal, so the plugin can be safely unloaded */
    delete animal;
    return 0;
}
