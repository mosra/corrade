#ifndef Corrade_PluginManager_Manager_h
#define Corrade_PluginManager_Manager_h
/*
    This file is part of Corrade.

    Copyright © 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016,
                2017, 2018, 2019, 2020, 2021, 2022, 2023, 2024, 2025
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

#include "Corrade/PluginManager/AbstractManager.h"

namespace Corrade { namespace PluginManager {

/**
@brief Plugin manager

Manages loading, instancing and unloading plugins. See @ref plugin-management
for a high-level introduction.

@section PluginManager-Manager-paths Plugin directories

Plugins are searched in the following directories, in order:

1.  If a non-empty `pluginDirectory` was passed to the
    @ref Manager(Containers::StringView) constructor, plugins are searched
    there. If the directory doesn't exist, no search is performed and the
    process stops here.
2.  Otherwise, it's expected that given plugin interface defined
    @ref AbstractPlugin::pluginSearchPaths(). The search goes through the
    entries and stops once an existing directory is found.

In both cases the path of @ref Utility::Path::executableLocation() is
prepended to relative paths before testing. The matching directory is then
saved and available through @ref pluginDirectory().

Besides the above, it's possible to call @ref load() with a concrete path to a
dynamic module file to load a plugin from outside of the plugin directory. The
file is expected to be accompanied by its corresponding `*.conf` metadata file
(unless overridden in the plugin interface using
@ref AbstractPlugin::pluginMetadataSuffix())
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
instances returned from @ref Manager::instantiate() or
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
plugin that wouldn't be otherwise available in a general image converter plugin
API.

See @ref PluginMetadata for detailed description of the plugin metadata file.

@section PluginManager-Manager-dependencies Plugin dependencies

Plugins can depend on each other by listing then in the metadata file, as
described in the @ref PluginMetadata class documentation. From the user
point-of-view the dependency handling is completely transparent --- loading a
plugin will also load all its dependencies, fail if some dependencies can't be
found, and disallow the plugin from being unloaded when some other still needs
it.

A special (and rarer) case is inter-manager dependencies. By default, to avoid
shared mutable global state, plugin manager instances don't know about each
other, so if a plugin depends on another from a different interface, you need
to instantiate a manager for the other interface and register it with the
original manager using @ref registerExternalManager().

@section PluginManager-Manager-multiple-instances Multiple instances of the same manager

It's possible to have more than one instance of the same manager as well as
loading and instantiating the same plugin in more than one manager at the same
time. For static plugins there's no reason this couldn't work, for dynamic both
the Unix @m_class{m-doc-external} [dlopen()](https://man.archlinux.org/man/dlopen.3)
and Windows @m_class{m-doc-external} [LoadLibrary()](https://docs.microsoft.com/en-us/windows/win32/api/libloaderapi/nf-libloaderapi-loadlibraryw)
load the library only once and maintain reference count to ensure it's unloaded
only after it's not needed anymore. The only way you may run into problems is
by loading the same plugin binary twice under different filenames, causing the
loader to have symbol conflicts, violating ODR.

@section PluginManager-Manager-template-definitions Custom plugin interfaces and template definitions

To avoid pulling in relatively big includes, the
@ref Corrade/PluginManager/Manager.h header contains only declarations of the
template functions. While that works fine when using builtin plugin interfaces
in Corrade and Magnum, you need to make an extra step when implementing your
own plugin interfaces.

The easiest is to simply include @ref Corrade/PluginManager/Manager.hpp instead
of the `*.h` file in order to get the full definitions. If you however want to
avoid the additional overhead of the template instantiations and includes it
pulls in, it's recommended to include it only in the source file where you
define the plugin interface and perform an explicit template instantiation:

@snippet PluginManager.cpp Manager-explicit-template-instantiation

The namespace where you locate the @cpp template class @ce is important.
Additionally, if the plugin interface is exposed in a shared library, you may
need to export the template instantiation symbols for example with
@ref CORRADE_VISIBILITY_EXPORT. Look into Corrade and Magnum sources for
further examples.

@section PluginManager-Manager-multithreading Thread safety

Static plugins register themselves into a global storage. If done implicitly,
the registration is executed before entering @cpp main() @ce and thus serially.
If done explicitly via @ref CORRADE_PLUGIN_IMPORT() / @ref CORRADE_PLUGIN_EJECT(),
these macros *have to* be called from a single thread or externally guarded to
avoid data races.

On the other hand, all other functionality only reads from the global storage
and thus is thread-safe.
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
        #ifdef DOXYGEN_GENERATING_OUTPUT
        explicit Manager(Containers::StringView pluginDirectory = {});
        #else
        explicit Manager(Containers::StringView pluginDirectory);
        explicit Manager();
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
        Containers::Pointer<T> instantiate(Containers::StringView plugin);

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
        Containers::Pointer<T> loadAndInstantiate(Containers::StringView plugin);

        /**
         * @brief Retrieve an external plugin manager
         * @m_since_latest
         *
         * If a plugin manager of type @p U was passed to
         * @ref registerExternalManager() earlier, returns a pointer to it,
         * otherwise returns @cpp nullptr @ce. The manager type is matched
         * using @ref pluginInterface(), thus it's expected that the plugin
         * defines it to a non-empty and unique value.
         */
        template<class U> Manager<U>* externalManager() {
            return static_cast<Manager<U>*>(externalManagerInternal(U::pluginInterface()));
        }
};

}}

#ifdef CORRADE_BUILD_DEPRECATED
/* The definitions used to be inline, include them for backwards compatibility.
   Updated code should either include the hpp directly or include it in plugin
   interface implementation only and export accordingly. */
#include "Corrade/PluginManager/Manager.hpp"
#endif

#endif
