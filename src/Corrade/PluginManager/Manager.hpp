#ifndef Corrade_PluginManager_Manager_hpp
#define Corrade_PluginManager_Manager_hpp
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

/** @file
@brief Template implementation for @ref Manager.h
@m_since_latest

Contains template definitions of the @ref Corrade::PluginManager::Manager
class. See @ref PluginManager-Manager-template-definitions for more
information.
*/

#include "Corrade/Containers/Array.h"
#include "Corrade/Containers/String.h"
#include "Corrade/PluginManager/Manager.h"

namespace Corrade { namespace PluginManager {

template<class T> Manager<T>::Manager(const Containers::StringView pluginDirectory):
    #ifndef CORRADE_PLUGINMANAGER_NO_DYNAMIC_PLUGIN_SUPPORT
    AbstractManager{T::pluginInterface(), T::pluginSearchPaths(), T::pluginSuffix(), T::pluginMetadataSuffix(), pluginDirectory} {}
    #else
    AbstractManager{T::pluginInterface(), T::pluginMetadataSuffix()} { static_cast<void>(pluginDirectory); }
    #endif

template<class T> Manager<T>::Manager(): Manager{Containers::StringView{}} {}

template<class T> Containers::Pointer<T> Manager<T>::instantiate(const Containers::StringView plugin) {
    return Containers::pointerCast<T>(instantiateInternal(plugin));
}

template<class T> Containers::Pointer<T> Manager<T>::loadAndInstantiate(const Containers::StringView plugin) {
    return Containers::pointerCast<T>(loadAndInstantiateInternal(plugin));
}

}}

#endif
