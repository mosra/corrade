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

#include "../AbstractDeletable.h"
#include "PluginManager/AbstractPluginManager.h"

namespace Corrade { namespace PluginManager { namespace Test {

class Deletable: public AbstractDeletable {
    public:
        inline Deletable(AbstractPluginManager* manager = 0, const std::string& plugin = ""):
            AbstractDeletable(manager, plugin) {}

        ~Deletable() { *var = 0xDEADBEEF; }
};

}}}

PLUGIN_REGISTER(Deletable, Corrade::PluginManager::Test::Deletable,
                "cz.mosra.Corrade.PluginManager.Test.AbstractDeletable/1.0")
