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
