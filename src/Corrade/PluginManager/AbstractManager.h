#ifndef Corrade_PluginManager_AbstractManager_h
#define Corrade_PluginManager_AbstractManager_h
/*
    This file is part of Corrade.

    Copyright © 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016,
                2017, 2018, 2019, 2020 Vladimír Vondruš <mosra@centrum.cz>

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
 * @brief Class @ref Corrade::PluginManager::AbstractManager, macro @ref CORRADE_PLUGIN_VERSION, @ref CORRADE_PLUGIN_REGISTER()
 */

#include "Corrade/Containers/EnumSet.h"
#include "Corrade/Containers/Pointer.h"
#include "Corrade/PluginManager/PluginManager.h"
#include "Corrade/PluginManager/visibility.h"
#include "Corrade/Utility/StlForwardString.h"
#include "Corrade/Utility/StlForwardVector.h"
#include "Corrade/Utility/Utility.h"

#ifdef CORRADE_TARGET_WINDOWS
/* I didn't find a better way to circumvent the need for including windows.h */
struct HINSTANCE__;
typedef struct HINSTANCE__* HMODULE;

/* combaseapi.h in Windows does this insane thing. Can be worked around by
   defining WIN32_LEAN_AND_MEAN but not everybody does that. */
#ifdef interface
#undef interface
#endif
#endif

namespace Corrade { namespace PluginManager {

/**
@brief Plugin load state

@see @ref LoadStates, @ref AbstractManager::loadState(),
    @ref AbstractManager::load(), @ref AbstractManager::unload()
*/
enum class LoadState: unsigned short {
    /**
     * The plugin cannot be found. Returned by @ref AbstractManager::loadState()
     * and @ref AbstractManager::load().
     */
    NotFound = 1 << 0,

    #ifndef CORRADE_PLUGINMANAGER_NO_DYNAMIC_PLUGIN_SUPPORT
    /**
     * The plugin is build with different version of plugin manager and cannot
     * be loaded. Returned by @ref AbstractManager::load().
     * @partialsupport Not available on platforms without
     *      @ref CORRADE_PLUGINMANAGER_NO_DYNAMIC_PLUGIN_SUPPORT "dynamic plugin support".
     */
    WrongPluginVersion = 1 << 1,

    /**
     * The plugin uses different interface than the interface used by plugin
     * manager and cannot be loaded. Returned by @ref AbstractManager::load().
     * @partialsupport Not available on platforms without
     *      @ref CORRADE_PLUGINMANAGER_NO_DYNAMIC_PLUGIN_SUPPORT "dynamic plugin support".
     */
    WrongInterfaceVersion = 1 << 2,

    /**
     * The plugin doesn't have any associated `*.conf` metadata file or the
     * metadata file contains errors. Returned by
     * @ref AbstractManager::loadState() and @ref AbstractManager::load().
     * @partialsupport Not available on platforms without
     *      @ref CORRADE_PLUGINMANAGER_NO_DYNAMIC_PLUGIN_SUPPORT "dynamic plugin support".
     */
    WrongMetadataFile = 1 << 3,

    /**
     * The plugin depends on another plugin, which cannot be loaded (e.g. not
     * found or wrong version). Returned by @ref AbstractManager::load(). Note
     * that plugins may have cross-manager dependencies, and to resolve these
     * you need to explicitly pass a manager instance containing the
     * dependencies to @ref AbstractManager::registerExternalManager().
     * @partialsupport Not available on platforms without
     *      @ref CORRADE_PLUGINMANAGER_NO_DYNAMIC_PLUGIN_SUPPORT "dynamic plugin support".
     */
    UnresolvedDependency = 1 << 4,

    /**
     * The plugin failed to load for other reason (e.g. linking failure).
     * Returned by @ref AbstractManager::load().
     * @partialsupport Not available on platforms without
     *      @ref CORRADE_PLUGINMANAGER_NO_DYNAMIC_PLUGIN_SUPPORT "dynamic plugin support".
     */
    LoadFailed = 1 << 5,
    #endif

    /**
     * The plugin is static. Returned by @ref AbstractManager::loadState() and
     * @ref AbstractManager::load().
     */
    Static = 1 << 6,

    /**
     * The plugin is successfully loaded. Returned by @ref AbstractManager::loadState()
     * and @ref AbstractManager::load(). The value includes value of @ref LoadState::Static,
     * see @ref LoadStates for more information. In @ref CORRADE_TARGET_EMSCRIPTEN "Emscripten",
     * @ref CORRADE_TARGET_WINDOWS_RT "Windows RT", @ref CORRADE_TARGET_IOS "iOS"
     * and @ref CORRADE_TARGET_ANDROID the value is equivalent to
     * @ref LoadState::Static.
     */
    #ifndef CORRADE_PLUGINMANAGER_NO_DYNAMIC_PLUGIN_SUPPORT
    Loaded = (1 << 7) | LoadState::Static,
    #else
    Loaded = LoadState::Static,
    #endif

    #ifndef CORRADE_PLUGINMANAGER_NO_DYNAMIC_PLUGIN_SUPPORT
    /**
     * The plugin is not loaded. Plugin can be unloaded only if is dynamic and
     * is not required by any other plugin. Returned by
     * @ref AbstractManager::loadState() and @ref AbstractManager::load().
     * @partialsupport Not available on platforms without
     *      @ref CORRADE_PLUGINMANAGER_NO_DYNAMIC_PLUGIN_SUPPORT "dynamic plugin support".
     */
    NotLoaded = 1 << 8,

    /**
     * The plugin failed to unload. Returned by @ref AbstractManager::unload().
     * @partialsupport Not available on platforms without
     *      @ref CORRADE_PLUGINMANAGER_NO_DYNAMIC_PLUGIN_SUPPORT "dynamic plugin support".
     */
    UnloadFailed = 1 << 9,

    /**
     * The plugin cannot be unloaded because another plugin is depending on it.
     * Unload that plugin first and try again. Returned by @ref AbstractManager::unload().
     * @partialsupport Not available on platforms without
     *      @ref CORRADE_PLUGINMANAGER_NO_DYNAMIC_PLUGIN_SUPPORT "dynamic plugin support".
     */
    Required = 1 << 10,

    /**
     * @ref AbstractManager::unload() returns this if the plugin has an active
     * instance and cannot be unloaded. Destroy all instances and try again.
     * @ref AbstractManager::load() returns this if loading a file path
     * directly and a plugin with the same name already exists.
     * @partialsupport Not available on platforms without
     *      @ref CORRADE_PLUGINMANAGER_NO_DYNAMIC_PLUGIN_SUPPORT "dynamic plugin support".
     */
    Used = 1 << 11
    #endif
};

/** @debugoperatorenum{LoadState} */
CORRADE_PLUGINMANAGER_EXPORT Utility::Debug& operator<<(Utility::Debug& debug, PluginManager::LoadState value);

/**
@brief Plugin load states

Useful when checking whether @ref LoadState in in given set of values, for
example:

@snippet PluginManager.cpp LoadStates

Note that @ref LoadState::Loaded includes the value of @ref LoadState::Static,
so you can use @cpp loadState & PluginManager::LoadState::Loaded @ce instead of
much more verbose
@cpp loadState & (PluginManager::LoadState::Loaded|PluginManager::LoadState::Static) @ce.
@see @ref AbstractManager::loadState(), @ref AbstractManager::load(),
    @ref AbstractManager::unload()
*/
typedef Containers::EnumSet<LoadState> LoadStates;

CORRADE_ENUMSET_OPERATORS(LoadStates)

/** @debugoperatorenum{LoadStates} */
CORRADE_PLUGINMANAGER_EXPORT Utility::Debug& operator<<(Utility::Debug& debug, PluginManager::LoadStates value);

namespace Implementation {
    struct StaticPlugin;
}

/**
@brief Base for plugin managers

See @ref Manager and @ref plugin-management for more information.
 */
class CORRADE_PLUGINMANAGER_EXPORT AbstractManager {
    friend AbstractPlugin;

    public:
        /** @brief Plugin version */
        static const int Version;

        /** @brief Copying is not allowed */
        AbstractManager(const AbstractManager&) = delete;

        /** @brief Moving is not allowed */
        AbstractManager(AbstractManager&&) = delete;

        /** @brief Copying is not allowed */
        AbstractManager& operator=(const AbstractManager&) = delete;

        /** @brief Moving is not allowed */
        AbstractManager& operator=(AbstractManager&&) = delete;

        /**
         * @brief Plugin interface
         *
         * Only plugins with the same plugin interface string can be used
         * in this plugin manager.
         */
        std::string pluginInterface() const;

        #ifndef CORRADE_PLUGINMANAGER_NO_DYNAMIC_PLUGIN_SUPPORT
        /**
         * @brief Plugin directory
         *
         * @partialsupport Not available on platforms without
         *      @ref CORRADE_PLUGINMANAGER_NO_DYNAMIC_PLUGIN_SUPPORT "dynamic plugin support".
         */
        std::string pluginDirectory() const;

        /**
         * @brief Set another plugin directory
         *
         * Keeps loaded plugins untouched, removes unloaded plugins which are
         * not existing anymore and adds newly found plugins. The directory is
         * expected to be in UTF-8.
         * @partialsupport Not available on platforms without
         *      @ref CORRADE_PLUGINMANAGER_NO_DYNAMIC_PLUGIN_SUPPORT "dynamic plugin support".
         */
        void setPluginDirectory(std::string directory);

        /**
         * @brief Reload plugin directory
         *
         * Convenience equivalent to @cpp setPluginDirectory(pluginDirectory()) @ce.
         * @partialsupport Not available on platforms without
         *      @ref CORRADE_PLUGINMANAGER_NO_DYNAMIC_PLUGIN_SUPPORT "dynamic plugin support".
         */
        void reloadPluginDirectory();
        #endif

        /**
         * @brief Set preferred plugins for given alias
         *
         * By default, if more than one plugin provides given @p alias, one of
         * them is arbitrarily chosen. With this function it's possible to
         * control the behavior. For given @p alias the function goes through
         * the list in @p plugins and uses the first plugin that is available.
         * The @p alias is expected to exist and be defined by plugins in
         * @p plugins. If none of @p plugins is available, nothing is done.
         *
         * Note that after calling @ref setPluginDirectory() or @ref reloadPluginDirectory()
         * this preference gets reset and you may need to call this function
         * again.
         */
        void setPreferredPlugins(const std::string& alias, std::initializer_list<std::string> plugins);

        /**
         * @brief List of all available plugin names
         *
         * Returns a list of names that correspond to concrete unique static or
         * dynamic plugins.
         * @see @ref aliasList()
         */
        std::vector<std::string> pluginList() const;

        /**
         * @brief List of all available alias names
         *
         * In addition to everything returned by @ref pluginList() contains
         * also all plugin aliases.
         */
        std::vector<std::string> aliasList() const;

        /**
         * @brief Plugin metadata
         *
         * Returns pointer to plugin metadata or @cpp nullptr @ce, if given
         * plugin is not found.
         * @see @ref AbstractPlugin::metadata()
         */
        PluginMetadata* metadata(const std::string& plugin);
        const PluginMetadata* metadata(const std::string& plugin) const; /**< @overload */

        /**
         * @brief Load state of a plugin
         *
         * Returns @ref LoadState::Loaded if the plugin is loaded or
         * @ref LoadState::NotLoaded if not. For static plugins returns always
         * @ref LoadState::Static. On failure returns @ref LoadState::NotFound
         * or @ref LoadState::WrongMetadataFile.
         * @see @ref load(), @ref unload()
         */
        LoadState loadState(const std::string& plugin) const;

        /**
         * @brief Load a plugin
         *
         * Returns @ref LoadState::Loaded if the plugin is already loaded or if
         * loading succeeded. For static plugins returns always
         * @ref LoadState::Static. On failure returns @ref LoadState::NotFound,
         * @ref LoadState::WrongPluginVersion,
         * @ref LoadState::WrongInterfaceVersion,
         * @ref LoadState::UnresolvedDependency or @ref LoadState::LoadFailed.
         *
         * If the plugin has any dependencies, they are recursively processed
         * before loading given plugin.
         *
         * If @p plugin is a plugin file path (i.e., ending with a
         * platform-specific extension such as `.so` or `.dll`), it's loaded
         * from given location and, if the loading succeeds, its basename
         * (without extension) is exposed as an available plugin name. It's
         * expected that a plugin with the same name is not already loaded. The
         * plugin will reside in the plugin list as long as it's loaded or,
         * after calling @ref unload() on it, until next call to
         * @ref setPluginDirectory() or @ref reloadPluginDirectory().
         *
         * @note If passing a file path, the implementation expects forward
         *      slashes as directory separators. Use @ref Utility::Directory::fromNativeSeparators()
         *      to convert from platform-specific format.
         *
         * @see @ref unload(), @ref loadState(), @ref Manager::instantiate(),
         *      @ref Manager::loadAndInstantiate()
         * @partialsupport On platforms without
         *      @ref CORRADE_PLUGINMANAGER_NO_DYNAMIC_PLUGIN_SUPPORT "dynamic plugin support"
         *      returns always either @ref LoadState::Static or
         *      @ref LoadState::NotFound.
         */
        LoadState load(const std::string& plugin);

        /**
         * @brief Unload a plugin
         *
         * Returns @ref LoadState::NotLoaded if the plugin is not loaded or
         * unloading succeeded. For static plugins always returns
         * @ref LoadState::Static. On failure returns @ref LoadState::NotFound,
         * @ref LoadState::UnloadFailed, @ref LoadState::Required or
         * @ref LoadState::Used.
         *
         * @see @ref load(), @ref loadState()
         * @partialsupport On platforms without
         *      @ref CORRADE_PLUGINMANAGER_NO_DYNAMIC_PLUGIN_SUPPORT "dynamic plugin support"
         *      returns always either @ref LoadState::Static or
         *      @ref LoadState::NotFound.
         */
        LoadState unload(const std::string& plugin);

        /**
         * @brief Register an external manager for resolving inter-manager dependencies
         * @m_since{2020,06}
         *
         * To be used for loading dependencies from different plugin
         * interfaces. Once registered, the @p manager is expected to stay in
         * scope for the whole lifetime of this instance.
         */
        void registerExternalManager(AbstractManager& manager);

    protected:
        /**
         * @brief Destructor
         *
         * Destroys all plugin instances and unloads all plugins.
         */
        /* Nobody will need to have (and delete) AbstractManager*, thus this is
           faster than public virtual destructor */
        ~AbstractManager();

    #ifdef DOXYGEN_GENERATING_OUTPUT
    private:
    #else
    public:
    #endif
        struct CORRADE_PLUGINMANAGER_LOCAL Plugin;
        typedef void* (*Instancer)(AbstractManager&, const std::string&);
        static void importStaticPlugin(int version, Implementation::StaticPlugin& plugin);
        static void ejectStaticPlugin(int version, Implementation::StaticPlugin& plugin);

    #ifdef DOXYGEN_GENERATING_OUTPUT
    private:
    #else
    protected:
    #endif
        #ifndef CORRADE_PLUGINMANAGER_NO_DYNAMIC_PLUGIN_SUPPORT
        explicit AbstractManager(std::string pluginInterface, const std::vector<std::string>& pluginSearchPaths, std::string pluginSuffix, std::string pluginConfSuffix, std::string pluginDirectory);
        #else
        explicit AbstractManager(std::string pluginInterface, std::string pluginConfSuffix);
        #endif

        Containers::Pointer<AbstractPlugin> instantiateInternal(const std::string& plugin);
        Containers::Pointer<AbstractPlugin> loadAndInstantiateInternal(const std::string& plugin);

    private:
        struct State;

        CORRADE_PLUGINMANAGER_LOCAL void registerDynamicPlugin(const std::string& name, Containers::Pointer<Plugin>&& plugin);

        CORRADE_PLUGINMANAGER_LOCAL void registerInstance(const std::string& plugin, AbstractPlugin& instance, const PluginMetadata*& metadata);
        CORRADE_PLUGINMANAGER_LOCAL void reregisterInstance(const std::string& plugin, AbstractPlugin& oldInstance, AbstractPlugin* newInstance);

        #ifndef CORRADE_PLUGINMANAGER_NO_DYNAMIC_PLUGIN_SUPPORT
        CORRADE_PLUGINMANAGER_LOCAL LoadState loadInternal(Plugin& plugin);
        CORRADE_PLUGINMANAGER_LOCAL LoadState loadInternal(Plugin& plugin, const std::string& filename);
        CORRADE_PLUGINMANAGER_LOCAL LoadState unloadInternal(Plugin& plugin);
        CORRADE_PLUGINMANAGER_LOCAL LoadState unloadRecursiveInternal(Plugin& plugin);
        #endif

        Containers::Pointer<State> _state;
};

namespace Implementation {

struct StaticPlugin {
    /* Assuming both plugin and interface are static strings produced by the
       CORRADE_PLUGIN_REGISTER() macro, so there's no need to make an allocated
       copy of them, just a direct reference */
    const char* plugin;
    const char* interface;
    AbstractManager::Instancer instancer;
    void(*initializer)();
    void(*finalizer)();
    /* This field shouldn't be written to by anything else than
       importStaticPlugin() / ejectStaticPlugin(). It's zero-initilized by
       default and those use it to avoid inserting a single item to the linked
       list more than once. */
    StaticPlugin* next;
};

}

/** @hideinitializer
@brief Import a static plugin
@param name      Static plugin name (the same as defined with
    @ref CORRADE_PLUGIN_REGISTER())

If you link static plugins to your executable, they can't automatically
register themselves at startup to be known to
@ref Corrade::PluginManager::Manager "PluginManager::Manager", and you need to
load them explicitly by calling @ref CORRADE_PLUGIN_IMPORT() at the beginning
of the @cpp main() @ce function. You can also wrap these macro calls in another
function (which will then be compiled into a dynamic library or the main
executable) and use the @ref CORRADE_AUTOMATIC_INITIALIZER() macro for an
automatic call:

@snippet PluginManager.cpp CORRADE_PLUGIN_IMPORT

@attention This macro should be called outside of any namespace. If you are
    running into linker errors with `pluginImporter_*`, this could be the
    reason. See @ref CORRADE_RESOURCE_INITIALIZE() documentation for more
    information.

Functions called by this macro don't do any dynamic allocation or other
operations that could fail, so it's safe to call it even in restricted phases
of application exection. It's also safe to call this macro more than once.

@see @ref CORRADE_PLUGIN_EJECT()
*/
/* This "bundles" CORRADE_RESOURCE_INITIALIZE() in itself. Keep in sync. */
#define CORRADE_PLUGIN_IMPORT(name)                                         \
    extern int pluginImporter_##name();                                     \
    extern int resourceInitializer_##name();                                \
    pluginImporter_##name();                                                \
    resourceInitializer_##name();

/** @hideinitializer
@brief Eject a previously imported static plugin
@param name      Static plugin name (the same as defined with
    @ref CORRADE_PLUGIN_REGISTER())
@m_since{2019,10}

Deregisters a plugin previously registered using @ref CORRADE_PLUGIN_IMPORT().

@attention This macro should be called outside of any namespace. See the
    @ref CORRADE_RESOURCE_INITIALIZE() macro for more information.

Functions called by this macro don't do any dynamic allocation or other
operations that could fail, so it's safe to call it even in restricted phases
of application exection. It's also safe to call this macro more than once.
*/
/* This "bundles" CORRADE_RESOURCE_FINALIZE() in itself. Keep in sync. */
#define CORRADE_PLUGIN_EJECT(name)                                          \
    extern int pluginEjector_##name();                                      \
    extern int resourceFinalizer_##name();                                  \
    pluginEjector_##name();                                                 \
    resourceFinalizer_##name();

/** @brief Plugin version */
#define CORRADE_PLUGIN_VERSION 6

/** @hideinitializer
@brief Register a static or dynamic plugin
@param name          Name of the static plugin (equivalent of dynamic plugin
     filename)
@param className     Plugin class name
@param interface     Interface name (the same as is defined by the
    @ref Corrade::PluginManager::AbstractPlugin::pluginInterface() "pluginInterface()"
    member of given plugin class)

If the plugin is built as **static** (using the CMake
@ref corrade-cmake-add-static-plugin "corrade_add_static_plugin()" command),
registers it, so it will be loaded automatically when PluginManager instance
with corresponding interface is created. When building as static plugin,
`CORRADE_STATIC_PLUGIN` preprocessor directive is defined.

If the plugin is built as **dynamic** (using the CMake
@ref corrade-cmake-add-plugin "corrade_add_plugin()" command), registers it, so
it can be dynamically loaded via @ref Corrade::PluginManager::Manager by
supplying a name of the plugin. When building as dynamic plugin,
`CORRADE_DYNAMIC_PLUGIN` preprocessor directive is defined.

If the plugin is built as dynamic or static **library or executable** (not as
a plugin, using e.g. CMake command @cmake add_library() @ce /
@cmake add_executable() @ce), this macro won't do anything to prevent linker
issues when linking more plugins together. No plugin-related preprocessor
directive is defined.

See @ref plugin-management for more information about plugin compilation.

@attention This macro should be called outside of any namespace. If you are
    running into linker errors with `pluginImporter_`, this could be the
    reason.
*/
#ifdef CORRADE_STATIC_PLUGIN
#define CORRADE_PLUGIN_REGISTER(name, className, interface_)                \
    namespace {                                                             \
        Corrade::PluginManager::Implementation::StaticPlugin staticPlugin_##name; \
    }                                                                       \
    int pluginImporter_##name();                                            \
    int pluginImporter_##name() {                                           \
        staticPlugin_##name.plugin = #name;                                 \
        staticPlugin_##name.interface = interface_;                         \
        staticPlugin_##name.instancer =                                     \
            [](Corrade::PluginManager::AbstractManager& manager, const std::string& plugin) -> void* { \
                return new className(manager, plugin);                      \
            };                                                              \
        staticPlugin_##name.initializer = className::initialize;            \
        staticPlugin_##name.finalizer = className::finalize;                \
        Corrade::PluginManager::AbstractManager::importStaticPlugin(CORRADE_PLUGIN_VERSION, staticPlugin_##name); \
        return 1;                                                           \
    }                                                                       \
    int pluginEjector_##name();                                             \
    int pluginEjector_##name() {                                            \
        Corrade::PluginManager::AbstractManager::ejectStaticPlugin(CORRADE_PLUGIN_VERSION, staticPlugin_##name); \
        return 1;                                                           \
    }
#elif defined(CORRADE_DYNAMIC_PLUGIN)
#define CORRADE_PLUGIN_REGISTER(name, className, interface)                 \
    extern "C" CORRADE_PLUGIN_EXPORT int pluginVersion();                   \
    extern "C" CORRADE_PLUGIN_EXPORT int pluginVersion() { return CORRADE_PLUGIN_VERSION; } \
    extern "C" CORRADE_PLUGIN_EXPORT void* pluginInstancer(Corrade::PluginManager::AbstractManager& manager, const std::string& plugin); \
    extern "C" CORRADE_PLUGIN_EXPORT void* pluginInstancer(Corrade::PluginManager::AbstractManager& manager, const std::string& plugin) \
        { return new className{manager, plugin}; }                          \
    extern "C" CORRADE_PLUGIN_EXPORT void pluginInitializer();              \
    extern "C" CORRADE_PLUGIN_EXPORT void pluginInitializer()               \
        { className::initialize(); }                                        \
    extern "C" CORRADE_PLUGIN_EXPORT void pluginFinalizer();                \
    extern "C" CORRADE_PLUGIN_EXPORT void pluginFinalizer()                 \
        { className::finalize(); }                                          \
    extern "C" CORRADE_PLUGIN_EXPORT const char* pluginInterface();         \
    extern "C" CORRADE_PLUGIN_EXPORT const char* pluginInterface() { return interface; }
#else
#define CORRADE_PLUGIN_REGISTER(name, className, interface)
#endif

}}

#endif
