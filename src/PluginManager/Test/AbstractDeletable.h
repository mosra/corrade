#ifndef Kompas_PluginManager_Test_AbstractDeletable_h
#define Kompas_PluginManager_Test_AbstractDeletable_h
/*
    Copyright © 2007, 2008, 2009, 2010, 2011 Vladimír Vondruš <mosra@centrum.cz>

    This file is part of Kompas.

    Kompas is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License version 3
    only, as published by the Free Software Foundation.

    Kompas is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU Lesser General Public License version 3 for more details.
*/

#include "PluginManager/Plugin.h"

namespace Kompas { namespace PluginManager { namespace Test {

class AbstractDeletable: public Plugin {
    PLUGIN_INTERFACE("cz.mosra.Kompas.PluginManager.Test.AbstractDeletable/1.0")

    protected:
        unsigned int* var;

    public:
        inline AbstractDeletable(AbstractPluginManager* manager = 0, const std::string& plugin = ""):
            Plugin(manager, plugin) {}

        void set(unsigned int* var) { this->var = var; }

        bool canBeDeleted() { return true; }
};

}}}

#endif
