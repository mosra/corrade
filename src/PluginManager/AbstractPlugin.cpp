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

#include "AbstractPlugin.h"

#include "AbstractPluginManager.h"

namespace Corrade { namespace PluginManager {

AbstractPlugin::AbstractPlugin(): _manager(nullptr), _configuration(nullptr), _metadata(nullptr) {}

AbstractPlugin::AbstractPlugin(AbstractPluginManager* manager, std::string plugin): _manager(manager), _plugin(std::move(plugin)) {
    _manager->registerInstance(_plugin, this, &_configuration, &_metadata);
}

AbstractPlugin::~AbstractPlugin() {
    if(_manager) _manager->unregisterInstance(_plugin, this);
}

}}
