/*
    Copyright © 2007, 2008, 2009, 2010 Vladimír Vondruš <mosra@centrum.cz>

    This file is part of Map2X.

    Map2X is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License version 3
    only, as published by the Free Software Foundation.

    Map2X is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU Lesser General Public License version 3 for more details.
*/

#include "AbstractAnimal.h"
#include "PluginManager/AbstractPluginManager.h"

namespace Map2X { namespace PluginManager { namespace Test {

class Snail: public AbstractAnimal {
    public:
        inline Snail(AbstractPluginManager* manager = 0, const std::string& plugin = ""):
            AbstractAnimal(manager, plugin) {}

        std::string name() { return "Hmlllghh"; }
        int legCount() { return 0; }
        bool hasTail() { return false; }
};

}}}

PLUGIN_REGISTER_STATIC(Snail, Map2X::PluginManager::Test::Snail,
                       "cz.mosra.Map2X.PluginManager.Test.AbstractAnimal/1.0")
