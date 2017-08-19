#ifndef Corrade_PluginManager_Manager_h
#define Corrade_PluginManager_Manager_h
/*
    This file is part of Corrade.

    Copyright © 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016,
                2017 Vladimír Vondruš <mosra@centrum.cz>

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

#include "Corrade/PluginManager/AbstractManager.h"

namespace Corrade { namespace PluginManager {

/**
@brief Plugin manager

Manages loading, instancing and unloading plugins. See also
@ref plugin-management.
 */
template<class T> class Manager: public AbstractManager {
    public:
        /**
         * @brief Constructor
         * @param pluginDirectory   Directory where plugins will be searched,
         *      encoded in UTF-8. No recursive processing is done.
         *
         * First goes through list of static plugins and finds ones that use
         * the same interface as this manager instance. Then gets list of all
         * dynamic plugins in given directory.
         * @note Dependencies of static plugins are skipped, as static plugins
         *      should have all dependencies present. Also, dynamic plugins
         *      with the same name as another static plugin are skipped.
         * @see @ref pluginList()
         * @partialsupport Parameter @p pluginDirectory has no effect on
         *      @ref CORRADE_TARGET_EMSCRIPTEN "Emscripten",
         *      @ref CORRADE_TARGET_WINDOWS_RT "Windows RT",
         *      @ref CORRADE_TARGET_IOS "iOS" and
         *      @ref CORRADE_TARGET_ANDROID "Android" as only static plugins
         *      are supported.
         */
        explicit Manager(std::string pluginDirectory = {}): AbstractManager(T::pluginInterface(), std::move(pluginDirectory)) {}

        /**
         * @brief Plugin instance
         *
         * Returns new instance of given plugin. The plugin must be
         * already successfully loaded by this manager. The returned value is
         * never `nullptr`.
         * @see @ref loadAndInstantiate(),
         *      @ref AbstractManager::loadState() "loadState()",
         *      @ref AbstractManager::load() "load()"
         */
        std::unique_ptr<T> instance(const std::string& plugin) {
            /** @todo C++14: `std::make_unique()` */
            return std::unique_ptr<T>(static_cast<T*>(instanceInternal(plugin)));
        }

        /**
         * @brief Load and instantiate plugin
         *
         * Convenience alternative to calling both @ref load() and
         * @ref instance(). If loading fails, `nullptr` is returned.
         */
        std::unique_ptr<T> loadAndInstantiate(const std::string& plugin) {
            if(!(load(plugin) & LoadState::Loaded)) return nullptr;
            return instance(plugin);
        }
};

}}

#endif
