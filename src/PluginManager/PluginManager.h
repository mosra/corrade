#ifndef Corrade_Plugins_PluginManager_h
#define Corrade_Plugins_PluginManager_h
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
 * @brief Class Corrade::PluginManager::PluginManager
 */

#include "AbstractPluginManager.h"

namespace Corrade { namespace PluginManager {

/**
@brief Plugin manager
@tparam T                   Plugin interface
@tparam BasePluginManager   Base class, subclassed from AbstractPluginManager
    (for example if you want to add some functionality to non-templated base,
    such as Qt signals)

Manages loading, instancing and unloading plugins. See also
@ref plugin-management.

@todo C++11 - provide constructor with arbitrary arguments
 */
template<class T, class BasePluginManager = AbstractPluginManager> class PluginManager: public BasePluginManager {
    public:
        /** @copydoc AbstractPluginManager::AbstractPluginManager() */
        explicit PluginManager(const std::string& pluginDirectory): BasePluginManager(pluginDirectory) {
            /* Find static plugins which have the same interface and have not
               assigned manager to them */
            for(typename std::map<std::string, typename BasePluginManager::PluginObject*>::iterator it = this->plugins()->begin(); it != this->plugins()->end(); ++it) {
                if(it->second->manager != nullptr || it->second->interface != pluginInterface())
                    continue;

                /* Assign the plugin to this manager */
                it->second->manager = this;
            }
        }

        /**
         * @brief Plugin interface
         *
         * Only plugins with the same plugin interface string can be used
         * in this plugin manager.
         */
        std::string pluginInterface() const override { return T::pluginInterface(); }

        /**
         * @brief Plugin instance
         *
         * Returns new instance of given plugin or `nullptr` on error. The
         * plugin must be successfully loaded for the operation to succeed.
         * @see loadState(), load()
         */
        T* instance(const std::string& plugin) {
            return static_cast<T*>(this->instanceInternal(plugin));
        }
};

}}

#endif
