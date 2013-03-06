#ifndef Corrade_Examples_AbstractAnimal_h
#define Corrade_Examples_AbstractAnimal_h
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

#include <PluginManager/Plugin.h>

namespace Corrade { namespace Examples {

class AbstractAnimal: public PluginManager::Plugin {
    PLUGIN_INTERFACE("cz.mosra.Animals.AbstractAnimal/1.0")

    public:
        AbstractAnimal(PluginManager::AbstractPluginManager* manager = nullptr, const std::string& plugin = std::string()):
            Plugin(manager, plugin) {}

        virtual std::string name() const = 0;
        virtual int legCount() const = 0;
        virtual bool hasTail() const = 0;
};

}}

#endif
