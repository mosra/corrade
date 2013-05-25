#ifndef Corrade_Utility_ConfigurationGroup_h
#define Corrade_Utility_ConfigurationGroup_h
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
 * @brief Class Corrade::Utility::ConfigurationGroup
 */

#include <vector>

#include "Utility/ConfigurationValue.h"
#include "Utility/Utility.h"

namespace Corrade { namespace Utility {

/**
@brief Group of values in configuration file

Provides access to values and subgroups. See Configuration class documentation
for usage example.
@todo Faster access to elements via multimap, find() and equal_range()
*/
class CORRADE_UTILITY_EXPORT ConfigurationGroup {
    friend class Configuration;

    public:
        /** @brief Copy constructor */
        ConfigurationGroup(const ConfigurationGroup& other);

        /** @brief Destructor */
        ~ConfigurationGroup();

        /** @brief Assignment operator */
        ConfigurationGroup& operator=(const ConfigurationGroup& other);

        /** @{ @name Group operations */

        /**
         * @brief Group
         * @param name      Name of the group
         * @param number    Number of the group. Default is first found group.
         * @return Pointer to group. If no group was found, returns null pointer.
         */
        ConfigurationGroup* group(const std::string& name, unsigned int number = 0);
        const ConfigurationGroup* group(const std::string& name, unsigned int number = 0) const; /**< @overload */

        /**
         * @brief All groups
         * @param name      Name of the group. If empty, returns all subgroups.
         * @return Vector of groups. If no group is found, returns empty vector.
         */
        std::vector<ConfigurationGroup*> groups(const std::string& name = std::string());
        std::vector<const ConfigurationGroup*> groups(const std::string& name = std::string()) const; /**< @overload */

        /**
         * @brief Count of groups with given name
         * @param name      Name of the group. If empty, returns number of all
         *      subgroups.
         * @return Count
         *
         * More efficient than calling `groups(name).size()`.
         * See also Configuration::UniqueGroups and Configuration::UniqueNames.
         */
        unsigned int groupCount(const std::string& name = std::string()) const;

        /**
         * @brief Whether given group exists
         * @param name      Name of the group. If empty, returns true if there
         *      are any subgroups.
         *
         * More efficient than calling `group(name) != nullptr`.
         * @todo split out to hasSubgroups()?
         */
        bool groupExists(const std::string& name = std::string()) const;

        /**
         * @brief Add new group
         * @param name      Name of the group. The name must not be empty and
         *      must not contain '/' character.
         * @param group     Existing group.
         * @return False if the group cannot be added (see above or flags
         *      Configuration::UniqueGroups and Configuration::ReadOnly), true
         *      otherwise.
         *
         * Adds existing group at the end of file. Note that the function
         * doesn't check whether the same group already exists in the
         * configuration - adding such group can result in infinite cycle when
         * saving.
         */
        bool addGroup(const std::string& name, ConfigurationGroup* group);

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
         * @return Whether the group was removed (see above or flag
         *      Configuration::ReadOnly).
         *
         * @see clear()
         */
        bool removeGroup(const std::string& name, unsigned int number = 0);

        /**
         * @brief Remove group
         * @param group     Pointer to the group
         * @return Whether the group was removed (see above or flag
         *      Configuration::ReadOnly).
         *
         * @see clear()
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
         * @brief String value
         * @param key       Key name
         * @param value     Pointer where to store value
         * @param number    Number of the value. Default is first found value.
         * @param flags     Flags (see ConfigurationGroup::Flags)
         * @return Whether the value was found
         *
         * See also Configuration::automaticKeyCreation().
         */
        bool value(const std::string& key, std::string* _value, unsigned int number = 0, ConfigurationValueFlags flags = ConfigurationValueFlags()) {
            return valueInternal(key, _value, number, flags);
        }

        /** @overload */
        bool value(const std::string& key, std::string* _value, unsigned int number = 0, ConfigurationValueFlags flags = ConfigurationValueFlags()) const {
            return valueInternal(key, _value, number, flags);
        }

        /**
         * @brief Value converted to given type
         *
         * Uses ConfigurationValue to convert the value to given type. See
         * value(const std::string&, std::string*, unsigned int, ConfigurationValueFlags)
         * for more information.
         */
        template<class T> bool value(const std::string& key, T* value, unsigned int number = 0, ConfigurationValueFlags flags = ConfigurationValueFlags());
        template<class T> bool value(const std::string& key, T* value, unsigned int number = 0, ConfigurationValueFlags flags = ConfigurationValueFlags()) const; /**< @overload */

        /**
         * @brief Value (directly returned)
         * @param key       Key name
         * @param number    Number of the value. Default is first found value.
         * @param flags     Flags (see ConfigurationGroup::Flags)
         * @return Value
         *
         * Directly returns the value. If the key is not found, returns
         * default constructed value. If @p T is not `std::string`, uses
         * ConfigurationValue::fromString() to convert the value to given type.
         */
        template<class T = std::string> T value(const std::string& key, unsigned int number = 0, ConfigurationValueFlags flags = ConfigurationValueFlags()) const;

        /**
         * @brief All values with given key name
         * @param key       Key name
         * @param flags     Flags (see ConfigurationGroup::Flags)
         * @return Vector with all found values
         *
         * If @p T is not `std::string`, uses ConfigurationValue::fromString()
         * to convert the value to given type.
         */
        template<class T = std::string> std::vector<T> values(const std::string& key, ConfigurationValueFlags flags = ConfigurationValueFlags()) const;

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
         * More efficient than calling `valueCount(key) != 0`.
         */
        bool keyExists(const std::string& key) const;

        /**
         * @brief Set string value
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
        bool setValue(const std::string& key, std::string value, unsigned int number = 0, ConfigurationValueFlags flags = ConfigurationValueFlags()) {
            return setValueInternal(key, std::move(value), number, flags);
        }

        /** @overload */
        bool setValue(const std::string& key, const char* value, unsigned int number = 0, ConfigurationValueFlags flags = ConfigurationValueFlags()) {
            return setValueInternal(key, value, number, flags);
        }

        /**
         * @brief Set value converted from given type
         *
         * Uses ConfigurationValue::toString() to convert the value from given
         * type. See setValue(const std::string&, std::string, unsigned int, ConfigurationValueFlags)
         * for more information.
         */
        template<class T> bool setValue(const std::string& key, const T& value, unsigned int number = 0, ConfigurationValueFlags flags = ConfigurationValueFlags()) {
            return setValueInternal(key, ConfigurationValue<T>::toString(value, flags), number, flags);
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
        bool addValue(std::string key, std::string value, const ConfigurationValueFlags flags = ConfigurationValueFlags()) {
            return addValueInternal(std::move(key), std::move(value), flags);
        }

        /** @overload */
        bool addValue(std::string key, const char* value, const ConfigurationValueFlags flags = ConfigurationValueFlags()) {
            return addValueInternal(std::move(key), value, flags);
        }

        /**
         * @brief Add new value
         *
         * Uses ConfigurationValue::toString() to convert the value from given
         * type. See addValue(const std::string&, std::string, ConfigurationValueFlags)
         * for more information.
         */
        template<class T> bool addValue(std::string key, const T& value, ConfigurationValueFlags flags = ConfigurationValueFlags()) {
            return addValueInternal(std::move(key), ConfigurationValue<T>::toString(value, flags), flags);
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

        /**
         * @brief Clear group
         * @return Whether the group was cleared (see above or flag
         *      Configuration::ReadOnly).
         *
         * Clears all values and subgroups. See also removeGroup().
         */
        bool clear();

    private:
        /** @brief Configuration item */
        struct CORRADE_UTILITY_LOCAL Item {
            std::string key,    /**< @brief Key name (only if item is key/value pair) */
                value;          /**< @brief Value or comment, empty line */
        };

        /** @brief Configuration group */
        struct CORRADE_UTILITY_LOCAL Group {
            std::string name;           /**< @brief Group name */
            ConfigurationGroup* group;  /**< @brief Configuration group */
        };

        std::vector<Item> items;                    /**< @brief Values and comments */
        std::vector<Group> _groups;                 /**< @brief Subgroups */

        Configuration* configuration;

        /** @brief Constructor */
        CORRADE_UTILITY_LOCAL explicit ConfigurationGroup(Configuration* _configuration): configuration(_configuration) {}

        CORRADE_UTILITY_EXPORT bool valueInternal(const std::string& key, std::string* _value, unsigned int number, ConfigurationValueFlags flags);
        CORRADE_UTILITY_EXPORT bool valueInternal(const std::string& key, std::string* _value, unsigned int number, ConfigurationValueFlags flags) const;
        CORRADE_UTILITY_EXPORT std::vector<std::string> valuesInternal(const std::string& key, ConfigurationValueFlags flags) const;
        CORRADE_UTILITY_EXPORT bool setValueInternal(const std::string& key, std::string value, unsigned int number, ConfigurationValueFlags flags);
        CORRADE_UTILITY_EXPORT bool addValueInternal(std::string key, std::string value, ConfigurationValueFlags flags);
};

#ifndef DOXYGEN_GENERATING_OUTPUT
/* Shorthand template specialization for string values, delete unwanted ones */
template<> bool ConfigurationGroup::value(const std::string&, std::string*, unsigned int, ConfigurationValueFlags flags) = delete;
template<> bool ConfigurationGroup::value(const std::string&, std::string*, unsigned int, ConfigurationValueFlags flags) const = delete;
template<> bool ConfigurationGroup::setValue(const std::string&, const std::string&, unsigned int, ConfigurationValueFlags) = delete;
template<> bool ConfigurationGroup::addValue(std::string, const std::string&, ConfigurationValueFlags) = delete;
template<> inline std::string ConfigurationGroup::value(const std::string& key, unsigned int number, const ConfigurationValueFlags flags) const {
    std::string stringValue;
    valueInternal(key, &stringValue, number, flags);
    return std::move(stringValue);
}
template<> inline std::vector<std::string> ConfigurationGroup::values(const std::string& key, const ConfigurationValueFlags flags) const {
    return valuesInternal(key, flags);
}
#endif

template<class T> bool ConfigurationGroup::value(const std::string& key, T* value, const unsigned int number, const ConfigurationValueFlags flags) {
    std::string stringValue = ConfigurationValue<T>::toString(*value, flags);
    const bool ret = valueInternal(key, &stringValue, number, flags);

    *value = ConfigurationValue<T>::fromString(stringValue, flags);
    return ret;
}

template<class T> bool ConfigurationGroup::value(const std::string& key, T* value, const unsigned int number, const ConfigurationValueFlags flags) const {
    std::string stringValue;
    const bool ret = valueInternal(key, &stringValue, number, flags);

    *value = ConfigurationValue<T>::fromString(stringValue, flags);
    return ret;
}

template<class T> T ConfigurationGroup::value(const std::string& key, const unsigned int number, const ConfigurationValueFlags flags) const {
    T _value;
    if(!value<T>(key, &_value, number, flags)) return T();
    return std::move(_value);
}

template<class T> std::vector<T> ConfigurationGroup::values(const std::string& key, const ConfigurationValueFlags flags) const {
    std::vector<T> _values;
    std::vector<std::string> stringValues = valuesInternal(key, flags);
    for(std::vector<std::string>::const_iterator it = stringValues.begin(); it != stringValues.end(); ++it)
        _values.push_back(ConfigurationValue<T>::fromString(*it, flags));

    return _values;
}

}}

#endif
