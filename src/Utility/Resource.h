#ifndef Corrade_Utility_Resource_h
#define Corrade_Utility_Resource_h
/*
    This file is part of Corrade.

    Copyright © 2007, 2008, 2009, 2010, 2011, 2012, 2013
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
 * @brief Class Corrade::Utility::Resource
 */

#include <map>
#include <string>
#include <vector>

#include "Utility/corradeUtilityVisibility.h"

namespace Corrade { namespace Utility {

/**
@brief Data resource management

This class provides support for compiled-in data resources - both compiling
and reading. Resources can be differentiated into more groups, every resource
in given group has unique filename.

See @ref resource-management for brief introduction and example usage.
Standalone resource compiler executable is implemented in @ref rc.cpp.

@section UtilityResource-configuration Resource configuration file

Function compileFrom() takes configuration file as parameter. The file allows
you to specify filenames and filename aliases of resource files instead of
passing the data manually to compile(). The file is used when compiling
resources using @ref corrade-cmake "corrade_add_resource()" via CMake. Example
file:

    group=myGroup

    [file]
    filename=../resources/intro-new-final.ogg
    alias=intro.ogg

    [file]
    filename=license.txt

    [file]
    filename=levels-insane.conf
    alias=levels-easy.conf

@todo Ad-hoc resources
@todo Test data unregistering
 */
class CORRADE_UTILITY_EXPORT Resource {
    public:
        /**
         * @brief Compile data resource file
         * @param name          %Resource name (see CORRADE_RESOURCE_INITIALIZE())
         * @param group         Group name for getting data
         * @param files         Files (pairs of filename, file data)
         *
         * Produces C++ file with hexadecimal data representation.
         */
        static std::string compile(const std::string& name, const std::string& group, const std::vector<std::pair<std::string, std::string>>& files);

        /**
         * @brief Compile data resource file using configuration file
         * @param name          %Resource name (see CORRADE_RESOURCE_INITIALIZE())
         * @param configurationFile %Filename of configuration file
         *
         * Produces C++ file with hexadecimal data representation. See class
         * documentation for configuration file syntax overview. The filenames
         * are taken relative to configuration file path.
         */
        static std::string compileFrom(const std::string& name, const std::string& configurationFile);

        /**
         * @brief Constructor
         * @param group         Group name for getting data
         *
         * If the group is not found, prints message to error output.
         */
        explicit Resource(const std::string& group);

        /**
         * @brief Get pointer to raw resource data
         * @param filename      Filename
         *
         * Returns data of given file in the group as pair of pointer and size.
         * If the group does not exist, returns `{nullptr, 0}`. If the file was
         * not found, prints message to error output and returns `{nullptr, 0}`.
         */
        std::pair<const unsigned char*, unsigned int> getRaw(const std::string& filename) const;

        /**
         * @brief Get data resource
         * @param filename      Filename
         * @return Data of given group (specified in constructor) and filename.
         *      Returns empty string if nothing was found.
         *
         * Returns data of given file in the group. If the group does not
         * exist, returns empty string. If the file was not found, prints
         * message to error output and returns empty string.
         */
        std::string get(const std::string& filename) const;

    #ifdef DOXYGEN_GENERATING_OUTPUT
    private:
    #endif
        /* Internal use only. */
        static void registerData(const char* group, unsigned int count, const unsigned char* positions, const unsigned char* filenames, const unsigned char* data);
        static void unregisterData(const char* group, const unsigned char* data);

    private:
        struct CORRADE_UTILITY_LOCAL ResourceData {
            unsigned int position;
            unsigned int size;
            const unsigned char* data;
        };

        /* Accessed through function to overcome "static initialization order
           fiasco" which I think currently fails only in static build */
        CORRADE_UTILITY_LOCAL static std::map<std::string, std::map<std::string, ResourceData>>& resources();

        CORRADE_UTILITY_LOCAL static std::string compileFromInternal(const std::string& name, std::istream& in, const std::string& path);
        CORRADE_UTILITY_LOCAL static std::pair<bool, std::string> fileContents(const std::string& filename);

        CORRADE_UTILITY_LOCAL static std::string comment(const std::string& comment);
        CORRADE_UTILITY_LOCAL static std::string hexcode(const std::string& data);
        template<class T> static std::string numberToString(const T& number);

        std::map<std::string, std::map<std::string, ResourceData>>::const_iterator _group;
};

/**
@brief Initialize resource

If a resource is compiled into dynamic library or directly into executable, it
will be initialized automatically thanks to CORRADE_AUTOMATIC_INITIALIZER()
macros. However, if the resource is compiled into static library, it must be
explicitly initialized via this macro, e.g. at the beginning of main(). You can
also wrap these macro calls into another function (which will then be compiled
into dynamic library or main executable) and use CORRADE_AUTOMATIC_INITIALIZER()
macro for automatic call.

@attention This macro should be called outside of any namespace. If you are
    running into linker errors with `resourceInitializer_*`, this could be the
    problem. If you are in a namespace and cannot call this macro from main(),
    try this:
@code
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

@see CORRADE_RESOURCE_FINALIZE()
*/
#define CORRADE_RESOURCE_INITIALIZE(name)                                     \
    extern int resourceInitializer_##name();                                  \
    resourceInitializer_##name();

/**
@brief Cleanup resource

Cleans up resource previously (even automatically) initialized via
CORRADE_RESOURCE_INITIALIZE().

@attention This macro should be called outside of any namespace. See
    CORRADE_RESOURCE_INITIALIZE() documentation for more information.
*/
#define CORRADE_RESOURCE_FINALIZE(name)                                       \
    extern int resourceFinalizer_##name();                                    \
    resourceFinalizer_##name();

}}

#endif
