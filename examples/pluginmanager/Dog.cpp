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

#include "AbstractAnimal.h"

namespace Corrade { namespace Examples {

class Dog: public AbstractAnimal {
    public:
        Dog() = default;
        Dog(PluginManager::AbstractPluginManager* manager, std::string plugin): AbstractAnimal(manager, std::move(plugin)) {}

        std::string name() const { return "Doug"; }
        int legCount() const { return 4; }
        bool hasTail() const { return true; }
};

}}

PLUGIN_REGISTER(Dog, Corrade::Examples::Dog, "cz.mosra.Corrade.Examples.AbstractAnimal/1.0")
