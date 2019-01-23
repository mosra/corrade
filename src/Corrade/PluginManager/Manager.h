#ifndef Corrade_PluginManager_Manager_h
#define Corrade_PluginManager_Manager_h
/*
    This file is part of Corrade.

    Copyright © 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016,
                2017, 2018, 2019 Vladimír Vondruš <mosra@centrum.cz>

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

#include "Corrade/Containers/Pointer.h"
#include "Corrade/PluginManager/AbstractManager.h"

#ifdef CORRADE_BUILD_DEPRECATED
#include "Corrade/Containers/PointerStl.h"
#endif

namespace Corrade { namespace PluginManager {

/**
@brief Plugin manager

Manages loading, instancing and unloading plugins. See @ref plugin-management
for a detailed usage tutorial.

@section PluginManager-Manager-paths Plugin directories

Plugins are searched in the following directories, in order:

1.  If a non-empty `pluginDirectory` was passed to the @ref Manager(std::string)
    constructor, plugins are searched there.
2.  Otherwise, it's expected that given plugin interface defined
    @ref AbstractPlugin::pluginSearchPaths(). The search goes through the
    entries and stops once an existing directory is found.

The matching directory is then saved and available through @ref pluginDirectory().

Besides the above, it's possible to call @ref load() with a concrete path to a
dynamic module file to load a plugin from outside of the plugin directory. The
file is expected to be accompanied by its corresponding `*.conf` metadata file
and no plugin with the same name is expected to be loaded at the same time. If
loading succeeds, the module is exposed through the API under its basename
(excluding extension).

@section PluginManager-Manager-reload Plugin loading, instantiation and unloading

A plugin is loaded by calling @ref load() with given plugin name or alias.
After that, you can get one or more instances using @ref instantiate(). For
convenience, @ref loadAndInstantiate() combines these two operations into one.

Plugin is unloaded either by calling @ref unload() or on plugin manager
destruction. By default, all active plugin instances have to be deleted before
a plugin can be unloaded. For hot reloading and other specific use cases it's
possible to unload a plugin that still has active instances. For that to work,
@ref AbstractPlugin::canBeDeleted() has to return @cpp true @ce for all active
instances, otherwise @ref unload() returns @ref LoadState::Used. Moreover, in
order to avoid double-free issues, you have to call
@ref Containers::Pointer::release() "release()" on all @ref Containers::Pointer
instances returned from @ref Manager::instance() or
@ref Manager::loadAndInstantiate().

@section PluginManager-Manager-data Plugin-specific data and configuration

Besides the API provided by a particular plugin interface after given plugin is
instantiated, plugins can also define various additional metadata that can be
accessed even if the plugin is not loaded. That can be used for example to
provide extra information to users in a plugin listing UI. Plugin-specific data
can be accessed via @ref PluginMetadata::data() through either @ref metadata()
or @ref AbstractPlugin::metadata().

In order to make it possible to configure behavior of specific plugins on top
of what the general plugin interface provides, the
@ref AbstractPlugin::configuration() function provides read-write access to
a configuration specific to a particular plugin instance. This can be used for
example to adjust various quality/speed properties of a JPEG image conversion
plugin that wouldn't be otherwise available in the general image converter
plugin API.

See @ref PluginMetadata for detailed description of the plugin metadata file.
 */
template<class T> class Manager: public AbstractManager {
    public:
        /**
         * @brief Constructor
         * @param pluginDirectory   Optional directory where plugins will be
         *      searched. See @ref PluginManager-Manager-paths for more
         *      information.
         *
         * First goes through list of static plugins and finds ones that use
         * the same interface as this manager instance. Then gets list of all
         * dynamic plugins in given directory.
         * @note Dependencies of static plugins are skipped, as static plugins
         *      should have all dependencies present. Also, dynamic plugins
         *      with the same name as another static plugin are skipped.
         * @see @ref pluginList()
         * @partialsupport The @p pluginDirectory parameter has no effect on
         *      platforms without
         *      @ref CORRADE_PLUGINMANAGER_NO_DYNAMIC_PLUGIN_SUPPORT "dynamic plugin support".
         */
        explicit Manager(std::string pluginDirectory = {}):
            #ifndef CORRADE_PLUGINMANAGER_NO_DYNAMIC_PLUGIN_SUPPORT
            AbstractManager{T::pluginInterface(), T::pluginSearchPaths(), std::move(pluginDirectory)} {}
            #else
            AbstractManager{T::pluginInterface()} { static_cast<void>(pluginDirectory); }
            #endif

        /**
         * @brief Instantiate a plugin
         *
         * Returns new instance of given plugin. The plugin must be already
         * successfully loaded by this manager. The returned value is never
         * @cpp nullptr @ce.
         * @see @ref loadAndInstantiate(),
         *      @ref AbstractManager::loadState() "loadState()",
         *      @ref AbstractManager::load() "load()"
         */
        Containers::Pointer<T> instantiate(const std::string& plugin) {
            return Containers::pointerCast<T>(instantiateInternal(plugin));
        }

        #ifdef CORRADE_BUILD_DEPRECATED
        /** @brief @copybrief instantiate()
         * @deprecated Use @ref instantiate() instead.
         */
        CORRADE_DEPRECATED("use instantiate() instead") Containers::Pointer<T> instance(const std::string& plugin) {
            return instantiate(plugin);
        }
        #endif

        /**
         * @brief Load and instantiate plugin
         *
         * Convenience alternative to calling both @ref load() and
         * @ref instantiate(). If loading fails, @cpp nullptr @ce is returned.
         *
         * As with @ref load(), it's possible to pass a file path to @p plugin.
         * See its documentation for more information. The resulting plugin
         * name is then loaded using @ref instantiate() as usual.
         */
        Containers::Pointer<T> loadAndInstantiate(const std::string& plugin) {
            return Containers::pointerCast<T>(loadAndInstantiateInternal(plugin));
        }
};

}}

#endif
