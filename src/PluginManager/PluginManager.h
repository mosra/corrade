#ifndef Kompas_Plugins_PluginManager_h
#define Kompas_Plugins_PluginManager_h
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

/** @file
 * @brief Class Kompas::PluginManager::PluginManager
 */

#ifndef KOMPAS_SKIP_PLUGINMANAGER_NAMESPACE
#include <string>
#include <vector>

#include "AbstractPluginManager.h"

namespace Kompas { namespace PluginManager {
#endif

/**
 * @brief %Plugin manager
 *
 * Manages loading, instancing and unloading plugins.
 * See also @ref PluginManagement.
 */
template<class T> class PluginManager: public AbstractPluginManager {
    public:
        /** @copydoc AbstractPluginManager::AbstractPluginManager() */
        PluginManager(const std::string& pluginDirectory): AbstractPluginManager(pluginDirectory) {
            /* Find static plugins which have the same interface and have not
               assigned manager to them */
            for(std::map<std::string, PluginObject*>::iterator it = plugins()->begin(); it != plugins()->end(); ++it) {
                if(it->second->manager != 0 || it->second->interface != pluginInterface())
                    continue;

                /* Assign the plugin to this manager */
                it->second->manager = this;
            }
        }

        std::string pluginInterface() const { return T::pluginInterface(); }

        /**
         * @brief %Plugin class instance
         * @param _plugin           %Plugin
         * @return Pointer to new instance of plugin class, zero on error
         *
         * Creates new instance of plugin class, if possible. If the plugin is
         * not successfully loaded, returns zero pointer.
         */
        T* instance(const std::string& _plugin) {
            /* Plugin with given name doesn't exist */
            if(plugins()->find(_plugin) == plugins()->end()) return 0;

            PluginObject& plugin = *plugins()->at(_plugin);

            /* Plugin is not successfully loaded */
            if(!(plugin.loadState & (LoadOk|IsStatic))) return 0;

            return static_cast<T*>(plugin.instancer(this, _plugin));
        }
};

#ifndef KOMPAS_SKIP_PLUGINMANAGER_NAMESPACE
}}
#endif

#endif
