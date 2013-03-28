#ifndef Corrade_Plugins_PluginManager_h
#define Corrade_Plugins_PluginManager_h
/*
    This file is part of Corrade.

    Copyright © 2007, 2008, 2009, 2010, 2011, 2012, 2013
              Vladimír Vondruš <mosra@centrum.cz>

    Permission is hereby granted, free of charge, to any person obtaining a
    copy of this software and associated documentation files (the "Software"),
    to deal in the Software without restriction, including without limitation
    the rights to use, copy, modify, merge, publish, distribute, sublicense,
    and/or sell copies of the Software, and to permit persons to whom the
    Software is furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included
    in all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
    THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
    FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
    DEALINGS IN THE SOFTWARE.
*/

/** @file
 * @brief Class Corrade::PluginManager::PluginManager
 */

#include "AbstractPluginManager.h"

#include "corradeCompatibility.h"

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
            for(typename std::map<std::string, typename BasePluginManager::Plugin*>::iterator it = this->plugins()->begin(); it != this->plugins()->end(); ++it) {
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
