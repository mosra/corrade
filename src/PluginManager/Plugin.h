#ifndef Map2X_PluginManager_AbstractPlugin_h
#define Map2X_PluginManager_AbstractPlugin_h
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

/** @file
 * @brief Class Map2X::PluginManager::AbstractPlugin
 */

#include <string>

namespace Map2X { namespace PluginManager {

class AbstractPluginManager;

/**
 * @brief Base class for plugin interfaces
 *
 * Connects every plugin instance to parent plugin manager to ensure the
 * plugin can be unloaded only if there are no active instances.
 * @todo Make constructors/destructors private
 */
class Plugin {
    public:
        /**
         * @brief Constructor
         * @param manager       Parent plugin manager
         * @param plugin        Plugin name
         *
         * Registers this plugin instance in plugin manager.
         */
        Plugin(AbstractPluginManager* manager = 0, const std::string& plugin = "");

        /**
         * @brief Destructor
         *
         * Unregisters this plugin instance in plugin manager.
         */
        virtual ~Plugin();

    private:
        AbstractPluginManager* _manager;
        std::string _plugin;
};

}}

#endif
