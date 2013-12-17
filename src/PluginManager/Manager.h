#ifndef Corrade_PluginManager_Manager_h
#define Corrade_PluginManager_Manager_h
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
 * @brief Class @ref Corrade::PluginManager::Manager
 */

#include <memory>

#include "AbstractManager.h"

#include "corradeCompatibility.h"

namespace Corrade { namespace PluginManager {

/**
@brief Plugin manager
@tparam T               Plugin interface
@tparam BaseManager     Base class, subclassed from @ref AbstractManager
    (for example if you want to add some functionality to non-templated base,
    such as signals...)

Manages loading, instancing and unloading plugins. See also
@ref plugin-management.
 */
#ifdef DOXYGEN_GENERATING_OUTPUT
template<class T, class BaseManager = AbstractManager>
#else
template<class T, class BaseManager>
#endif
class Manager: public BaseManager {
    public:
        /**
         * @brief Constructor
         *
         * Forwards arguments to @p BaseManager constructor. See
         * @ref AbstractManager::AbstractManager() for more information.
         */
        template<class ...U> explicit Manager(U&&... args);

        /**
         * @brief Plugin interface
         *
         * Only plugins with the same plugin interface string can be used
         * in this plugin manager.
         */
        std::string pluginInterface() const override;

        /**
         * @brief Plugin instance
         *
         * Returns new instance of given plugin or `nullptr` on error. The
         * plugin must be successfully loaded for the operation to succeed.
         * @see @ref loadState(), @ref load()
         */
        std::unique_ptr<T> instance(const std::string& plugin) {
            /** @todo C++14: `std::make_unique()` */
            return std::unique_ptr<T>(static_cast<T*>(BaseManager::instanceInternal(plugin)));
        }
};

template<class T, class BaseManager> template<class ...U> Manager<T, BaseManager>::Manager(U&&... args): BaseManager(std::forward<U>(args)...) {
    /* Find static plugins which have the same interface and have not
        assigned manager to them */
    for(typename std::map<std::string, typename BaseManager::Plugin*>::iterator it = BaseManager::plugins()->begin(); it != BaseManager::plugins()->end(); ++it) {
        if(it->second->loadState != LoadState::Static || it->second->manager != nullptr || it->second->staticPlugin->interface != pluginInterface())
            continue;

        /* Assign the plugin to this manager and initialize it */
        it->second->manager = this;
        it->second->staticPlugin->initializer();
    }
}

template<class T, class BaseManager> std::string Manager<T, BaseManager>::pluginInterface() const {
    return T::pluginInterface();
}

}}

#endif
