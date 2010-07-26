#ifndef Map2X_Utility_ConfigurationGroup_h
#define Map2X_Utility_ConfigurationGroup_h
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
 * @brief Class Map2X::Utility::ConfigurationGroup
 */

#include <string>
#include <vector>

namespace Map2X { namespace Utility {

class Configuration;

/**
 * @brief Group of values in configuration file
 *
 * @todo Faster access to elements via multimap, find() and equal_range()
 */
class ConfigurationGroup {
    friend class Configuration;

    public:
        /** @brief Flags for value type */
        enum Flags {
            Oct     = 0x01,     /**< @brief Numeric value as octal */
            Hex     = 0x02,     /**< @brief Numeric value as hexadecimal */
            Color   = 0x04      /**< @brief Numeric value as color representation */
        };

        /** @brief Group name */
        inline std::string name() const { return _name; }

        /**
         * @brief Value
         * @param key       Key name
         * @param value     Pointer where to store value
         * @param number    Number of the value. Default is first found value.
         * @param flags     Flags (see ConfigurationGroup::Flags)
         * @return Whether the value was found
         *
         * See also Configuration::automaticKeyCreation().
         */
        template<class T> bool value(const std::string& key, T* value, unsigned int number = 0, int flags = 0);

        /**
         * @brief All values with given key name
         * @param key       Key name
         * @param flags     Flags (see ConfigurationGroup::Flags)
         * @return Vector with all found values
         */
        template<class T> std::vector<T> values(const std::string& key, int flags = 0) const;

        /**
         * @brief Count of values with given key name
         * @param key       Key name
         * @return Count
         *
         * See also Configuration::UniqueKeys and Configuration::UniqueNames.
         */
        unsigned int valueCount(const std::string& key) const;

        /**
         * @brief Set value
         * @param key       Key name
         * @param value     Value
         * @param number    Number of the value. Default is first found value.
         * @param flags     Flags (see ConfigurationGroup::Flags)
         * @return Whether the value was set. If the number is not 0 and the
         *      value with given number doesn't exist, returns false. See also
         *      Configuration::ReadOnly.
         *
         * If the key already exists, changes it to new value. If the key
         * doesn't exist, adds a new key with given name.
         */
        template<class T> bool setValue(const std::string& key, const T& value, unsigned int number = 0, int flags = 0);

        /**
         * @brief Add new value
         * @param key       Key name
         * @param value     Value
         * @param flags     Flags (see ConfigurationGroup::Flags)
         * @return Whether the value was added. See Configuration::ReadOnly,
         *      Configuration::UniqueKeys and Configuration::UniqueNames.
         *
         * Adds new key/value pair at the end of current group (it means also
         * after all comments).
         */
        template<class T> bool addValue(const std::string& key, const T& value, int flags = 0);

        /**
         * @brief Remove value
         * @param key       Key name
         * @param number    Number of the value
         * @return Whether the value was removed. If value with given number
         *      doesn't exist, returns false. See also Configuration::ReadOnly.
         */
        bool removeValue(const std::string& key, unsigned int number = 0);

        /**
         * @brief Remove all values with given key
         * @param key       Key name
         * @return Whether the values were removed. See Configuration::ReadOnly.
         *
         * @todo Return false if no values was removed?
         */
        bool removeAllValues(const std::string& key);

    private:
        /** @brief Configuration item */
        struct Item {
            std::string key,    /**< @brief Key name (only if item is key/value pair) */
                value;          /**< @brief Value or comment, empty line */
        };

        std::string _name;
        std::vector<Item> _items;
        Configuration* configuration;

        inline ConfigurationGroup(const std::string& name, std::vector<Item> items, Configuration* _configuration): _name(name), _items(items), configuration(_configuration) {}

        inline const std::vector<Item>& items() const { return _items; }
};

}}

#endif
