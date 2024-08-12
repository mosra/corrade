#ifndef Corrade_PluginManager_Implementation_Plugin_h
#define Corrade_PluginManager_Implementation_Plugin_h
/*
    This file is part of Corrade.

    Copyright © 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016,
                2017, 2018, 2019, 2020, 2021, 2022, 2023
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

#include <vector>

#include "Corrade/Containers/Array.h"
#include "Corrade/Containers/String.h"
#include "Corrade/PluginManager/PluginMetadata.h"
#include "Corrade/Utility/Configuration.h"

#ifdef CORRADE_TARGET_WINDOWS
/* I didn't find a better way to circumvent the need for including windows.h.
   Same is in PluginManager/AbstractManager.h. */
struct HINSTANCE__;
typedef struct HINSTANCE__* HMODULE;
#endif

namespace Corrade { namespace PluginManager { namespace Implementation {

struct StaticPlugin;

struct Plugin: PluginMetadata {
    #ifndef CORRADE_PLUGINMANAGER_NO_DYNAMIC_PLUGIN_SUPPORT
    LoadState loadState;
    #else
    const LoadState loadState; /* Always LoadState::Static */
    #endif
    Utility::Configuration metadata;
    Containers::String name;
    /** @todo these two could be views once Configuration is reworked; the
        depends and provides could be in the same allocation as the Plugin
        itself once Pointer supports custom deleters */
    Containers::Array<Containers::String> depends;
    Containers::Array<Containers::String> provides;
    /* This array is filled by the `name` field of other plugins, which is
       guaranteed to stay and not be reallocated for as long as the other
       plugin exists (and once it's unloaded, it's removed from this array),
       meaning that we can store just views. */
    Containers::Array<Containers::StringView> usedBy;
    const Utility::ConfigurationGroup* data;
    Utility::ConfigurationGroup* configuration;

    void*(*instancer)(AbstractManager&, const Containers::StringView&);
    void(*finalizer)();

    #ifndef CORRADE_PLUGINMANAGER_NO_DYNAMIC_PLUGIN_SUPPORT
    union {
        /* For static plugins */
        const StaticPlugin* staticPlugin;

        /* For dynamic plugins */
        #ifndef CORRADE_TARGET_WINDOWS
        void* module;
        #else
        HMODULE module;
        #endif
    };
    #else
    const StaticPlugin* staticPlugin;
    #endif

    /** @todo can't be a growable Array because the growable deleter, assigned
        in registerInstance() / reregisterInstance() would point to the dynamic
        plugin binary in static builds -- either wait until Array<T, Allocator>
        is a thing or (better) turn this into a linked list similarly to how
        static plugins are handled, thereby avoiding allocations and linear
        lookups when reregistering and removing instances */
    std::vector<AbstractPlugin*> instances;

    /* Delegated to from both the other constructors */
    explicit Plugin(Utility::Configuration&& configuration, Containers::String&& name, void*(*instancer)(AbstractManager&, const Containers::StringView&));

    #ifndef CORRADE_PLUGINMANAGER_NO_DYNAMIC_PLUGIN_SUPPORT
    /* Constructor for dynamic plugins. Defined in AbstractManager.cpp. */
    explicit Plugin(Containers::StringView name, Containers::StringView metadata);
    #endif

    /* Constructor for static plugins. Defined in AbstractManager.cpp. */
    explicit Plugin(const StaticPlugin& staticPlugin, Utility::Configuration&& configuration);

    Plugin(const Plugin&) = delete;
    Plugin(Plugin&&) = delete;
    Plugin& operator=(const Plugin&) = delete;
    Plugin& operator=(Plugin&&) = delete;

    ~Plugin() = default;
};

}}}

#endif
