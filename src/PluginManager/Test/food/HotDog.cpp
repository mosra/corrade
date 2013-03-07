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

#include "../AbstractFood.h"
#include "../Dog.h"

namespace Corrade { namespace PluginManager { namespace Test {

class HotDog: public AbstractFood {
    public:
        explicit HotDog(AbstractPluginManager* manager, std::string plugin): AbstractFood(manager, std::move(plugin)) {}

        bool isTasty()
            { return dog.hasTail() ? false : true; }
        int weight()
            { return dog.legCount()*700 + 4000; }

    private:
        Dog dog;
};

}}}

PLUGIN_REGISTER(HotDog, Corrade::PluginManager::Test::HotDog,
                "cz.mosra.Corrade.PluginManager.Test.AbstractFood/1.0")
