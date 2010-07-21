#ifndef Map2X_Core_Utility_Configuration_h
#define Map2X_Core_Utility_Configuration_h
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
 * @brief Class Map2X::Utility::Configuration
 */

#include <string>
#include <vector>

#include "ConfigurationGroup.h"

namespace Map2X { namespace Utility {

/**
 * @brief Parser and writer for configuration files
 * @todo CR+LF / LF / CR files
 * @todo Renaming, copying groups
 */
class Configuration {
    public:
        /** @brief Flags for opening configuration file */
        enum Flags {
            /**
             * @brief Preserve BOM
             *
             * Preserves Byte-Order-Mark in UTF-8 files, if present. Otherwise
             * the BOM will not be saved back into the file.
             */
            PreserveBom     = 0x01,

            /**
             * @brief Truncate the file
             *
             * Doesn't load any configuration from file. On saving discards
             * everything in the file and writes only newly created values.
             */
            Truncate        = 0x02,

            /**
             * @brief Skip comments in the file
             *
             * No comments will be preserved on saving. Useful for memory saving.
             */
            SkipComments    = 0x04,

            /**
             * @brief Open the file read-only
             *
             * Opens the file read-only, which means faster access to elements
             * and less memory used. Enables Configuration::SkipComments.
             * Adding, changing and removing groups and keys will not be allowed.
             */
            ReadOnly        = 0x0c,

            /**
             * @brief Force unique groups
             *
             * When loading the file only first group with given name will be
             * loaded, every other will be skipped and discarded on saving.
             * Also doesn't allow adding new group with already existing name.
             */
            UniqueGroups    = 0x10,

            /**
             * @brief Force unique keys
             *
             * Only first value with given key (per group) will be loaded, every
             * other will be skipped and discarded on saving. Also doesn't allow
             * adding new value with the key name already existing in the group.
             */
            UniqueKeys      = 0x20,

            /**
             * @brief Force unique groups and keys
             *
             * Same as Configuration::UniqueGroups | Configuration::UniqueKeys .
             */
            UniqueNames     = 0x30,
        };

        /**
         * @brief Constructor
         * @param file      %Configuration file
         * @param flags     Flags (see Configuration::Flags)
         *
         * Opens the file and loads it according to specified flags.
         */
        Configuration(const std::string& file, int flags = 0);

        /** @{ @name Group operations */

        /**
         * @brief Get group
         * @param name      Name of the group. Empty string means global group.
         * @param number    Number of the group. Default is first found group.
         * @return Pointer to group. If no group was found, returns null pointer.
         */
        ConfigurationGroup* group(const std::string& name = "", unsigned int number = 0);
        const ConfigurationGroup* group(const std::string& name = "", unsigned int number = 0) const; /**< @overload */

        /**
         * @brief Get all groups
         * @param name      Name of the group. Because global group is only one,
         *      retrieving list of all global groups is meaningless as it
         *      returns always one-item list. For global group use group()
         *      instead.
         * @return Vector of groups. If no group found, returns empty vector.
         */
        std::vector<ConfigurationGroup*> groups(const std::string& name);
        std::vector<const ConfigurationGroup*> groups(const std::string& name) const; /**< @overload */

        /**
         * @brief Count of groups with given name
         * @param name      Name of the group. Retrieving count of global groups
         *      is meaningless as global group is always only one.
         * @return Count
         *
         * See also Configuration::UniqueGroups and Configuration::UniqueNames.
         */
        unsigned int groupCount(const std::string& name) const;

        /**
         * @brief Add new group
         * @param name      Name of the group. Global group is always present
         *      and can be only one per file, so adding new global group will
         *      fail.
         * @return Newly created group or null pointer when new group cannot be
         *      added (see above or flags Configuration::UniqueGroups and
         *      Configuration::ReadOnly).
         */
        ConfigurationGroup* addGroup(const std::string& name);

        /**
         * @brief Remove group
         * @param name      Name of the group. Global group cannot be removed.
         * @param number    Number of the group. Default is first found group.
         * @return Whether the groups were removed. (see above or flag
         *      Connfiguration::ReadOnly).
         */
        bool removeGroup(const std::string& name, unsigned int number = 0);

        /**
         * @brief Remove all groups with given name
         * @param name      Name of groups to remove. Global group cannot be
         *      removed.
         * @return Whether the removal was successful (see above or flag
         *      Connfiguration::ReadOnly).
         */
        bool removeAllGroups(const std::string& name);

        /*@}*/

        /** @{ @name Convenience functions for operating with global group
         *
         * Calling Configuration::foo() with these functions has the same
         * functionality as calling Configuration::group()->foo().
         */

        /** @copydoc ConfigurationGroup::value() */
        template<class T> inline bool value(const std::string& key, T* value, int flags = 0, unsigned int number = 0) const
            { return group()->value(key, value, number); }
        /** @copydoc ConfigurationGroup::values() */
        template<class T> inline std::vector<T> values(const std::string& key, int flags = 0) const
            { return group()->values<T>(key); }

        /** @copydoc ConfigurationGroup::setValue() */
        template<class T> inline bool setValue(const std::string& key, const T& value, int flags = 0, unsigned int number = 0)
            { return group()->setValue(key, value, number); }
        /** @copydoc ConfigurationGroup::addValue() */
        template<class T> inline bool addValue(const std::string& key, const T& value, int flags = 0)
            { return group()->addValue(key, value); }

        /** @copydoc ConfigurationGroup::removeValue() */
        inline bool removeValue(const std::string& key, unsigned int number = 0)
            { return group()->removeValue(key, number); }
        /** @copydoc ConfigurationGroup::removeAllValues() */
        inline bool removeAllValues(const std::string& key)
            { return group()->removeAllValues(key); }

        /*@}*/
};

}}

#endif
