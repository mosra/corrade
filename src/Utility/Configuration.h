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
#include <map>

#include "ConfigurationGroup.h"

namespace Map2X { namespace Utility {

/**
 * @brief Parser and writer for configuration files
 * @todo CR+LF / LF / CR files
 * @todo Renaming, copying groups
 * @todo Use find() and equal_range() for faster (log) access
 * @todo Are groups saved in same order as originallly?
 * @todo Use some try/catch for parsing (avoid repeated code for group adding)
 * @todo Don't throw out whole group on invalid row
 * @todo More data types
 * @todo EOL autodetection according to system on unsure/new files (default is
 *      preserve)
 */
class Configuration {
    friend class ConfigurationGroup;

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
             * @brief Force Unix line endings
             *
             * Forces Unix EOL (LF). Default behavior is to preserve original.
             * If original EOL type cannot be distinguished, Unix is used.
             */
            ForceUnixEol    = 0x02,

            /** @brief Force Windows line endings */
            ForceWindowsEol = 0x04,

            /**
             * @brief Truncate the file
             *
             * Doesn't load any configuration from file. On saving discards
             * everything in the file and writes only newly created values.
             */
            Truncate        = 0x08,

            /**
             * @brief Skip comments in the file
             *
             * No comments or empty lines will be preserved on saving. Useful
             * for memory saving. See also Configuration::ReadOnly.
             */
            SkipComments    = 0x10,

            /**
             * @brief Open the file read-only
             *
             * Opens the file read-only, which means faster access to elements
             * and less memory used. Sets also Configuration::SkipComments.
             * Adding, changing and removing groups and keys will not be
             * allowed.
             */
            ReadOnly        = 0x20,

            /**
             * @brief Force unique groups
             *
             * When loading the file only first group with given name will be
             * loaded, every other will be skipped and discarded on saving.
             * Also doesn't allow adding new group with already existing name.
             */
            UniqueGroups    = 0x40,

            /**
             * @brief Force unique keys
             *
             * Only first value with given key (per group) will be loaded, every
             * other will be skipped and discarded on saving. Also doesn't allow
             * adding new value with the key name already existing in the group.
             */
            UniqueKeys      = 0x80,

            /**
             * @brief Force unique groups and keys
             *
             * Same as Configuration::UniqueGroups | Configuration::UniqueKeys .
             */
            UniqueNames     = 0xc0
        };

        /**
         * @brief Constructor
         * @param _file      %Configuration file
         * @param _flags     Flags (see Configuration::Flags)
         *
         * Opens the file and loads it according to specified flags. If file
         * cannot be opened, sets invalid flag (see isValid()).
         */
        Configuration(const std::string& _file, int _flags = 0);

        /**
         * @brief Destructor
         *
         * If the configuration has been changed, writes configuration back to
         * the file. See also save().
         */
        inline ~Configuration() { if(flags & Changed) save(); }

        /**
         * @brief Whether the file is valid
         * @return Returns false if the file has syntax errors or couldn't be
         *      opened, true otherwise.
         *
         * Invalid files cannot be changed or saved back.
         */
        inline bool isValid() { return flags & IsValid; }

        /**
         * @brief Save configuration
         * @return Whether the file was saved successfully
         *
         * Writes configuration back to the file (only if
         * Configuration::ReadOnly flag wasn't set). Note that even if no change
         * to the configuration was made, the file could differ after saving
         * (see Configuration::Flags).
         */
        bool save();

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
         *
         * Adds new group at the end of file.
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
         *
         * @todo Return false if no group was removed?
         */
        bool removeAllGroups(const std::string& name);

        /*@}*/

        /** @{ @name Convenience functions for operating with global group
         *
         * Calling Configuration::foo() with these functions has the same
         * functionality as calling Configuration::group()->foo().
         */

        /** @copydoc ConfigurationGroup::value() */
        template<class T> inline bool value(const std::string& key, T* value, unsigned int number = 0, int flags = 0) const
            { return group()->value<T>(key, value, number, flags); }
        /** @copydoc ConfigurationGroup::values() */
        template<class T> inline std::vector<T> values(const std::string& key, int flags = 0) const
            { return group()->values<T>(key, flags); }
        /** @copydoc ConfigurationGroup::valueCount() */
        inline unsigned int valueCount(const std::string& key) const
            { return group()->valueCount(key); }

        /** @copydoc ConfigurationGroup::setValue() */
        template<class T> inline bool setValue(const std::string& key, const T& value, unsigned int number = 0, int flags = 0)
            { return group()->setValue<T>(key, value, number, flags); }
        /** @copydoc ConfigurationGroup::addValue() */
        template<class T> inline bool addValue(const std::string& key, const T& value, int flags = 0)
            { return group()->addValue<T>(key, value, flags); }

        /** @copydoc ConfigurationGroup::removeValue() */
        inline bool removeValue(const std::string& key, unsigned int number = 0)
            { return group()->removeValue(key, number); }
        /** @copydoc ConfigurationGroup::removeAllValues() */
        inline bool removeAllValues(const std::string& key)
            { return group()->removeAllValues(key); }

        /*@}*/

    private:
        /** @brief Private flags for file state */
        enum PrivateFlags {
            IsValid = 0x10000,      /**< @brief Whether the loaded file is valid */
            HasBom = 0x20000,       /**< @brief BOM mark was found in the file */
            WindowsEol = 0x40000,   /**< @brief The file has Windows line endings */
            Changed = 0x80000       /**< @brief Whether the file has changed */
        };

        /** @brief Configuration file */
        std::string filename;

        /**
         * @brief Flags
         *
         * Combination of Configuration::Flags and Configuration::PrivateFlags.
         */
        int flags;

        /** @brief Configuration groups */
        std::vector<ConfigurationGroup> _groups;
};

}}

#endif
