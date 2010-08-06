#ifndef Map2X_PluginManager_Test_AbstractAnimal_h
#define Map2X_PluginManager_Test_AbstractAnimal_h
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

#include "PluginManager/definitions.h"
#include "PluginManager/Plugin.h"

namespace Map2X { namespace PluginManager { namespace Test {

class AbstractAnimal: public Plugin {
    PLUGIN_INTERFACE("cz.mosra.Map2X.PluginManager.Test.AbstractAnimal/1.0")

    public:
        inline AbstractAnimal(AbstractPluginManager* manager = 0, const std::string& plugin = ""):
            Plugin(manager, plugin) {}

        virtual std::string name() = 0;
        virtual int legCount() = 0;
        virtual bool hasTail() = 0;
};

}}}

#endif
