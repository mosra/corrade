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

#include "AbstractPluginAccessor.h"

namespace Corrade { namespace PluginManager {

void AbstractPluginAccessor::initialize(AbstractPluginManager* pluginManager, const std::string& pluginName, Utility::Configuration* pluginConfiguration) {
    this->pluginManager = pluginManager;
    this->pluginName = pluginName;
    this->pluginConfiguration = pluginConfiguration;
    this->pluginMetadata = new PluginMetadata(*pluginConfiguration);
}

void AbstractPluginAccessor::finalize() {
    pluginManager = 0;
    pluginName.clear();

    delete pluginMetadata;
    pluginMetadata = 0;
    delete pluginConfiguration;
    pluginConfiguration = 0;
}

}}
