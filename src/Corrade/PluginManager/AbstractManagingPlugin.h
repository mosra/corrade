#ifndef Corrade_PluginManager_AbstractManagingPlugin_h
#define Corrade_PluginManager_AbstractManagingPlugin_h
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
 * @brief Class @ref Corrade::PluginManager::AbstractManagingPlugin
 */

#include "Corrade/PluginManager/AbstractPlugin.h"
#include "Corrade/PluginManager/Manager.h"

namespace Corrade { namespace PluginManager {

/**
@brief Base class for plugin interfaces with access to associated manager

Useful for plugins which needs to access the manager (e.g. for loading and
using other plugins).
*/
template<class Interface> class AbstractManagingPlugin: public AbstractPlugin {
    public:
        /**
         * @brief Default constructor
         *
         * Define this constructor in your subclass only if you want to allow
         * using the interface or plugin without any access to plugin manager.
         *
         * The @ref manager() and @ref metadata() functions will return
         * @cpp nullptr @ce.
         */
        explicit AbstractManagingPlugin() = default;

        /** @brief Copying is not allowed */
        AbstractManagingPlugin(const AbstractManagingPlugin<Interface>&) = delete;

        /**
         * @brief Move constructor
         *
         * Destructive move. See @ref AbstractPlugin::AbstractPlugin(AbstractPlugin&&)
         * for more information.
         */
        AbstractManagingPlugin(AbstractManagingPlugin<Interface>&& other) noexcept = default;

        /** @brief Copying is not allowed */
        AbstractManagingPlugin<Interface>& operator=(const AbstractManagingPlugin<Interface>&) = delete;

        /** @brief Only move construction is allowed */
        AbstractManagingPlugin<Interface>& operator=(AbstractManagingPlugin<Interface>&&) = delete;

        /**
         * @brief Default constructor with access to plugin manager
         *
         * Define this constructor in your subclass only if you want to allow
         * using the plugin directly and the plugin needs access to its plugin
         * manager.
         *
         * The @ref metadata() function will return @cpp nullptr @ce.
         */
        explicit AbstractManagingPlugin(Manager<Interface>& manager): AbstractPlugin{manager} {}

        /**
         * @brief Plugin manager constructor
         *
         * Used by plugin manager. Don't forget to redefine this constructor in
         * all your subclasses.
         * @see @ref manager(), @ref metadata()
         */
        explicit AbstractManagingPlugin(AbstractManager& manager, const std::string& plugin):
            AbstractPlugin{manager, plugin} {}

    protected:
        /**
         * @brief Manager
         *
         * Manager associated to given plugin. If the plugin was not
         * instantiated with access to plugin manager, returns @cpp nullptr @ce.
         *
         * Can't be called on a moved-out instance.
         */
        Manager<Interface>* manager() {
            return static_cast<Manager<Interface>*>(AbstractPlugin::manager());
        }

        /** @overload */
        const Manager<Interface>* manager() const {
            return static_cast<const Manager<Interface>*>(AbstractPlugin::manager());
        }
};

}}

#endif
