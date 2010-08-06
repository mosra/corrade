#ifndef Map2X_Plugins_PluginManager_h
#define Map2X_Plugins_PluginManager_h
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
 * @brief Class Map2X::PluginManager::PluginManager
 */

#include <string>
#include <vector>

#include "PluginMetadata.h"
#include "AbstractPluginManager.h"

namespace Map2X { namespace PluginManager {

/**
 * @brief Plugin manager
 *
 * Manages loading, instancing and unloading plugins.
 * See also @ref PluginManagement.
 */
template<class T> class PluginManager: public AbstractPluginManager {
    public:
        /** @copydoc AbstractPluginManager::AbstractPluginManager() */
        PluginManager(const std::string& pluginDirectory): AbstractPluginManager(pluginDirectory) {
            /* Load static plugins which use the same API */
            for(std::vector<StaticPlugin>::const_iterator i = staticPlugins.begin(); i != staticPlugins.end(); ++i) {
                PluginObject p;
                p.loadState = IsStatic;
                p.instancer = i->instancer;
                i->metadataCreator(&p.metadata);

                if(p.metadata.interface == pluginInterface())
                    plugins.insert(std::pair<std::string, PluginObject>(i->name, p));
            }
        }

        std::string pluginInterface() const { return T::pluginInterface(); }

        /**
         * @brief Plugin class instance
         * @param name              Plugin name
         * @return Pointer to new instance of plugin class, zero on error
         *
         * Creates new instance of plugin class, if possible. If the plugin is
         * not successfully loaded, returns zero pointer.
         */
        T* instance(const std::string& name) {
            /* Plugin with given name doesn't exist */
            if(plugins.find(name) == plugins.end()) return 0;

            PluginObject& plugin = plugins.at(name);

            /* Plugin is not successfully loaded */
            if(!(plugin.loadState & (LoadOk|IsStatic))) return 0;

            return static_cast<T*>(plugin.instancer(this, name));
        }
};

}}

#endif
