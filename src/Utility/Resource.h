#ifndef Map2X_Utility_Resource_h
#define Map2X_Utility_Resource_h
/*
    Copyright © 2007, 2008, 2009, 2010 Vladimír Vondruš <mosra@centrum.cz>

    This file is part of Map2X.

    Map2X is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License version 3
    only, as published by the Free Software Foundation.

    Map2X is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU Lesser General Public License version 3 for more details.
*/

/** @file
 * @brief Class Map2X::Utility::Resource
 */

#include <string>
#include <map>

namespace Map2X { namespace Utility {

/**
 * @brief Data resource management
 *
 * This class provides support for compiled-in data resources - both compiling
 * and reading.
 *
 * Resources can be differentiated into more groups, every resource in given
 * group has unique filename.
 *
 * See also @ref rc.cpp.
 * @todo Ad-hoc resources
 * @todo Test data unregistering
 * @todo Test empty files
 */
class Resource {
    public:
        /**
         * @brief Register data resource
         * @param group         Group name
         * @param count         File count
         * @param positions     Positions of filenames and data in binary blobs
         * @param filenames     Pointer to binary blob with filenames
         * @param data          Pointer to binary blob with file data
         *
         * This function is used internally for automatic data resource
         * registering, no need to use it directly.
         */
        static void registerData(const char* group, size_t count, const unsigned char* positions, const unsigned char* filenames, const unsigned char* data);

        /**
         * @brief Unregister data resource
         * @param group         Group name
         * @param data          Pointer to binary blob with file data
         *
         * This function is used internally for automatic data resource
         * unregistering, no need to use it directly.
         */
        static void unregisterData(const char* group, const unsigned char* data);

        /**
         * @brief Constructor
         * @param _group        Group name for getting data or compiling new
         *      resources.
         */
        inline Resource(const std::string& _group): group(_group) {}

        /**
         * @brief Compile data resource file
         * @param name          Resource name (see RESOURCE_INITIALIZE())
         * @param files         Map with files, first item of pair is filename,
         *      second is file data.
         *
         * Produces C++ file with hexadecimally represented file data. The file
         * then must be compiled directly to executable or library.
         */
        std::string compile(const std::string& name, const std::map<std::string, std::string>& files) const;

        /**
         * @brief Compile data resource file
         * @param name          Resource name (see RESOURCE_INITIALIZE())
         * @param filename      Filename
         * @param data          File data
         *
         * Convenience function for compiling resource with only one file.
         */
        std::string compile(const std::string& name, const std::string& filename, const std::string& data) const;

        /**
         * @brief Get data resource
         * @param filename      Filename
         * @return Data of given group (specified in constructor) and filename.
         *      Returns empty string if nothing was found.
         */
        std::string get(const std::string& filename) const;

    private:
        struct ResourceData {
            size_t position;
            size_t size;
            const unsigned char* data;
        };

        static std::map<std::string, std::map<std::string, ResourceData> > resources;

        std::string group;

        std::string hexcode(const std::string& data, const std::string& comment = "") const;

        /** @todo Move to utilities.h? */
        template<class T> static std::string numberToString(const T& number);
        template<class T> static T numberFromString(const std::string& number);
};

/**
 * @brief Initialize resource
 *
 * If a resource is compiled into dynamic library or directly into executable,
 * it will be initialized automatically thanks to AUTOMATIC_INITIALIZER()
 * macros. However, if the resource is compiled into static library, it must be
 * explicitly initialized at via this macro, e.g. at the beginning of main().
 * You can also wrap these macro calls into another function (which will then
 * be compiled into dynamic library or main executable) and use
 * AUTOMATIC_INITIALIZER() macro for automatic call.
 * @attention This macro should be called outside of any namespace. If you are
 * running into linker errors with @c resourceInitializer_*, this could be the
 * problem. If you are in a namespace and cannot call this macro from main(),
 * try this:
 * @code
 * void initialize() {
 *     RESOURCE_INITIALIZE(res)
 * }
 *
 * namespace Foo {
 *     void bar() {
 *         initialize();
 *
 *         //...
 *     }
 * }
 * @endcode
 */
#define RESOURCE_INITIALIZE(name)                                             \
    extern int resourceInitializer_##name();                                  \
    resourceInitializer_##name();

/**
 * @brief Cleanup resource
 *
 * Cleans up previously (even automatically) initialized resource.
 * @attention This macro should be called outside of any namespace. See
 * RESOURCE_INITIALIZE() documentation for more information.
 */
#define RESOURCE_CLEANUP(name)                                                \
    extern int resourceInitializer_##name();                                  \
    resourceFinalizer_##name();

}}

#endif
