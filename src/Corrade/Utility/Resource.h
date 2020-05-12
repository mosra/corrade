#ifndef Corrade_Utility_Resource_h
#define Corrade_Utility_Resource_h
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
 * @brief Class @ref Corrade::Utility::Resource
 */

#include <utility>

#include "Corrade/Containers/ArrayView.h"
#include "Corrade/Utility/StlForwardString.h"
#include "Corrade/Utility/StlForwardVector.h"
#include "Corrade/Utility/visibility.h"

namespace Corrade { namespace Utility {

namespace Implementation {
    struct ResourceGroup;
}

/**
@brief Data resource management

This class provides support for compiled-in data resources - both compiling
and reading. Resources can be differentiated into more groups, every resource
in given group has unique filename.

See @ref resource-management for brief introduction and example usage.
Standalone resource compiler executable is implemented in
@ref corrade-rc "corrade-rc".

@m_class{m-note m-default}

@par Memory access and operation complexity
    Resource registration (either automatic or using
    @ref CORRADE_RESOURCE_INITIALIZE()) is a simple operation without any
    heap access or other operations that could potentially fail. When using
    only the @ref hasGroup() and @ref getRaw() APIs with plain C string
    literals, no memory allocation or heap access is involved either.
@par
    The group lookup during construction and @ref hasGroup() is done with a
    @f$ \mathcal{O}(n) @f$ complexity as the resources register themselves
    into a linked list. Actual file lookup after is done in-place on the
    compiled-in data in a @f$ \mathcal{O}(\log{}n) @f$ time.

@section Utility-Resource-conf Resource configuration file

Function @ref compileFrom() takes a configuration file as parameter. The file
allows you to specify filenames and filename aliases of resource files instead
of passing the data manually to @ref compile(). The file is used when compiling
resources using @ref corrade-cmake-add-resource "corrade_add_resource()" via
CMake. The file can be also used when overriding compiled-in resources with
live data using @ref overrideGroup(). All filenames are expected to be in
UTF-8. Example file:

@code{.ini}
group=myGroup

[file]
filename=../resources/intro-new-final.ogg
alias=intro.ogg

[file]
filename=license.txt

[file]
filename=levels-insane.conf
alias=levels-easy.conf
@endcode

@section Utility-Resource-multithreading Thread safety

The resources register themselves into a global storage. If done
implicitly, the registration is executed before entering @cpp main() @ce and
thus serially. If done explicitly via @ref CORRADE_RESOURCE_INITIALIZE() /
@ref CORRADE_RESOURCE_FINALIZE(), these macros *have to* be called from a
single thread or externally guarded to avoid data races. Same goes for the
@ref overrideGroup() function.

On the other hand, all other functionality only reads from the global storage
and thus is thread-safe.

@todo Ad-hoc resources
 */
class CORRADE_UTILITY_EXPORT Resource {
    public:
        /**
         * @brief Compile data resource file
         * @param name          Resource name (see @ref CORRADE_RESOURCE_INITIALIZE())
         * @param group         Group name
         * @param files         Files (pairs of filename, file data)
         *
         * Produces a C++ file with hexadecimal data representation.
         */
        static std::string compile(const std::string& name, const std::string& group, const std::vector<std::pair<std::string, std::string>>& files);

        /**
         * @brief Compile data resource file using configuration file
         * @param name          Resource name (see @ref CORRADE_RESOURCE_INITIALIZE())
         * @param configurationFile Filename of configuration file
         *
         * Produces a C++ file with hexadecimal data representation. See class
         * documentation for configuration file syntax overview. The filenames
         * are taken relative to configuration file path.
         */
        static std::string compileFrom(const std::string& name, const std::string& configurationFile);

        /**
         * @brief Override group
         * @param group         Group name
         * @param configurationFile Filename of configuration file. Empty
         *      string will discard the override.
         *
         * Overrides compiled-in resources of given group with live data
         * specified in given configuration file, useful during development and
         * debugging. Subsequently created @ref Resource instances with the
         * same group will take data from live filesystem instead and fallback
         * to compiled-in resources only for files that are not found.
         *
         * @attention Unlike all other methods of this class, this one is *not*
         *      thread-safe. See @ref Utility-Resource-multithreading for more
         *      information.
         */
        static void overrideGroup(const std::string& group, const std::string& configurationFile);

        /** @brief Whether given group exists */
        static bool hasGroup(const std::string& group);

        /** @overload */
        template<std::size_t size> static bool hasGroup(const char(&group)[size]) {
            return hasGroupInternal({group, size - 1});
        }

        /**
         * @brief Constructor
         *
         * Expects that the group exists.
         * @see @ref hasGroup()
         */
        explicit Resource(const std::string& group);

        /** @overload */
        template<std::size_t size>
        explicit Resource(const char(&group)[size]): Resource{{group, size - 1}, nullptr} {}

        ~Resource();

        /**
         * @brief List of all resources in the group
         *
         * Note that the list contains only list of compiled-in files, no
         * additional filenames from overriden group are included.
         */
        std::vector<std::string> list() const;

        /**
         * @brief Get resource data
         * @param filename      Filename in UTF-8
         *
         * Returns a view on data of given file in the group. Expects that
         * the file exists. If the file is empty, returns @cpp nullptr @ce.
         */
        Containers::ArrayView<const char> getRaw(const std::string& filename) const;

        /** @overload */
        template<std::size_t size> Containers::ArrayView<const char> getRaw(const char(&filename)[size]) const {
            return getInternal({filename, size - 1});
        }

        /**
         * @brief Get resource data as a @ref std::string
         * @param filename      Filename in UTF-8
         *
         * Returns data of given file in the group. Expects that the file
         * exists.
         */
        std::string get(const std::string& filename) const;

    #ifdef DOXYGEN_GENERATING_OUTPUT
    private:
    #endif
        /* Used internally from the CORRADE_RESOURCE_INITIALIZE() /
           CORRADE_RESOURCE_FINALIZE() macros */
        static void registerData(Implementation::ResourceGroup& resource);
        static void unregisterData(Implementation::ResourceGroup& resource);

    private:
        struct OverrideData;

        static bool hasGroupInternal(Containers::ArrayView<const char> group);

        /* The void* is just to avoid this being matched by accident */
        explicit Resource(Containers::ArrayView<const char> group, void*);

        Containers::ArrayView<const char> getInternal(Containers::ArrayView<const char> filename) const;

        Implementation::ResourceGroup* _group;
        OverrideData* _overrideGroup;
};

/**
@brief Initialize a resource

If a resource is compiled into a dynamic library or directly into the
executable, it will be registered automatically thanks to the
@ref CORRADE_AUTOMATIC_INITIALIZER() macro. However, if the resource is
compiled into static library, it must be explicitly initialized via this macro,
e.g. at the beginning of @cpp main() @ce, otherwise it won't be known to
@ref Corrade::Utility::Resource "Utility::Resource". You can also wrap these
macro calls into another function (which will then be compiled into a dynamic
library or the main executable) and use the @ref CORRADE_AUTOMATIC_INITIALIZER()
macro for an automatic call:

@attention This macro should be called outside of any namespace. If you are
    running into linker errors with `resourceInitializer_*`, this could be the
    reason. If you are in a namespace and cannot call this macro from `main()`,
    try this:
@attention
    @code{.cpp}
    static void initialize() {
        CORRADE_RESOURCE_INITIALIZE(res)
    }

    namespace Foo {
        void bar() {
            initialize();

            //...
        }
    }
    @endcode

Functions called by this macro don't do any dynamic allocation or other
operations that could fail, so it's safe to call it even in restricted phases
of application exection. It's also safe to call this macro more than once.

@see @ref CORRADE_RESOURCE_FINALIZE()
*/
/* Contents of this macro are used in PluginManager/AbstractManager.h to avoid
   an include dependency. Keep in sync. */
#define CORRADE_RESOURCE_INITIALIZE(name)                                     \
    extern int resourceInitializer_##name();                                  \
    resourceInitializer_##name();

/**
@brief Finalize a resource

De-registers resource previously (even automatically) initialized via
@ref CORRADE_RESOURCE_INITIALIZE(). After this call,
@ref Corrade::Utility::Resource "Utility::Resource" will not know about given
resource anymore.

@attention This macro should be called outside of any namespace. See the
    @ref CORRADE_RESOURCE_INITIALIZE() macro for more information.

Functions called by this macro don't do any dynamic allocation or other
operations that could fail, so it's safe to call it even in restricted phases
of application exection. It's also safe to call this macro more than once.
*/
#define CORRADE_RESOURCE_FINALIZE(name)                                       \
    extern int resourceFinalizer_##name();                                    \
    resourceFinalizer_##name();

namespace Implementation {

struct ResourceGroup {
    const char* name;
    unsigned int count;
    const unsigned int* positions;
    const unsigned char* filenames;
    const unsigned char* data;
    /* This field shouldn't be written to by anything else than
       resourceInitializer() / resourceFinalizer(). It's zero-initilized by
       default and those use it to avoid inserting a single item to the linked
       list more than once. */
    ResourceGroup* next;
};

}

}}

#endif
