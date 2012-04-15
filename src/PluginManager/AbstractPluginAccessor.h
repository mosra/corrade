#ifndef Corrade_PluginManager_AbstractPluginAccessor_h
#define Corrade_PluginManager_AbstractPluginAccessor_h
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

/** @file
 * @brief Class Corrade::PluginManager::AbstractPluginAccessor
 */

#include "Plugin.h"

namespace Corrade { namespace PluginManager {

class AbstractPluginAccessor: public Plugin {
    friend class AbstractPluginManager;

    public:
        AbstractPluginAccessor(AbstractPluginManager* manager = 0, const std::string& plugin = ""): Plugin(manager, plugin) {}

        void initialize(AbstractPluginManager* pluginManager, const std::string& pluginName, Utility::Configuration* pluginConfiguration);
        void finalize();

        virtual AbstractPluginManager::LoadState load() = 0;
        virtual AbstractPluginManager::LoadState unload() = 0;
        virtual void* instance() = 0;

    protected:
        std::string pluginName;
        AbstractPluginManager::LoadState loadState;
        AbstractPluginManager* pluginManager;
        Utility::Configuration* pluginConfiguration;
        PluginMetadata* pluginMetadata;
};

}}

#endif
