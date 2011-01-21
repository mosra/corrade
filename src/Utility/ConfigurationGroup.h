#ifndef Kompas_Utility_ConfigurationGroup_h
#define Kompas_Utility_ConfigurationGroup_h
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
 * @brief Class Kompas::Utility::ConfigurationGroup
 */

#include <string>
#include <vector>
#include <sstream>

#include "utilities.h"

namespace Kompas { namespace Utility {

class Configuration;

/**
 * @brief Template structure for type conversion
 *
 * Functions in this struct are called internally by ConfigurationGroup
 * functions to convert values from and to templated types. Reimplement the
 * structure with template specialization to allow saving and getting
 * non-standard types into and from configuration files.
 * @todo Document implementation (include, namespace...)
 */
template<class T> struct ConfigurationValue {
    /**
    * @brief Convert value to string
    * @param value         Value
    * @param flags         Conversion flags (see ConfigurationGroup::Flags)
    * @return Value as string
    */
    static std::string toString(const T& value, int flags = 0);

    /**
    * @brief Convert value from string
    * @param stringValue   Value as string
    * @param flags         Conversion flags (see ConfigurationGroup::Flags)
    * @return Value
    */
    static T fromString(const std::string& stringValue, int flags = 0);
};

/**
 * @brief Group of values in configuration file
 *
 * @todo Faster access to elements via multimap, find() and equal_range()
 */
class UTILITY_EXPORT ConfigurationGroup {
    friend class Configuration;

    DISABLE_COPY(ConfigurationGroup)

    public:
        /** @brief Flags for value type */
        enum Flags {
            Oct         = 0x01, /**< @brief Numeric value as octal */
            Hex         = 0x02, /**< @brief Numeric value as hexadecimal */
            Color       = 0x04, /**< @brief Numeric value as color representation */
            Scientific  = 0x08  /**< @brief Floating point values in scientific notation */
        };

        /** @brief Destructor */
        ~ConfigurationGroup();

        /** @{ @name Group operations */

        /** @brief Group name */
        inline std::string name() const { return _name; }

        /**
         * @brief Get group
         * @param name      Name of the group
         * @param number    Number of the group. Default is first found group.
         * @return Pointer to group. If no group was found, returns null pointer.
         */
        ConfigurationGroup* group(const std::string& name, unsigned int number = 0);
        const ConfigurationGroup* group(const std::string& name, unsigned int number = 0) const; /**< @overload */

        /**
         * @brief Get all groups
         * @param name      Name of the group. If empty, returns all subgroups.
         * @return Vector of groups. If no group found, returns empty vector.
         */
        std::vector<ConfigurationGroup*> groups(const std::string& name = "");
        std::vector<const ConfigurationGroup*> groups(const std::string& name = "") const; /**< @overload */

        /**
         * @brief Count of groups with given name
         * @param name      Name of the group. If empty, returns number of all
         *      subgroups.
         * @return Count
         *
         * More efficient than calling <tt>groups(name).size()</tt>.
         * See also Configuration::UniqueGroups and Configuration::UniqueNames.
         */
        inline unsigned int groupCount(const std::string& name = "") const {
            if(name.empty()) return _groups.size();
            return groups(name).size();
        }

        /**
         * @brief Whether given group exists
         * @param name      Name of the group. If empty, returns true if there
         *      are any subgroups.
         *
         * More efficient than calling <tt>group(name) != 0</tt>.
         */
        inline bool groupExists(const std::string& name = "") const {
            if(name.empty()) return !_groups.empty();
            return group(name) != 0;
        }

        /**
         * @brief Add new group
         * @param name      Name of the group. The name must not be empty and
         *      must not contain '/' character.
         * @return Newly created group or null pointer when new group cannot be
         *      added (see above or flags Configuration::UniqueGroups and
         *      Configuration::ReadOnly).
         *
         * Adds new group at the end of file.
         */
        ConfigurationGroup* addGroup(const std::string& name);

        /**
         * @brief Remove group
         * @param name      Name of the group
         * @param number    Number of the group. Default is first found group.
         * @return Whether the groups were removed. (see above or flag
         *      Connfiguration::ReadOnly).
         */
        bool removeGroup(const std::string& name, unsigned int number = 0);

        /**
         * @brief Remove group
         * @param group     Pointer to the group
         * @return Whether the group was removed (see above or flag
         *      Configuration::ReadOnly).
         */
        bool removeGroup(ConfigurationGroup* group);

        /**
         * @brief Remove all groups with given name
         * @param name      Name of groups to remove
         * @return True if all groups with the given name were removed (see
         *      above or flag Connfiguration::ReadOnly).
         */
        bool removeAllGroups(const std::string& name);

        /*@}*/

        /** @{ @name Value operations */

        /**
         * @brief Value
         * @param key       Key name
         * @param _value    Pointer where to store value
         * @param number    Number of the value. Default is first found value.
         * @param flags     Flags (see ConfigurationGroup::Flags)
         * @return Whether the value was found
         *
         * See also Configuration::automaticKeyCreation().
         */
        template<class T> bool value(const std::string& key, T* _value, unsigned int number = 0, int flags = 0) {
            std::string stringValue = ConfigurationValue<T>::toString(*_value, flags);
            bool ret = value<std::string>(key, &stringValue, number, flags);

            *_value = ConfigurationValue<T>::fromString(stringValue, flags);
            return ret;
        }
        template<class T> bool value(const std::string& key, T* _value, unsigned int number = 0, int flags = 0) const {
            std::string stringValue;
            bool ret = value<std::string>(key, &stringValue, number, flags);

            *_value = ConfigurationValue<T>::fromString(stringValue, flags);
            return ret;
        } /**< @overload */

        /**
         * @brief Value (directly returned)
         * @param key       Key name
         * @param number    Number of the value. Default is first found value.
         * @param flags     Flags (see ConfigurationGroup::Flags)
         * @return Value
         *
         * Directly returns the value. If the key is not found, returns
         * default constructed value.
         */
        template<class T> T value(const std::string& key, unsigned int number = 0, int flags = 0) const {
            T _value;
            if(!value<T>(key, &_value, number, flags)) return T();
            return _value;
        }

        /**
         * @brief All values with given key name
         * @param key       Key name
         * @param flags     Flags (see ConfigurationGroup::Flags)
         * @return Vector with all found values
         */
        template<class T> std::vector<T> values(const std::string& key, int flags = 0) const {
            std::vector<T> _values;
            std::vector<std::string> stringValues = values<std::string>(key, flags);
            for(std::vector<std::string>::const_iterator it = stringValues.begin(); it != stringValues.end(); ++it)
                _values.push_back(ConfigurationValue<T>::fromString(*it, flags));

            return _values;
        }

        /**
         * @brief Count of keys with given name
         * @param key       Key name
         *
         * See also Configuration::UniqueKeys and Configuration::UniqueNames.
         */
        unsigned int keyCount(const std::string& key) const;

        /**
         * @brief Whether given key exists
         *
         * More efficient than calling <tt>valueCount(key) != 0</tt>.
         */
        bool keyExists(const std::string& key) const;

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
        template<class T> bool setValue(const std::string& key, const T& value, unsigned int number = 0, int flags = 0) {
            return setValue<std::string>(key, ConfigurationValue<T>::toString(value, flags), number, flags);
        }

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
        template<class T> bool addValue(const std::string& key, const T& value, int flags = 0) {
            return addValue<std::string>(key, ConfigurationValue<T>::toString(value, flags), flags);
        }

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
         * @return True if all values with given key were removed. See also
         *      Configuration::ReadOnly.
         */
        bool removeAllValues(const std::string& key);

        /*@}*/

    private:
        /** @brief Configuration item */
        struct Item {
            std::string key,    /**< @brief Key name (only if item is key/value pair) */
                value;          /**< @brief Value or comment, empty line */
        };

        std::string _name;                          /**< @brief Group name */
        std::vector<Item> items;                    /**< @brief Values and comments */
        std::vector<ConfigurationGroup*> _groups;   /**< @brief Subgroups */

        Configuration* configuration;

        ConfigurationGroup(const std::string& name, Configuration* _configuration);
};

#ifndef DOXYGEN_GENERATING_OUTPUT

template<class T> std::string ConfigurationValue<T>::toString(const T& value, int flags) {
    std::ostringstream stream;

    /* Hexadecimal / octal values */
    if(flags & (ConfigurationGroup::Color|ConfigurationGroup::Hex))
        stream.setf(std::istringstream::hex, std::istringstream::basefield);
    if(flags & ConfigurationGroup::Oct)
        stream.setf(std::istringstream::oct, std::istringstream::basefield);
    if(flags & ConfigurationGroup::Scientific)
        stream.setf(std::istringstream::scientific, std::istringstream::floatfield);

    stream << value;

    std::string stringValue = stream.str();

    /* Strip initial # character, if user wants a color */
    if(flags & ConfigurationGroup::Color)
        stringValue = '#' + stringValue;

    return stringValue;
}

template<class T> T ConfigurationValue<T>::fromString(const std::string& stringValue, int flags) {
    std::string _stringValue = stringValue;

    /* Strip initial # character, if user wants a color */
    if(flags & ConfigurationGroup::Color && !stringValue.empty() && stringValue[0] == '#')
        _stringValue = stringValue.substr(1);

    std::istringstream stream(_stringValue);

    /* Hexadecimal / octal values, scientific notation */
    if(flags & (ConfigurationGroup::Color|ConfigurationGroup::Hex))
        stream.setf(std::istringstream::hex, std::istringstream::basefield);
    if(flags & ConfigurationGroup::Oct)
        stream.setf(std::istringstream::oct, std::istringstream::basefield);
    if(flags & ConfigurationGroup::Scientific)
        stream.setf(std::istringstream::scientific, std::istringstream::floatfield);

    T value;
    stream >> value;

    return value;
}

/* Forward declared template specializations to avoid infinite recursion in
    template functions above */
template<> UTILITY_EXPORT bool ConfigurationGroup::value(const std::string& key, std::string* _value, unsigned int number, int flags);
template<> UTILITY_EXPORT bool ConfigurationGroup::value(const std::string& key, std::string* _value, unsigned int number, int flags) const;
template<> UTILITY_EXPORT std::vector<std::string> ConfigurationGroup::values(const std::string& key, int flags) const;
template<> UTILITY_EXPORT bool ConfigurationGroup::setValue(const std::string& key, const std::string& value, unsigned int number, int flags);
template<> UTILITY_EXPORT bool ConfigurationGroup::addValue(const std::string& key, const std::string& value, int flags);

template<> struct UTILITY_EXPORT ConfigurationValue<bool> {
    static bool fromString(const std::string& value, int flags);
    static std::string toString(const bool& value, int flags);
};

#endif

}}

#endif
