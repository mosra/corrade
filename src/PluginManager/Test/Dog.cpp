/*
    Copyright © 2007, 2008, 2009, 2010 Vladimír Vondruš <mosra@centrum.cz>

    This file is part of Kompas.

    Kompas is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License version 3
    only, as published by the Free Software Foundation.

    Kompas is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU Lesser General Public License version 3 for more details.
*/

#include "Dog.h"

namespace Kompas { namespace PluginManager { namespace Test {

std::string Dog::name() { return "Doug"; }
bool Dog::hasTail() { return true; }
int Dog::legCount() { return 4; }

PLUGIN_REGISTER(Dog, "cz.mosra.Kompas.PluginManager.Test.AbstractAnimal/1.0")

}}}
