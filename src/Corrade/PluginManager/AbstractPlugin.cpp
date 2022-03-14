/*
    This file is part of Corrade.

    Copyright © 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016,
                2017, 2018, 2019, 2020, 2021, 2022
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

#include "AbstractPlugin.h"

#include "Corrade/Containers/Array.h"
#include "Corrade/Containers/GrowableArray.h"
#include "Corrade/Containers/Pair.h"
#include "Corrade/PluginManager/AbstractManager.h"
#include "Corrade/PluginManager/PluginMetadata.h"
#include "Corrade/Utility/ConfigurationGroup.h"
#include "Corrade/Utility/Path.h"

#ifndef CORRADE_PLUGINMANAGER_NO_DYNAMIC_PLUGIN_SUPPORT
#include "Corrade/PluginManager/configure.h"
#endif

namespace Corrade { namespace PluginManager {

using namespace Containers::Literals;

struct AbstractPlugin::State {
    AbstractManager* manager{};
    Containers::String plugin;
    const PluginMetadata* metadata{};
    Utility::ConfigurationGroup configuration{};
};

Containers::StringView AbstractPlugin::pluginInterface() { return ""_s; }

#ifndef CORRADE_PLUGINMANAGER_NO_DYNAMIC_PLUGIN_SUPPORT
Containers::Array<Containers::String> AbstractPlugin::pluginSearchPaths() { return {}; }

Containers::StringView AbstractPlugin::pluginSuffix() {
    return PLUGIN_FILENAME_SUFFIX;
}
#endif

Containers::StringView AbstractPlugin::pluginMetadataSuffix() {
    return ".conf"_s;
}

void AbstractPlugin::initialize() {}

void AbstractPlugin::finalize() {}

AbstractPlugin::AbstractPlugin(): _state{InPlaceInit} {}

AbstractPlugin::AbstractPlugin(AbstractManager& manager, const Containers::StringView& plugin): _state{InPlaceInit} {
    _state->manager = &manager;
    _state->plugin = Containers::String::nullTerminatedGlobalView(plugin);
    manager.registerInstance(plugin, *this, _state->metadata);
    _state->configuration = _state->metadata->configuration();
}

AbstractPlugin::AbstractPlugin(AbstractManager& manager): _state{InPlaceInit} {
    _state->manager = &manager;
}

AbstractPlugin::AbstractPlugin(AbstractPlugin&& other) noexcept: _state{std::move(other._state)} {
    /* Reregister the instance if the plugin was instantiated through a plugin
       manager. Note that instantiating using
       AbstractManagingPlugin::AbstractManagingPlugin(AbstractManager&) is
       *not* instantiating through the manager, in that case the _metadata
       field would be nullptr. */
    if(_state && _state->manager && _state->metadata)
        /* Takes the real name, not the alias -- see this function source
           for details why */
        _state->manager->reregisterInstance(_state->metadata->name(), other, this);
}

AbstractPlugin::~AbstractPlugin() {
    /* Unregister the instance only if the plugin was instantiated through a
       plugin manager. Note that instantiating using
       AbstractManagingPlugin::AbstractManagingPlugin(AbstractManager&) is
       *not* instantiating through the manager, in that case the _metadata
       field would be nullptr. */
    if(_state && _state->manager && _state->metadata)
        /* Takes the real name, not the alias -- see this function source
           for details why */
        _state->manager->reregisterInstance(_state->metadata->name(), *this, nullptr);
}

bool AbstractPlugin::canBeDeleted() { return false; }

Containers::StringView AbstractPlugin::plugin() const {
    CORRADE_ASSERT(_state, "PluginManager::AbstractPlugin::plugin(): can't be called on a moved-out plugin", _state->plugin);
    return _state->plugin;
}

const PluginMetadata* AbstractPlugin::metadata() const {
    CORRADE_ASSERT(_state, "PluginManager::AbstractPlugin::metadata(): can't be called on a moved-out plugin", {});
    return _state->metadata;
}

Utility::ConfigurationGroup& AbstractPlugin::configuration() {
    CORRADE_ASSERT(_state, "PluginManager::AbstractPlugin::configuration(): can't be called on a moved-out plugin", _state->configuration);
    return _state->configuration;
}
const Utility::ConfigurationGroup& AbstractPlugin::configuration() const {
    CORRADE_ASSERT(_state, "PluginManager::AbstractPlugin::configuration(): can't be called on a moved-out plugin", _state->configuration);
    return _state->configuration;
}

/* These asserts refer AbstractManagingPlugin because it's public only there */
AbstractManager* AbstractPlugin::manager() {
    CORRADE_ASSERT(_state, "PluginManager::AbstractManagingPlugin::manager(): can't be called on a moved-out plugin", {});
    return _state->manager;
}
const AbstractManager* AbstractPlugin::manager() const {
    CORRADE_ASSERT(_state, "PluginManager::AbstractManagingPlugin::manager(): can't be called on a moved-out plugin", {});
    return _state->manager;
}

#ifndef CORRADE_PLUGINMANAGER_NO_DYNAMIC_PLUGIN_SUPPORT
Containers::Array<Containers::String> implicitPluginSearchPaths(const Containers::StringView libraryLocation, const Containers::StringView hardcodedPath, const Containers::StringView relativePath) {
    Containers::Array<Containers::String> out;
    #ifdef CORRADE_TARGET_APPLE
    arrayReserve(out, 5);
    #elif !defined(CORRADE_TARGET_WINDOWS)
    arrayReserve(out, 4);
    #else
    arrayReserve(out, 3);
    #endif

    if(hardcodedPath) arrayAppend(out, Containers::String::nullTerminatedGlobalView(hardcodedPath));
    #ifdef CORRADE_TARGET_APPLE
    arrayAppend(out, Utility::Path::join("../PlugIns"_s, relativePath));
    #endif
    if(libraryLocation)
        arrayAppend(out, Utility::Path::join(Utility::Path::split(libraryLocation).first(), relativePath));
    #ifndef CORRADE_TARGET_WINDOWS
    arrayAppend(out, Utility::Path::join("../lib"_s, relativePath));
    #endif
    arrayAppend(out, Containers::String::nullTerminatedGlobalView(relativePath));

    return out;
}
#endif

}}
