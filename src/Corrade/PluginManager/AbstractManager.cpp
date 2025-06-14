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

#include "AbstractManager.h"

#include <algorithm> /* std::find() */
#include <map>
#include <set>
#include <sstream>

#include "Corrade/Containers/Array.h"
#include "Corrade/Containers/GrowableArray.h"
#include "Corrade/Containers/EnumSet.hpp"
#include "Corrade/Containers/Optional.h"
#include "Corrade/Containers/Reference.h"
#include "Corrade/Containers/StringIterable.h"
#include "Corrade/Containers/Implementation/RawForwardList.h"
#include "Corrade/PluginManager/AbstractPlugin.h"
#include "Corrade/PluginManager/Implementation/Plugin.h"
#include "Corrade/Utility/Assert.h"
#include "Corrade/Utility/Configuration.h"

#ifndef CORRADE_PLUGINMANAGER_NO_DYNAMIC_PLUGIN_SUPPORT
#include "Corrade/Utility/Path.h"

#ifndef CORRADE_TARGET_WINDOWS
#include <dlfcn.h>
#else
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN /* Otherwise `#define interface struct` breaks everything */
#endif
#include <windows.h>
#include "Corrade/Utility/Unicode.h"
#endif
#endif

#if defined(CORRADE_TARGET_WINDOWS) && defined(CORRADE_BUILD_STATIC_UNIQUE_GLOBALS) && !defined(CORRADE_TARGET_WINDOWS_RT)
#include "Corrade/Utility/Implementation/WindowsWeakSymbol.h"
#endif
#if defined(CORRADE_TARGET_WINDOWS) && !defined(CORRADE_PLUGINMANAGER_NO_DYNAMIC_PLUGIN_SUPPORT)
#include "Corrade/Utility/Implementation/ErrorString.h"
#endif

namespace Corrade { namespace PluginManager {

using namespace Containers::Literals;

struct AbstractManager::State {
    explicit State(Containers::StringView pluginInterface,
        #ifndef CORRADE_PLUGINMANAGER_NO_DYNAMIC_PLUGIN_SUPPORT
        Containers::StringView pluginSuffix,
        #endif
        Containers::StringView pluginMetadataSuffix):
        pluginInterface{pluginInterface},
        #ifndef CORRADE_PLUGINMANAGER_NO_DYNAMIC_PLUGIN_SUPPORT
        pluginSuffix{pluginSuffix},
        #endif
        pluginMetadataSuffix{pluginMetadataSuffix}
    {}

    #ifndef CORRADE_PLUGINMANAGER_NO_DYNAMIC_PLUGIN_SUPPORT
    Containers::String pluginDirectory;
    #endif
    /* These are all global, checked in the AbstractManager constructor */
    Containers::StringView pluginInterface;
    #ifndef CORRADE_PLUGINMANAGER_NO_DYNAMIC_PLUGIN_SUPPORT
    Containers::StringView pluginSuffix;
    #endif
    Containers::StringView pluginMetadataSuffix;

    /* The Plugin itself contains an (owned) name string, thus the key could be
       a StringView if each insertion was done carefully with a view pointing
       into the already-allocated Plugin instance. A plugin can however also be
       loaded under an alias, for which a view would have to be created from
       one of the strings inside the `provides` array, and that's quite a
       complication. So a String is used for implementer sanity. Given that it
       can hold up to 22 bytes in the SSO representation, majority of names
       should fit without allocation. */
    std::map<Containers::String, Containers::Pointer<Implementation::Plugin>> plugins;
    /* Similarly here, at least one Plugin itself contains the (owned) alias
       string. But it's not a 1:1 mapping, meaning that if the key would be a
       StringView pointing to *some* occurence of that alias, it could happen
       that the plugin owning the alias the view points to gets unloaded, and
       to fix that, the key would have to get changed to point to some of the
       remaining occurences. Which is again very complicated, so it's again a
       String. Regarding the SSO case, aliases are generally shorter than names
       so even more of them should fit into 22 bytes. */
    std::map<Containers::String, Implementation::Plugin&> aliases;

    std::set<AbstractManager*> externalManagers;
    #ifndef CORRADE_NO_ASSERT
    std::set<AbstractManager*> externalManagerUsedBy;
    #endif
};

const int AbstractManager::Version = CORRADE_PLUGIN_VERSION;

#if !defined(CORRADE_BUILD_STATIC_UNIQUE_GLOBALS) || defined(CORRADE_TARGET_WINDOWS)
/* (Of course) can't be in an unnamed namespace in order to export it below
   (except for Windows, where we do extern "C" so this doesn't matter) */
namespace {
#endif

/* What the hell is going on here with the #ifdefs?! */
#if !defined(CORRADE_BUILD_STATIC) || !defined(CORRADE_BUILD_STATIC_UNIQUE_GLOBALS) || (defined(CORRADE_BUILD_STATIC_UNIQUE_GLOBALS) && !defined(CORRADE_TARGET_WINDOWS)) || defined(CORRADE_TARGET_WINDOWS_RT)
#ifdef CORRADE_BUILD_STATIC_UNIQUE_GLOBALS
/* On static builds that get linked to multiple shared libraries and then used
   in a single app we want to ensure there's just one global symbol. On Linux
   it's apparently enough to just export, macOS needs the weak attribute. */
CORRADE_VISIBILITY_EXPORT
    #ifdef CORRADE_TARGET_GCC
    __attribute__((weak))
    #else
    /* uh oh? the test will fail, probably */
    #endif
#endif
/* A linked list of static plugins. Managed using utilities from
   Containers/Implementation/RawForwardList.h, look there for more info.

   The value of this variable is guaranteed to be zero-filled even before any
   static plugin initializers are executed, which means we don't hit any static
   initialization order fiasco. */
Implementation::StaticPlugin* globalStaticPlugins = nullptr;
#else
/* On Windows the symbol is exported unmangled and then fetched via
   GetProcAddress() to emulate weak linking. Using an extern "C" block instead
   of just a function annotation because otherwise MinGW prints a warning:
   '...' initialized and declared 'extern' (uh?) */
extern "C" {
    CORRADE_VISIBILITY_EXPORT Implementation::StaticPlugin* corradePluginManagerUniqueGlobalStaticPlugins = nullptr;
}
#endif

#if !defined(CORRADE_BUILD_STATIC_UNIQUE_GLOBALS) || defined(CORRADE_TARGET_WINDOWS)
}
#endif

/* Windows don't have any concept of weak symbols, instead GetProcAddress() on
   GetModuleHandle(nullptr) "emulates" the weak linking as it's guaranteed to
   pick up the same symbol of the final exe independently of the DLL it was
   called from. To avoid #ifdef hell in code below, the globalStaticPlugins are
   redefined to return a value from this uniqueness-ensuring function. */
#if defined(CORRADE_TARGET_WINDOWS) && defined(CORRADE_BUILD_STATIC_UNIQUE_GLOBALS) && !defined(CORRADE_TARGET_WINDOWS_RT)
namespace {

Implementation::StaticPlugin*& windowsGlobalStaticPlugins() {
    /* A function-local static to ensure it's only initialized once without any
       race conditions among threads */
    static Implementation::StaticPlugin** const uniqueGlobals = reinterpret_cast<Implementation::StaticPlugin**>(Utility::Implementation::windowsWeakSymbol("corradePluginManagerUniqueGlobalStaticPlugins", &corradePluginManagerUniqueGlobalStaticPlugins));
    return *uniqueGlobals;
}

}

#define globalStaticPlugins windowsGlobalStaticPlugins()
#endif

static_assert(std::is_standard_layout<Implementation::StaticPlugin>::value &&
    #ifdef CORRADE_NO_STD_IS_TRIVIALLY_TRAITS
    std::has_trivial_default_constructor<Implementation::StaticPlugin>::value
    #else
    std::is_trivially_constructible<Implementation::StaticPlugin>::value
    #endif
,
    "static plugins shouldn't cause any global initialization / finalization to happen on their own");

void AbstractManager::importStaticPlugin(int version, Implementation::StaticPlugin& plugin) {
    CORRADE_ASSERT(version == Version,
        "PluginManager: wrong version of static plugin" << plugin.plugin << Utility::Debug::nospace << ", got" << version << "but expected" << Version, );
    #ifdef CORRADE_NO_ASSERT
    static_cast<void>(version);
    #endif
    Containers::Implementation::forwardListInsert(globalStaticPlugins, plugin);
}

void AbstractManager::ejectStaticPlugin(int version, Implementation::StaticPlugin& plugin) {
    CORRADE_ASSERT(version == Version,
        "PluginManager: wrong version of static plugin" << plugin.plugin << Utility::Debug::nospace << ", got" << version << "but expected" << Version, );
    #ifdef CORRADE_NO_ASSERT
    static_cast<void>(version);
    #endif
    Containers::Implementation::forwardListRemove(globalStaticPlugins, plugin);
}

#ifndef CORRADE_PLUGINMANAGER_NO_DYNAMIC_PLUGIN_SUPPORT
AbstractManager::AbstractManager(const Containers::StringView pluginInterface, const Containers::ArrayView<const Containers::String> pluginSearchPaths, const Containers::StringView pluginSuffix, const Containers::StringView pluginMetadataSuffix, const Containers::StringView pluginDirectory):
    _state{InPlaceInit, pluginInterface, pluginSuffix, pluginMetadataSuffix}
#else
AbstractManager::AbstractManager(const Containers::StringView pluginInterface, const Containers::StringView pluginMetadataSuffix):
    _state{InPlaceInit, pluginInterface, pluginMetadataSuffix}
#endif
{
    CORRADE_ASSERT(pluginInterface.flags() & Containers::StringViewFlag::Global,
        "PluginManager::AbstractPlugin::pluginInterface(): returned view is not global:" << pluginInterface, );
    #ifndef CORRADE_PLUGINMANAGER_NO_DYNAMIC_PLUGIN_SUPPORT
    CORRADE_ASSERT(pluginSuffix.flags() & Containers::StringViewFlag::Global,
        "PluginManager::AbstractPlugin::pluginSuffix(): returned view is not global:" << pluginSuffix, );
    #endif
    CORRADE_ASSERT(pluginMetadataSuffix.flags() & Containers::StringViewFlag::Global,
        "PluginManager::AbstractPlugin::pluginMetadataSuffix(): returned view is not global:" << pluginMetadataSuffix, );

    /* Add static plugins which have the same interface and don't have a
       manager assigned to them (i.e, aren't in the map yet). */
    for(const Implementation::StaticPlugin* staticPlugin = globalStaticPlugins; staticPlugin; staticPlugin = Containers::Implementation::forwardListNext(*staticPlugin)) {
        /* The plugin doesn't belong to this manager, skip it */
        if(staticPlugin->interface != _state->pluginInterface) continue;

        /* Assign the plugin to this manager, parse its metadata and
           initialize it (unless the plugin is metadata-less) */
        Utility::Configuration configuration;
        if(_state->pluginMetadataSuffix) {
            std::istringstream metadata{std::string{reinterpret_cast<const char*>(staticPlugin->metadataData), staticPlugin->metadataSize}};
            configuration = Utility::Configuration{metadata, Utility::Configuration::Flag::ReadOnly};
        }

        /* Insert a plugin placeholder into our list. In rare cursed cases it
           may happen that the same static plugin is present multiple times due
           to being linked to multiple dynamic libraries. That's likely an
           undesired buildsystem issue, but shouldn't be fatal, so print a
           warning and skip it */
        const auto inserted = _state->plugins.emplace(
            /* The plugin name is guaranteed to be a global literal even though
               it's just a const char*, wrap it without copying */
            Containers::String::nullTerminatedView(staticPlugin->plugin),
            nullptr);
        if(!inserted.second) {
            Utility::Warning{} << "PluginManager::Manager: duplicate static plugin" << staticPlugin->plugin << Utility::Debug::nospace << ", ignoring";
            continue;
        }

        /* If it got successfully inserted, create the actual plugin state */
        inserted.first->second.emplace(*staticPlugin, Utility::move(configuration));

        Implementation::Plugin& p = *inserted.first->second;

        p.staticPlugin->initializer();

        /* The plugin is the best version of itself. If there was already
           an alias for this name, replace it. */
        {
            /* Of course std::map::find() would allocate a new string in order
               to find it, prevent that from happening by wrapping it again */
            /** @todo clean this up once we kill std::map */
            const auto alias = _state->aliases.find(Containers::String::nullTerminatedView(staticPlugin->plugin));
            if(alias != _state->aliases.end()) _state->aliases.erase(alias);
            /* And here as well -- wrap the string to avoid a copy */
            /* Libc++ frees the passed Plugin& reference when using
               emplace(), causing double-free memory corruption later.
               Everything is okay with insert(). */
            /** @todo put back emplace() here when libc++ is fixed */
            CORRADE_INTERNAL_ASSERT_OUTPUT(_state->aliases.insert({Containers::String::nullTerminatedView(staticPlugin->plugin), p}).second);
        }

        /* Add aliases to the list (only the ones that aren't already there
           are added) */
        for(const Containers::String& alias: p.provides) {
            /* Libc++ frees the passed Plugin& reference when using
               emplace(), causing double-free memory corruption later.
               Everything is okay with insert(). */
            /** @todo put back emplace() here when libc++ is fixed */
            _state->aliases.insert({alias, p});
        }
    }

    #ifndef CORRADE_PLUGINMANAGER_NO_DYNAMIC_PLUGIN_SUPPORT
    /* If plugin directory is set, use it, otherwise loop through */
    if(pluginDirectory) setPluginDirectory(pluginDirectory);
    else {
        CORRADE_ASSERT(!pluginSearchPaths.isEmpty(),
            "PluginManager::Manager: either pluginDirectory has to be set or T::pluginSearchPaths() is expected to have at least one entry", );

        const Containers::Optional<Containers::String> executableLocation = Utility::Path::executableLocation();
        CORRADE_INTERNAL_ASSERT(executableLocation);
        const Containers::StringView executableDir = Utility::Path::path(*executableLocation);
        for(const Containers::StringView path: pluginSearchPaths) {
            const Containers::String fullPath = Utility::Path::join(executableDir, path);
            if(!Utility::Path::exists(fullPath)) continue;

            setPluginDirectory(fullPath);
            break;
        }

        /* If no hardcoded path exists and plugin directory is "", disable
           plugin discovery as searching in the current directory would almost
           never be what the user wants -- e.g., it would treat
           CorradeUtility.dll as a plugin. Don't print the warning in case
           we have static plugins (the aliases are non-empty) -- in that case
           assume the user might want to only use static plugins. */
        if(!_state->pluginDirectory && _state->aliases.empty())
            Utility::Warning{} << "PluginManager::Manager: none of the plugin search paths in" << pluginSearchPaths << "exists and pluginDirectory was not set, skipping plugin discovery";
    }
    #endif
}

AbstractManager::~AbstractManager() {
    #ifndef CORRADE_NO_ASSERT
    /* Check that this instance is not used for external dependencies anywhere */
    if(!_state->externalManagerUsedBy.empty()) {
        #ifdef CORRADE_GRACEFUL_ASSERT
        for(AbstractManager* manager: _state->externalManagerUsedBy)
            CORRADE_INTERNAL_ASSERT_OUTPUT(manager->_state->externalManagers.erase(this) == 1);
        #endif

        CORRADE_ASSERT(false,
            "PluginManager::Manager: wrong destruction order," << _state->pluginInterface << "plugins still needed by" << _state->externalManagerUsedBy.size() << "other managers for external dependencies", );
    }

    /* Let the external managers know that we're not using them anymore. This
       is done only if asserts are enabled, otherwise the backreference isn't
       used in any way. */
    for(AbstractManager* manager: _state->externalManagers)
        CORRADE_INTERNAL_ASSERT_OUTPUT(manager->_state->externalManagerUsedBy.erase(this) == 1);
    #endif

    /* Unload all plugins */
    for(std::pair<const Containers::String, Containers::Pointer<Implementation::Plugin>>& plugin: _state->plugins) {
        #ifndef CORRADE_PLUGINMANAGER_NO_DYNAMIC_PLUGIN_SUPPORT
        /* Try to unload the plugin (and all plugins that depend on it) */
        unloadRecursiveInternal(*plugin.second);
        #endif

        /* Finalize static plugins before they get removed from the list */
        if(plugin.second->loadState == LoadState::Static)
            plugin.second->staticPlugin->finalizer();
    }
}

#ifndef CORRADE_PLUGINMANAGER_NO_DYNAMIC_PLUGIN_SUPPORT
LoadState AbstractManager::unloadRecursiveInternal(Implementation::Plugin& plugin) {
    /* If the plugin is not static and is used by others, try to unload these
       first so it can be unloaded too. This function is called from the
       destructor after we verified there are no other managers using this for
       external dependencies, so everything the usedBy list is in this manager
       as well */
    while(!plugin.usedBy.isEmpty()) {
        const auto found = _state->plugins.find(plugin.usedBy.front());
        CORRADE_INTERNAL_ASSERT(found != _state->plugins.end());
        return unloadRecursiveInternal(*found->second);
    }

    /* Unload the plugin */
    const LoadState after = unloadInternal(plugin);
    CORRADE_ASSERT(after & (LoadState::Static|LoadState::NotLoaded|LoadState::WrongMetadataFile),
        "PluginManager::Manager: cannot unload plugin" << plugin.name << "on manager destruction:" << after, {});

    return after;
}
#endif

Containers::StringView AbstractManager::pluginInterface() const {
    return _state->pluginInterface;
}

#ifndef CORRADE_PLUGINMANAGER_NO_DYNAMIC_PLUGIN_SUPPORT
Containers::StringView AbstractManager::pluginDirectory() const {
    return _state->pluginDirectory;
}

void AbstractManager::setPluginDirectory(const Containers::StringView directory) {
    _state->pluginDirectory = Containers::String::nullTerminatedGlobalView(directory);

    /* Remove aliases for unloaded plugins from the container. They need to be
       removed before plugins themselves */
    auto ait = _state->aliases.cbegin();
    while(ait != _state->aliases.cend()) {
        if(ait->second.loadState & (LoadState::NotLoaded|LoadState::WrongMetadataFile))
            ait = _state->aliases.erase(ait);
        else ++ait;
    }

    /* Remove all unloaded plugins from the container */
    auto it = _state->plugins.cbegin();
    while(it != _state->plugins.cend()) {
        if(it->second->loadState & (LoadState::NotLoaded|LoadState::WrongMetadataFile))
            it = _state->plugins.erase(it);
        else ++it;
    }

    /* Find plugin files in the directory. Sort the list so we have predictable
       plugin preference behavior for aliases on systems that have random
       directory listing order. */
    /** @todo Currently this preserves original behavior of not complaining
        when the directory doesn't exist, as a lot of existing code and tests
        relies on it. Figure out a better solution. */
    if(Utility::Path::exists(_state->pluginDirectory)) {
        Containers::Optional<Containers::Array<Containers::String>> d = Utility::Path::list(
            _state->pluginDirectory,
            Utility::Path::ListFlag::SkipDirectories|
            Utility::Path::ListFlag::SkipDotAndDotDot|
            Utility::Path::ListFlag::SortAscending);
        if(d) for(const Containers::StringView filename: *d) {
            /* File doesn't have module suffix, continue to next */
            if(!filename.hasSuffix(_state->pluginSuffix))
                continue;

            /* Dig plugin name from filename */
            const Containers::StringView name = filename.exceptSuffix(_state->pluginSuffix);

            /* Skip the plugin if it is among loaded */
            if(_state->plugins.find(name) != _state->plugins.end()) continue;

            registerDynamicPlugin(name, Containers::Pointer<Implementation::Plugin>{InPlaceInit, name,
                _state->pluginMetadataSuffix ? Utility::Path::join(_state->pluginDirectory, name + _state->pluginMetadataSuffix) : Containers::String{}});
        }
    }

    /* If some of the currently loaded plugins aliased plugins that were in the
       old plugin directory, these are no longer there. Refresh the alias list
       with the new plugins. */
    for(std::pair<const Containers::String, Containers::Pointer<Implementation::Plugin>>& p: _state->plugins) {
        /* Add aliases to the list (only the ones that aren't already there are
           added, calling insert() won't overwrite the existing value) */
        for(const Containers::String& alias: p.second->provides) {
            /* Libc++ frees the passed Plugin& reference when using emplace(),
               causing double-free memory corruption later. Everything is okay
               with insert(). */
            /** @todo put back emplace() here when libc++ is fixed */
            _state->aliases.insert({alias, *p.second});
        }
    }
}

void AbstractManager::reloadPluginDirectory() {
    setPluginDirectory(pluginDirectory());
}
#endif

void AbstractManager::setPreferredPlugins(const Containers::StringView alias, const Containers::StringIterable& plugins) {
    /* Of course std::map::find() would allocate a new String in order to
       find it, prevent that from happening by wrapping a view */
    /** @todo clean this up once we kill std::map */
    auto foundAlias = _state->aliases.find(Containers::String::nullTerminatedView(alias));
    CORRADE_ASSERT(foundAlias != _state->aliases.end(),
        "PluginManager::Manager::setPreferredPlugins():" << alias << "is not a known alias", );

    /* Replace the alias with the first candidate that exists */
    for(const Containers::StringView plugin: plugins) {
        /* Of course std::map::find() would allocate a new String in order to
           find it, prevent that from happening by wrapping a view */
        /** @todo clean this up once we kill std::map */
        auto foundPlugin = _state->plugins.find(Containers::String::nullTerminatedView(plugin));
        if(foundPlugin == _state->plugins.end())
            continue;

        CORRADE_ASSERT(std::find(foundPlugin->second->provides.begin(), foundPlugin->second->provides.end(), alias) != foundPlugin->second->provides.end(),
            "PluginManager::Manager::setPreferredPlugins():" << plugin << "does not provide" << alias, );
        _state->aliases.erase(foundAlias);
        /* Libc++ frees the passed Plugin& reference when using emplace(),
           causing double-free memory corruption later. Everything is okay with
           insert(). */
        /** @todo put back emplace() here when libc++ is fixed */
        _state->aliases.insert({Containers::String::nullTerminatedGlobalView(alias), *foundPlugin->second});
        break;
    }
}

Containers::Array<Containers::StringView> AbstractManager::pluginList() const {
    Containers::Array<Containers::StringView> names{NoInit, _state->plugins.size()};
    std::size_t i = 0;
    for(const std::pair<const Containers::String, Containers::Pointer<Implementation::Plugin>>& plugin: _state->plugins)
        new(&names[i++]) Containers::StringView{plugin.first};
    return names;
}

Containers::Array<Containers::StringView> AbstractManager::aliasList() const {
    Containers::Array<Containers::StringView> names{NoInit, _state->aliases.size()};
    std::size_t i = 0;
    for(const std::pair<const Containers::String, Implementation::Plugin&>& alias: _state->aliases)
        new(&names[i++]) Containers::StringView{alias.first};
    return names;
}

const PluginMetadata* AbstractManager::metadata(const Containers::StringView plugin) const {
    /* Of course std::map::find() would allocate a new String in order to
       find it, prevent that from happening by wrapping a view */
    /** @todo clean this up once we kill std::map */
    auto found = _state->aliases.find(Containers::String::nullTerminatedView(plugin));
    if(found != _state->aliases.end()) return &found->second;

    return nullptr;
}

PluginMetadata* AbstractManager::metadata(const Containers::StringView plugin) {
    /* Of course std::map::find() would allocate a new String in order to
       find it, prevent that from happening by wrapping a view */
    /** @todo clean this up once we kill std::map */
    auto found = _state->aliases.find(Containers::String::nullTerminatedView(plugin));
    if(found != _state->aliases.end()) return &found->second;

    return nullptr;
}

LoadState AbstractManager::loadState(const Containers::StringView plugin) const {
    /* Of course std::map::find() would allocate a new String in order to
       find it, prevent that from happening by wrapping a view */
    /** @todo clean this up once we kill std::map */
    auto found = _state->aliases.find(Containers::String::nullTerminatedView(plugin));
    if(found != _state->aliases.end()) return found->second.loadState;

    return LoadState::NotFound;
}

LoadState AbstractManager::load(const Containers::StringView plugin) {
    #ifndef CORRADE_PLUGINMANAGER_NO_DYNAMIC_PLUGIN_SUPPORT
    /* File path passed, load directly */
    if(plugin.hasSuffix(_state->pluginSuffix)) {
        /* Dig plugin name from filename and verify it's not loaded at the moment */
        const Containers::StringView filename = Utility::Path::filename(plugin);
        const Containers::StringView name = filename.exceptSuffix(_state->pluginSuffix.size());
        /* std::map::find() would allocate a new String in order to find it,
           unfortunately as the name slice is not null-terminated we can't
           prevent anything with String::nullTerminatedView() */
        /** @todo clean this up once we kill std::map */
        const auto found = _state->plugins.find(name);
        if(found != _state->plugins.end() && (found->second->loadState & LoadState::Loaded)) {
            Utility::Error{} << "PluginManager::load():" << filename << "conflicts with currently loaded plugin of the same name";
            return LoadState::Used;
        }

        /* Load the plugin and register it only if loading succeeded so we
           don't crap the alias state. If there's already a registered
           plugin of this name, replace it. */
        Containers::Pointer<Implementation::Plugin> data{InPlaceInit, name,
            _state->pluginMetadataSuffix ? Utility::Path::join(Utility::Path::path(plugin), name + _state->pluginMetadataSuffix) : Containers::String{}};
        /* Explicitly join the filename with current working directory, since
           that's how the metadata were loaded above. Without that, the OS APIs
           would search in LD_LIBRARY_PATH and C:/Windows/system32 and
           elsewhere, with current directory being considered either quite late
           (step 5 on Windows https://docs.microsoft.com/en-us/windows/win32/dlls/dynamic-link-library-search-order#standard-search-order-for-desktop-applications)
           or not at all (with dlopen()). This should also prevent some random
           other plugin binary being picked up by accident.

           On the other hand, the implicit behavior wouldn't make sense anyway
           because the metadata are loaded from a completely different
           location. */
        /** @todo this assumes currentDirectory() doesn't fail, which it might
            if it gets deleted underneath a running program -- once Utility::Path::isAbsolute() exists, use that to query it only when
            needed and then fail gracefully in that case */
        const LoadState state = loadInternal(*data, Utility::Path::join(*Utility::Path::currentDirectory(), plugin));
        if(state & LoadState::Loaded) {
            /* Remove the potential plugin with the same name (we already
               checked above that it's *not* loaded) */
            if(found != _state->plugins.end()) {
                /* Erase all aliases that reference this plugin, as they would
                   be dangling now. */
                auto ait = _state->aliases.cbegin();
                while(ait != _state->aliases.cend()) {
                    if(&ait->second == found->second.get())
                        ait = _state->aliases.erase(ait);
                    else ++ait;
                }

                /* Erase the plugin from the plugin map */
                _state->plugins.erase(found);
            }

            registerDynamicPlugin(name, Utility::move(data));
        }
        return state;
    }
    #endif

    /* Of course std::map::find() would allocate a new String in order to
       find it, prevent that from happening by wrapping a view */
    /** @todo clean this up once we kill std::map */
    auto found = _state->aliases.find(Containers::String::nullTerminatedView(plugin));
    if(found != _state->aliases.end()) {
        #ifndef CORRADE_PLUGINMANAGER_NO_DYNAMIC_PLUGIN_SUPPORT
        return loadInternal(found->second);
        #else
        return found->second.loadState;
        #endif
    }

    Utility::Error() << "PluginManager::Manager::load(): plugin" << plugin
        #ifndef CORRADE_PLUGINMANAGER_NO_DYNAMIC_PLUGIN_SUPPORT
        << "is not static and was not found in" << _state->pluginDirectory;
        #else
        << "was not found";
        #endif
    return LoadState::NotFound;
}

#ifndef CORRADE_PLUGINMANAGER_NO_DYNAMIC_PLUGIN_SUPPORT
LoadState AbstractManager::loadInternal(Implementation::Plugin& plugin) {
    return loadInternal(plugin, Utility::Path::join(_state->pluginDirectory, plugin.name + _state->pluginSuffix));
}

LoadState AbstractManager::loadInternal(Implementation::Plugin& plugin, Containers::StringView filename) {
    /* Plugin is not ready to load */
    if(plugin.loadState != LoadState::NotLoaded) {
        if(!(plugin.loadState & (LoadState::Static|LoadState::Loaded)))
            Utility::Error{} << "PluginManager::Manager::load(): plugin" << plugin.name << "is not ready to load:" << plugin.loadState;
        return plugin.loadState;
    }

    /* Load dependencies and remember their names for later. Their names will
       be added to usedBy list only if everything goes well. */
    Containers::Array<Containers::Reference<Implementation::Plugin>> dependencies;
    arrayReserve(dependencies, plugin.depends.size());
    for(const Containers::String& dependency: plugin.depends) {
        /* If the dependency is not in our plugin manager, check the registered
           external managers as well */
        AbstractManager* dependencyManager = nullptr;
        auto foundDependency = _state->plugins.find(dependency);
        if(foundDependency != _state->plugins.end())
            dependencyManager = this;
        else for(AbstractManager* other: _state->externalManagers) {
            foundDependency = other->_state->plugins.find(dependency);
            if(foundDependency != other->_state->plugins.end()) {
                dependencyManager = other;
                break;
            }
        }

        if(!dependencyManager || !(dependencyManager->loadInternal(*foundDependency->second) & LoadState::Loaded)) {
            Utility::Error{} << "PluginManager::Manager::load(): unresolved dependency" << dependency << "of plugin" << plugin.name;
            return LoadState::UnresolvedDependency;
        }

        arrayAppend(dependencies, *foundDependency->second);
    }

    /* Open plugin file, make symbols globally available for next libs (which
       may depend on this) */
    #ifndef CORRADE_TARGET_WINDOWS
    void* module = dlopen(Containers::String::nullTerminatedView(filename).data(), RTLD_NOW|RTLD_GLOBAL);
    #else
    HMODULE module = LoadLibraryW(Utility::Unicode::widen(filename));
    #endif
    if(!module) {
        Utility::Error err;
        err << "PluginManager::Manager::load(): cannot load plugin"
            << plugin.name << "from \"" << Utility::Debug::nospace
            << filename << Utility::Debug::nospace << "\":";
        #ifndef CORRADE_TARGET_WINDOWS
        err << dlerror();
        #else
        Utility::Implementation::printWindowsErrorString(err, GetLastError());
        #endif
        return LoadState::LoadFailed;
    }

    /* Check plugin version */
    /* MinGW GCC 8+ warns that "cast between incompatible function types from
       ‘FARPROC’ to ‘int (*)()’", the __extension__ doesn't help. OTOH, on
       Linux without the __extension__ it causes a -Wpedantic warning on GCC
       4.8 (and perhaps newer), so we need both. */
    #if defined(CORRADE_TARGET_MINGW) && !defined(CORRADE_TARGET_CLANG) && __GNUC__ >= 8
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wcast-function-type"
    #endif
    #ifdef CORRADE_TARGET_GCC
    __extension__ /* http://web.archive.org/web/20160826013457/http://www.mr-edd.co.uk/blog/supressing_gcc_warnings */
    #endif
    int (*version)() = reinterpret_cast<int(*)()>(
        #ifndef CORRADE_TARGET_WINDOWS
        dlsym
        #else
        GetProcAddress
        #endif
        (module, "pluginVersion"));
    #if defined(CORRADE_TARGET_MINGW) && !defined(CORRADE_TARGET_CLANG) && __GNUC__ >= 8
    #pragma GCC diagnostic pop
    #endif
    if(version == nullptr) {
        Utility::Error err;
        err << "PluginManager::Manager::load(): cannot get version of plugin"
            << plugin.name << Utility::Debug::nospace << ":";
        #ifndef CORRADE_TARGET_WINDOWS
        err << dlerror();
        #else
        Utility::Implementation::printWindowsErrorString(err, GetLastError());
        #endif
        #ifndef CORRADE_TARGET_WINDOWS
        dlclose(module);
        #else
        FreeLibrary(module);
        #endif
        return LoadState::LoadFailed;
    }
    if(version() != Version) {
        Utility::Error{} << "PluginManager::Manager::load(): wrong version of plugin"
                << plugin.name << Utility::Debug::nospace << ", expected"
                << Version << "but got" << version();
        #ifndef CORRADE_TARGET_WINDOWS
        dlclose(module);
        #else
        FreeLibrary(module);
        #endif
        return LoadState::WrongPluginVersion;
    }

    /* Check interface string */
    #if defined(CORRADE_TARGET_MINGW) && !defined(CORRADE_TARGET_CLANG) && __GNUC__ >= 8
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wcast-function-type" /* see above */
    #endif
    #ifdef CORRADE_TARGET_GCC
    __extension__ /* see above */
    #endif
    const char* (*interface)() = reinterpret_cast<const char* (*)()>(
        #ifndef CORRADE_TARGET_WINDOWS
        dlsym
        #else
        GetProcAddress
        #endif
        (module, "pluginInterface"));
    #if defined(CORRADE_TARGET_MINGW) && !defined(CORRADE_TARGET_CLANG) && __GNUC__ >= 8
    #pragma GCC diagnostic pop
    #endif
    if(interface == nullptr) {
        Utility::Error err;
        err << "PluginManager::Manager::load(): cannot get interface string of plugin"
            << plugin.name << Utility::Debug::nospace << ":";
        #ifndef CORRADE_TARGET_WINDOWS
        err << dlerror();
        #else
        Utility::Implementation::printWindowsErrorString(err, GetLastError());
        #endif
        #ifndef CORRADE_TARGET_WINDOWS
        dlclose(module);
        #else
        FreeLibrary(module);
        #endif
        return LoadState::LoadFailed;
    }
    if(interface() != pluginInterface()) {
        Utility::Error{} << "PluginManager::Manager::load(): wrong interface string of plugin" << plugin.name + ", expected" << pluginInterface() << "but got" << interface();
        #ifndef CORRADE_TARGET_WINDOWS
        dlclose(module);
        #else
        FreeLibrary(module);
        #endif
        return LoadState::WrongInterfaceVersion;
    }

    /* Load plugin initializer */
    #if defined(CORRADE_TARGET_MINGW) && !defined(CORRADE_TARGET_CLANG) && __GNUC__ >= 8
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wcast-function-type" /* see above */
    #endif
    #ifdef CORRADE_TARGET_GCC
    __extension__ /* see above */
    #endif
    void(*initializer)() = reinterpret_cast<void(*)()>(
        #ifndef CORRADE_TARGET_WINDOWS
        dlsym
        #else
        GetProcAddress
        #endif
        (module, "pluginInitializer"));
    #if defined(CORRADE_TARGET_MINGW) && !defined(CORRADE_TARGET_CLANG) && __GNUC__ >= 8
    #pragma GCC diagnostic pop
    #endif
    if(initializer == nullptr) {
        Utility::Error err;
        err << "PluginManager::Manager::load(): cannot get initializer of plugin"
            << plugin.name + ":";
        #ifndef CORRADE_TARGET_WINDOWS
        err << dlerror();
        #else
        Utility::Implementation::printWindowsErrorString(err, GetLastError());
        #endif
        #ifndef CORRADE_TARGET_WINDOWS
        dlclose(module);
        #else
        FreeLibrary(module);
        #endif
        return LoadState::LoadFailed;
    }

    /* Load plugin finalizer */
    #if defined(CORRADE_TARGET_MINGW) && !defined(CORRADE_TARGET_CLANG) && __GNUC__ >= 8
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wcast-function-type" /* see above */
    #endif
    #ifdef CORRADE_TARGET_GCC
    __extension__ /* see above */
    #endif
    void(*finalizer)() = reinterpret_cast<void(*)()>(
        #ifndef CORRADE_TARGET_WINDOWS
        dlsym
        #else
        GetProcAddress
        #endif
        (module, "pluginFinalizer"));
    #if defined(CORRADE_TARGET_MINGW) && !defined(CORRADE_TARGET_CLANG) && __GNUC__ >= 8
    #pragma GCC diagnostic pop
    #endif
    if(finalizer == nullptr) {
        Utility::Error err;
        err << "PluginManager::Manager::load(): cannot get finalizer of plugin"
            << plugin.name + ":";
        #ifndef CORRADE_TARGET_WINDOWS
        err << dlerror();
        #else
        Utility::Implementation::printWindowsErrorString(err, GetLastError());
        #endif
        #ifndef CORRADE_TARGET_WINDOWS
        dlclose(module);
        #else
        FreeLibrary(module);
        #endif
        return LoadState::LoadFailed;
    }

    /* Load plugin instancer */
    #if defined(CORRADE_TARGET_MINGW) && !defined(CORRADE_TARGET_CLANG) && __GNUC__ >= 8
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wcast-function-type" /* see above */
    #endif
    #ifdef CORRADE_TARGET_GCC
    __extension__ /* see above */
    #endif
    void*(*instancer)(AbstractManager&, const Containers::StringView&) = reinterpret_cast<void*(*)(AbstractManager&, const Containers::StringView&)>(
        #ifndef CORRADE_TARGET_WINDOWS
        dlsym
        #else
        GetProcAddress
        #endif
        (module, "pluginInstancer"));
    #if defined(CORRADE_TARGET_MINGW) && !defined(CORRADE_TARGET_CLANG) && __GNUC__ >= 8
    #pragma GCC diagnostic pop
    #endif
    if(instancer == nullptr) {
        Utility::Error err;
        err << "PluginManager::Manager::load(): cannot get instancer of plugin"
            << plugin.name + ":";
        #ifndef CORRADE_TARGET_WINDOWS
        err << dlerror();
        #else
        Utility::Implementation::printWindowsErrorString(err, GetLastError());
        #endif
        #ifndef CORRADE_TARGET_WINDOWS
        dlclose(module);
        #else
        FreeLibrary(module);
        #endif
        return LoadState::LoadFailed;
    }

    /* Initialize plugin */
    initializer();

    /* Everything is okay, add this plugin to usedBy list of each dependency */
    for(Implementation::Plugin& dependency: dependencies)
        arrayAppend(dependency.usedBy, plugin.name);

    /* Update plugin object, set state to loaded */
    plugin.loadState = LoadState::Loaded;
    plugin.module = module;
    plugin.instancer = instancer;
    plugin.finalizer = finalizer;
    return LoadState::Loaded;
}
#endif

LoadState AbstractManager::unload(const Containers::StringView plugin) {
    /* Of course std::map::find() would allocate a new String in order to
       find it, prevent that from happening by wrapping a view */
    /** @todo clean this up once we kill std::map */
    auto found = _state->aliases.find(Containers::String::nullTerminatedView(plugin));
    if(found != _state->aliases.end()) {
        #ifndef CORRADE_PLUGINMANAGER_NO_DYNAMIC_PLUGIN_SUPPORT
        return unloadInternal(found->second);
        #else
        return found->second.loadState;
        #endif
    }

    Utility::Error() << "PluginManager::Manager::unload(): plugin" << plugin << "was not found";
    return LoadState::NotFound;
}

#ifndef CORRADE_PLUGINMANAGER_NO_DYNAMIC_PLUGIN_SUPPORT
LoadState AbstractManager::unloadInternal(Implementation::Plugin& plugin) {
    /* Plugin is not ready to unload, nothing to do. The only thing this can
       happen is when the plugin is static or not loaded (which is fine, so we
       just return that load state) or when its metadata file is broken (which
       is not good, but what can we do). All other states (such as UnloadFailed
       etc.) are transient -- not saved into the local state, only returned. */
    if(plugin.loadState != LoadState::Loaded) {
        CORRADE_INTERNAL_ASSERT(plugin.loadState & (LoadState::Static|LoadState::NotLoaded|LoadState::WrongMetadataFile));
        return plugin.loadState;
    }

    /* Plugin is used by another plugin, don't unload */
    if(!plugin.usedBy.isEmpty()) {
        Utility::Error{} << "PluginManager::Manager::unload(): plugin"
            << plugin.name << "is required by other plugins:" << plugin.usedBy;
        return LoadState::Required;
    }

    /* Plugin has active instances */
    if(!plugin.instances.empty()) {
        /* Check if all instances can be safely deleted */
        for(AbstractPlugin* const instance: plugin.instances)
            if(!instance->canBeDeleted()) {
                Utility::Error{} << "PluginManager::Manager::unload(): plugin"
                    << plugin.name << "has a live instance which cannot be automatically deleted";
                return LoadState::Used;
            }

        /* If they can be, delete them. They remove itself from instances
           list on destruction, thus going backwards */
        for(std::size_t i = plugin.instances.size(); i != 0; --i)
            delete plugin.instances[i-1];
    }

    /* Remove this plugin from "used by" list of dependencies. If not found
       in this manager, try in registered external managers. */
    for(auto it = plugin.depends.cbegin(); it != plugin.depends.cend(); ++it) {
        Implementation::Plugin* dependency = nullptr;
        auto foundDependency = _state->plugins.find(*it);
        if(foundDependency != _state->plugins.end())
            dependency = foundDependency->second.get();
        else for(AbstractManager* other: _state->externalManagers) {
            foundDependency = other->_state->plugins.find(*it);
            if(foundDependency != other->_state->plugins.end()) {
                dependency = foundDependency->second.get();
                break;
            }
        }
        CORRADE_INTERNAL_ASSERT(dependency);

        auto found = std::find(dependency->usedBy.begin(), dependency->usedBy.end(), plugin.name);
        CORRADE_INTERNAL_ASSERT(found != dependency->usedBy.end());
        arrayRemove(dependency->usedBy, found - dependency->usedBy.begin());
    }

    /* Finalize plugin */
    plugin.finalizer();

    /* Close the module */
    #ifndef CORRADE_TARGET_WINDOWS
    if(dlclose(plugin.module) != 0)
    #else
    if(!FreeLibrary(plugin.module))
    #endif
    {
        /* This is hard to test, the only possibility I can think of is
           dlclose() when a symbol is still needed (by another plugin, e.g.),
           but that's possible only on QNX, on linux dlclose() only unloads the
           library if it's really not needed. Source:
           https://stackoverflow.com/questions/28882298/error-on-dlclose-shared-objects-still-referenced */
        Utility::Error err;
        err << "PluginManager::Manager::unload(): cannot unload plugin"
            << plugin.name << Utility::Debug::nospace << ":";
        #ifndef CORRADE_TARGET_WINDOWS
        err << dlerror();
        #else
        Utility::Implementation::printWindowsErrorString(err, GetLastError());
        #endif
        plugin.loadState = LoadState::NotLoaded;
        return LoadState::UnloadFailed;
    }

    /* Update plugin object, set state to not loaded */
    plugin.loadState = LoadState::NotLoaded;
    plugin.module = nullptr;
    plugin.instancer = nullptr;
    plugin.finalizer = nullptr;
    return LoadState::NotLoaded;
}
#endif

void AbstractManager::registerExternalManager(AbstractManager& manager) {
    _state->externalManagers.insert(&manager);
    #ifndef CORRADE_NO_ASSERT
    manager._state->externalManagerUsedBy.insert(this);
    #endif
}

void AbstractManager::registerDynamicPlugin(const Containers::StringView name, Containers::Pointer<Implementation::Plugin>&& plugin) {
    /* Insert the plugin to the map. The name is never null terminated so even
       if it would be global in case of an unlikely scenario of coming from a
       filename as a string view literal passed directly to load(), we won't
       save anything */
    const auto result = _state->plugins.emplace(name, Utility::move(plugin));
    CORRADE_INTERNAL_ASSERT(result.second);

    /* The plugin is the best version of itself. If there was already an
       alias for this name, replace it. */
    {
        /* std::map::find() would allocate a new String in order to find it,
           unfortunately as the name slice is never null-terminated we can't
           prevent anything with String::nullTerminatedView() */
        /** @todo clean this up once we kill std::map */
        const auto alias = _state->aliases.find(name);
        if(alias != _state->aliases.end()) _state->aliases.erase(alias);
        CORRADE_INTERNAL_ASSERT_OUTPUT(_state->aliases.insert({name, *result.first->second}).second);
    }

    /* Add aliases to the list. Calling insert() won't overwrite the
       existing value, which ensures that the above note is still held. */
    for(const Containers::String& alias: result.first->second->provides) {
        /* Libc++ frees the passed Plugin& reference when using emplace(),
           causing double-free memory corruption later. Everything is okay
           with insert(). */
        /** @todo put back emplace() here when libc++ is fixed */
        _state->aliases.insert({alias, *result.first->second});
    }
}

/* This function takes an alias name, since at the time of instantiation the
   real plugin name is not yet known */
void AbstractManager::registerInstance(const Containers::StringView plugin, AbstractPlugin& instance, const PluginMetadata*& metadata) {
    /* Of course std::map::find() would allocate a new String in order to
       find it, prevent that from happening by wrapping a view */
    /** @todo clean this up once we kill std::map */
    /** @todo assert proper interface */
    auto found = _state->aliases.find(Containers::String::nullTerminatedView(plugin));
    CORRADE_ASSERT(found != _state->aliases.end(),
        "PluginManager::AbstractPlugin::AbstractPlugin(): attempt to register instance of plugin not known to given manager", );

    found->second.instances.push_back(&instance);
    metadata = &found->second;
}

/* This function however takes the real name, taken from the metadata. This is
   done in order to avoid a nasty interaction with setPreferredPlugins() and
   potential other APIs that redirect an alias to some other plugin, which
   would then lead to the instance not being found */
void AbstractManager::reregisterInstance(const Containers::StringView plugin, AbstractPlugin& oldInstance, AbstractPlugin* const newInstance) {
    /* Of course std::map::find() would allocate a new String in order to
       find it, prevent that from happening by wrapping a view */
    /** @todo clean this up once we kill std::map */
    auto found = _state->plugins.find(Containers::String::nullTerminatedView(plugin));
    CORRADE_INTERNAL_ASSERT(found != _state->plugins.end());

    auto pos = std::find(found->second->instances.begin(), found->second->instances.end(), &oldInstance);
    CORRADE_INTERNAL_ASSERT(pos != found->second->instances.end());

    /* If the plugin is being moved, replace the instance pointer. Otherwise
       remove it from the list, and if the list is empty, delete it fully. */
    if(newInstance) *pos = newInstance;
    else found->second->instances.erase(pos);
}

Containers::Pointer<AbstractPlugin> AbstractManager::instantiateInternal(const Containers::StringView plugin) {
    /* Of course std::map::find() would allocate a new String in order to
       find it, prevent that from happening by wrapping a view */
    /** @todo clean this up once we kill std::map */
    auto found = _state->aliases.find(Containers::String::nullTerminatedView(plugin));

    CORRADE_ASSERT(found != _state->aliases.end() && (found->second.loadState & LoadState::Loaded),
        "PluginManager::Manager::instantiate(): plugin" << plugin << "is not loaded", nullptr);

    return Containers::pointer(static_cast<AbstractPlugin*>(found->second.instancer(*this, plugin)));
}

Containers::Pointer<AbstractPlugin> AbstractManager::loadAndInstantiateInternal(const Containers::StringView plugin) {
    if(!(load(plugin) & LoadState::Loaded)) return nullptr;

    #ifndef CORRADE_PLUGINMANAGER_NO_DYNAMIC_PLUGIN_SUPPORT
    /* If file path passed, instantiate extracted name instead */
    if(Containers::StringView{plugin}.hasSuffix(_state->pluginSuffix)) {
        const Containers::StringView name = Utility::Path::filename(plugin).exceptSuffix(_state->pluginSuffix);
        auto found = _state->aliases.find(name);
        CORRADE_INTERNAL_ASSERT(found != _state->aliases.end());
        return Containers::pointer(static_cast<AbstractPlugin*>(found->second.instancer(*this, name)));
    }
    #endif

    auto found = _state->aliases.find(plugin);
    CORRADE_INTERNAL_ASSERT(found != _state->aliases.end());
    return Containers::pointer(static_cast<AbstractPlugin*>(found->second.instancer(*this, plugin)));
}

AbstractManager* AbstractManager::externalManagerInternal(const Containers::StringView pluginInterface) {
    CORRADE_ASSERT(pluginInterface,
        "PluginManager::Manager::externalManager(): can only retrieve managers with a non-empty plugin interface", {});
    for(AbstractManager* manager: _state->externalManagers)
        if(manager->pluginInterface() == pluginInterface) return manager;
    return nullptr;
}

namespace Implementation {

Plugin::Plugin(Utility::Configuration&& configuration, Containers::String&& name, void*(*instancer)(AbstractManager&, const Containers::StringView&)):
    #ifdef CORRADE_PLUGINMANAGER_NO_DYNAMIC_PLUGIN_SUPPORT
    loadState{LoadState::Static},
    #endif
    metadata{Utility::move(configuration)}, name{Utility::move(name)}, instancer{instancer}
{
    /* Dependencies, aliases */
    /** @todo clean up once Configuration is reworked to have values() return a
        (String)Iterable or whatever */
    {
        const std::vector<Containers::StringView> depends = metadata.values<Containers::StringView>("depends");
        this->depends = Containers::Array<Containers::String>{ValueInit, depends.size()};
        for(std::size_t i = 0; i != depends.size(); ++i)
            this->depends[i] = depends[i];
    } {
        const std::vector<Containers::StringView> provides = metadata.values<Containers::StringView>("provides");
        this->provides = Containers::Array<Containers::String>{ValueInit, provides.size()};
        for(std::size_t i = 0; i != provides.size(); ++i)
            this->provides[i] = provides[i];
    }

    /* Plugin data, configuration */
    /** @todo clean up once Configuration is reworked to have group() either
        return or create the group */
    if(!(data = metadata.group("data")))
        data = metadata.addGroup("data");
    if(!(this->configuration = metadata.group("configuration")))
        this->configuration = metadata.addGroup("configuration");
}

#ifndef CORRADE_PLUGINMANAGER_NO_DYNAMIC_PLUGIN_SUPPORT
Plugin::Plugin(const Containers::StringView name, const Containers::StringView metadata): Plugin{Utility::Configuration{metadata, Utility::Configuration::Flag::ReadOnly}, Containers::String::nullTerminatedGlobalView(name), nullptr} {
    module = nullptr;

    /* If the path is empty, we don't use any plugin configuration file, so
       don't do any checks. */
    if(!metadata) {
        loadState = LoadState::NotLoaded;
    } else {
        /** @todo ahem, this brancing seems a bit convoluted, error if
            configuration *is* valid?! */
        if(this->metadata.isValid()) {
            if(Utility::Path::exists(metadata)) {
                loadState = LoadState::NotLoaded;
                return;
            }

            Utility::Error{} << "PluginManager::Manager:" << metadata << "was not found";
        }

        loadState = LoadState::WrongMetadataFile;
    }
}
#endif

/* staticPlugin.plugin is a pointer to a global string literal, make use of
   that to avoid a copy */
Plugin::Plugin(const StaticPlugin& staticPlugin, Utility::Configuration&& configuration): Plugin{Utility::move(configuration), Containers::String::nullTerminatedGlobalView(Containers::StringView{staticPlugin.plugin, Containers::StringViewFlag::Global}), staticPlugin.instancer} {
    #ifndef CORRADE_PLUGINMANAGER_NO_DYNAMIC_PLUGIN_SUPPORT
    loadState = LoadState::Static;
    #endif
    this->staticPlugin = &staticPlugin;
}

}

Utility::Debug& operator<<(Utility::Debug& debug, LoadState value) {
    switch(value) {
        /* LCOV_EXCL_START */
        #define _c(value) case LoadState::value: return debug << "PluginManager::LoadState::" #value;
        _c(NotFound)
        #ifndef CORRADE_PLUGINMANAGER_NO_DYNAMIC_PLUGIN_SUPPORT
        _c(WrongPluginVersion)
        _c(WrongInterfaceVersion)
        _c(WrongMetadataFile)
        _c(UnresolvedDependency)
        _c(LoadFailed)
        _c(Loaded)
        _c(NotLoaded)
        _c(UnloadFailed)
        _c(Required)
        #endif
        _c(Static)
        #ifndef CORRADE_PLUGINMANAGER_NO_DYNAMIC_PLUGIN_SUPPORT
        _c(Used)
        #endif
        #undef _c
        /* LCOV_EXCL_STOP */
    }

    return debug << "PluginManager::LoadState(" << Utility::Debug::nospace << Utility::Debug::hex << std::uint16_t(value) << Utility::Debug::nospace << ")";
}

Utility::Debug& operator<<(Utility::Debug& debug, const LoadStates value) {
    return Containers::enumSetDebugOutput(debug, value, "PluginManager::LoadStates{}", {
        LoadState::NotFound,
        #ifndef CORRADE_PLUGINMANAGER_NO_DYNAMIC_PLUGIN_SUPPORT
        LoadState::WrongPluginVersion,
        LoadState::WrongInterfaceVersion,
        LoadState::WrongMetadataFile,
        LoadState::UnresolvedDependency,
        LoadState::LoadFailed,
        LoadState::Loaded,
        LoadState::NotLoaded,
        LoadState::UnloadFailed,
        LoadState::Required,
        #endif
        LoadState::Static,
        #ifndef CORRADE_PLUGINMANAGER_NO_DYNAMIC_PLUGIN_SUPPORT
        LoadState::Used
        #endif
        });
}

}}
