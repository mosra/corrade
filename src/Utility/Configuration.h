#ifndef Kompas_Core_Utility_Configuration_h
#define Kompas_Core_Utility_Configuration_h
/*
    Copyright © 2007, 2008, 2009, 2010, 2011 Vladimír Vondruš <mosra@centrum.cz>

    This file is part of Kompas.

    Kompas is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License version 3
    only, as published by the Free Software Foundation.

    Kompas is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU Lesser General Public License version 3 for more details.
*/

/** @file
 * @brief Class Kompas::Utility::Configuration
 */

#include <string>
#include <vector>
#include <map>
#include <fstream>

#include "ConfigurationGroup.h"

namespace Kompas { namespace Utility {

/**
@brief Parser and writer for configuration files

Provides hierarchical configuration storage. The key/value pairs are stored in
hierarchical groups, this class acts as an root configuration group. Supported
are either non-unique or unique group and key names, uniqueness can be enforced
via flag in constructor. See ConfigurationGroup class documentation for
accessing, deleting, adding and setting groups and values.

Values can be saved and retrieved using templated function, so it's possible to
implement saving for any type. See ConfigurationValue documentation for an
example.

@section Configuration_Example Example usage
@code
Configuration conf("my.conf");

// Get value of third occurence of the key from some deep group
string myValue = conf.group("foo")->group("bar")->value<string>("myKey", 2);

// Save new value
conf.group("foo")->group("bar")->setValue<string>("myKey", "newValue");

// Remove all groups named "bar" from root
conf.removeGroups("bar");

// Add three new values of integer type
conf.addValue<int>("a", 1);
conf.addValue<int>("a", 2);
conf.addValue<int>("a", 3);

conf.save();
@endcode

@section Configuration_Syntax File syntax
File syntax is based on INI syntax. Every row can be of one type:
- group header
- key/value pair
- comment / empty line

Rows can have leading/trailing whitespaces, they will be stripped on parsing and
saving. Comments and empty lines are preserved, unless the comment is in group
which was deleted.

%Configuration group header is enclosed in @c [ and @c ], hierarchic group names
are separated with @c / character. No group name can be empty.

Key/value pair consist of key name string, zero or more whitespaces, @c =
character, zero or more whitespaces and value. Whitespaces around the value are
stripped on parsing, if you want to preserve them, enclose the value in @c "
characters. The value cannot span multiple lines.

Comments begin with @c # or @c ; character and continue to the end of line. Each
line of multiline comments must begin with these characters.

Example %file:
<pre># Hierarchic group
[foo/bar]
myKey=myValue

# Multiple groups with the same name
[group]
a = 35.3
[group]
[group]
a = 19

# Value of custom type
vec = -3 2 17 0

; Another type of comment
</pre>

 * @todo Renaming, copying groups
 * @todo Use find() and equal_range() for faster (log) access
 * @todo Use some try/catch for parsing (avoid repeated code for group adding)
 * @todo Don't throw out whole group on invalid row
 * @todo EOL autodetection according to system on unsure/new files (default is
 *      preserve)
 * @todo Join ReadOnly / IsValid flag checks
 * @bug When value with number > 0 is not found, pointed integer is changed
 * @bug Setting inexistent value with number > 0 creates new key/value pair
 * @todo Test, whether the configurationValueToString() is called also with string type
 * @todo Support different syntax for hierarchic groups [g1][g2][...] along with
 *      [g1/g2/...]
 */
class UTILITY_EXPORT Configuration: public ConfigurationGroup {
    friend class ConfigurationGroup;

    DISABLE_COPY(Configuration)

    public:
        /** @brief Flags for opening configuration file */
        enum Flags {
            /**
             * Preserve Byte-Order-Mark in UTF-8 files, if present. Otherwise
             * the BOM will not be saved back into the file.
             */
            PreserveBom     = 0x01,

            /**
             * Force Unix line-endings (LF). Default behavior is to preserve
             * original. If original EOL type cannot be distinguished, Unix is
             * used.
             */
            ForceUnixEol    = 0x02,

            /**
             * Force Windows line endings (CR+LF).
             */
            ForceWindowsEol = 0x04,

            /**
             * Truncate the file. Don't load any configuration from the file.
             * On saving discards everything in the file and writes only newly
             * created values.
             */
            Truncate        = 0x08,

            /**
             * No comments or empty lines will be preserved on saving. Useful
             * for memory saving. See also Configuration::ReadOnly.
             */
            SkipComments    = 0x10,

            /**
             * Open the file read-only, which means faster access to elements
             * and less memory used. Sets also Configuration::SkipComments.
             * Adding, changing and removing groups and keys will not be
             * allowed.
             */
            ReadOnly        = 0x20,

            /**
             * Force unique groups. When loading the file only first group with
             * given name will be loaded, every other will be skipped and
             * discarded on saving. Also doesn't allow adding new group with
             * already existing name.
             */
            UniqueGroups    = 0x40,

            /**
             * Force unique keys. Only first value with given key (per group)
             * will be loaded, every other will be skipped and discarded on
             * saving. Also doesn't allow adding new value with the key name
             * already existing in the group.
             */
            UniqueKeys      = 0x80,

            /**
             * Force unique groups and keys. Same as
             * Configuration::UniqueGroups | Configuration::UniqueKeys .
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
         * @brief Constructor
         * @param _file     %Configuration file
         * @param _flags    Flags (see Configuration::Flags)
         *
         * Creates configuration from given istream. Sets flag
         * Configuration::ReadOnly, because the configuration cannot be saved
         * anywhere.
         */
        Configuration(std::istream& _file, int _flags = 0);

        /**
         * @brief Destructor
         *
         * If the configuration has been changed, writes configuration back to
         * the file. See also save().
         */
        inline ~Configuration() { if(flags & Changed) save(); }

        /** @brief Filename */
        inline std::string filename() const { return _filename; }

        /**
         * @brief Whether the file is valid
         * @return Returns false if the file has syntax errors or couldn't be
         *      opened, true otherwise.
         *
         * Invalid files cannot be changed or saved back.
         */
        inline bool isValid() const { return flags & IsValid; }

        /**
         * @brief Enable/disable automatic creation of inexistent groups
         * @param enabled   Whether to enable or disable this feature
         *
         * By default, calling Configuration::group() with inexistent name
         * returns null pointer. If this feature is enabled, the group is
         * automatically created if doesn't exist. Note that this works only for
         * first group with that name, not when requesting group with @c number
         * parameter set to non-zero value.
         * @todo Check readonly/isvalid
         */
        inline void setAutomaticGroupCreation(bool enabled)
            { flags |= AutoCreateGroups; }

        /**
         * @brief Whether automatic creation of inexistent groups is enabled
         * @see setAutomaticGroupCreation().
         */
        inline bool automaticGroupCreation() const
            { return flags & AutoCreateGroups; }

        /**
         * @brief Enable/disable automatic creation of inexistent key/value pairs.
         * @param enabled   Whether to enable or disable this feature
         *
         * By default, calling value() with inexistent key returns false and
         * pointed variable is unchanged. If this feature is enabled, the
         * key/value pair is automatically created from given key
         * name and value in pointed variable.
         * @todo Check readonly/isvalid
         */
        inline void setAutomaticKeyCreation(bool enabled)
            { flags |= AutoCreateKeys; }

        /**
         * @brief Whether automatic creation of inexistent keys is enabled
         * @see setAutomaticKeyCreation().
         */
        inline bool automaticKeyCreation() const
            { return flags & AutoCreateKeys; }

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

    private:
        /** @brief Private flags for file state */
        enum PrivateFlags {
            IsValid = 0x10000,      /**< @brief Whether the loaded file is valid */
            HasBom = 0x20000,       /**< @brief BOM mark was found in the file */
            WindowsEol = 0x40000,   /**< @brief The file has Windows line endings */
            Changed = 0x80000,      /**< @brief Whether the file has changed */
            AutoCreateGroups = 0x100000,    /**< @brief Automatically create inexistent groups */
            AutoCreateKeys = 0x200000       /**< @brief Automatically create inexistent keys */
        };

        /** @brief Configuration file */
        std::string _filename;

        /**
         * @brief Flags
         *
         * Combination of Configuration::Flags and Configuration::PrivateFlags.
         */
        int flags;

        void parse(std::istream& file);
        std::string parse(std::istream& file, ConfigurationGroup* group, const std::string& fullPath);

        void save(std::ofstream& file, const std::string& eol, ConfigurationGroup* group, const std::string& fullPath) const;
};

}}

#endif
