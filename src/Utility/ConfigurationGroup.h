#ifndef Corrade_Utility_ConfigurationGroup_h
#define Corrade_Utility_ConfigurationGroup_h
/*
    Copyright © 2007, 2008, 2009, 2010, 2011, 2012
              Vladimír Vondruš <mosra@centrum.cz>

    This file is part of Corrade.

    Corrade is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License version 3
    only, as published by the Free Software Foundation.

    Corrade is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU Lesser General Public License version 3 for more details.
*/

/** @file
 * @brief Class Corrade::Utility::ConfigurationGroup
 */

#include <vector>

#include "ConfigurationValue.h"
#include "utilities.h"

namespace Corrade { namespace Utility {

class Configuration;

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
        std::vector<ConfigurationGroup*> groups(const std::string& name = "");
        std::vector<const ConfigurationGroup*> groups(const std::string& name = "") const; /**< @overload */

        /**
         * @brief Count of groups with given name
         * @param name      Name of the group. If empty, returns number of all
         *      subgroups.
         * @return Count
         *
         * More efficient than calling `groups(name).size()`.
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
         * More efficient than calling `group(name) != 0`.
         * @todo split out to hasSubgroups()?
         */
        inline bool groupExists(const std::string& name = "") const {
            if(name.empty()) return !_groups.empty();
            return group(name) != nullptr;
        }

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
         * @brief Value
         * @param key       Key name
         * @param value     Pointer where to store value
         * @param number    Number of the value. Default is first found value.
         * @param flags     Flags (see ConfigurationGroup::Flags)
         * @return Whether the value was found
         *
         * See also Configuration::automaticKeyCreation().
         */
        template<class T> bool value(const std::string& key, T* value, unsigned int number = 0, ConfigurationValueFlags flags = ConfigurationValueFlags()) {
            std::string stringValue = ConfigurationValue<T>::toString(*value, flags);
            bool ret = valueInternal(key, &stringValue, number, flags);

            *value = ConfigurationValue<T>::fromString(stringValue, flags);
            return ret;
        }
        template<class T> bool value(const std::string& key, T* value, unsigned int number = 0, ConfigurationValueFlags flags = ConfigurationValueFlags()) const {
            std::string stringValue;
            bool ret = valueInternal(key, &stringValue, number, flags);

            *value = ConfigurationValue<T>::fromString(stringValue, flags);
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
        template<class T = std::string> T value(const std::string& key, unsigned int number = 0, ConfigurationValueFlags flags = ConfigurationValueFlags()) const {
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
        template<class T = std::string> std::vector<T> values(const std::string& key, ConfigurationValueFlags flags = ConfigurationValueFlags()) const {
            std::vector<T> _values;
            std::vector<std::string> stringValues = valuesInternal(key, flags);
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
         * More efficient than calling `valueCount(key) != 0`.
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
        template<class T> bool setValue(const std::string& key, const T& value, unsigned int number = 0, ConfigurationValueFlags flags = ConfigurationValueFlags()) {
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
        template<class T> bool addValue(const std::string& key, const T& value, ConfigurationValueFlags flags = ConfigurationValueFlags()) {
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
        CORRADE_UTILITY_LOCAL ConfigurationGroup(Configuration* _configuration): configuration(_configuration) {}

        CORRADE_UTILITY_EXPORT bool valueInternal(const std::string& key, std::string* _value, unsigned int number, ConfigurationValueFlags flags);
        CORRADE_UTILITY_EXPORT bool valueInternal(const std::string& key, std::string* _value, unsigned int number, ConfigurationValueFlags flags) const;
        CORRADE_UTILITY_EXPORT std::vector<std::string> valuesInternal(const std::string& key, ConfigurationValueFlags flags) const;
        CORRADE_UTILITY_EXPORT bool setValueInternal(const std::string& key, const std::string& value, unsigned int number, ConfigurationValueFlags flags);
        CORRADE_UTILITY_EXPORT bool addValueInternal(const std::string& key, const std::string& value, ConfigurationValueFlags flags);
};

#ifndef DOXYGEN_GENERATING_OUTPUT
/* Forward declared template specializations to avoid infinite recursion in
    template functions above */
template<> inline bool ConfigurationGroup::value(const std::string& key, std::string* _value, unsigned int number, ConfigurationValueFlags flags) {
    return valueInternal(key, _value, number, flags);
}
template<> inline bool ConfigurationGroup::value(const std::string& key, std::string* _value, unsigned int number, ConfigurationValueFlags flags) const {
    return valueInternal(key, _value, number, flags);
}
template<> inline std::vector<std::string> ConfigurationGroup::values(const std::string& key, ConfigurationValueFlags flags) const {
    return valuesInternal(key, flags);
}
template<> inline bool ConfigurationGroup::setValue(const std::string& key, const std::string& value, unsigned int number, ConfigurationValueFlags flags) {
    return setValueInternal(key, value, number, flags);
}
template<> inline bool ConfigurationGroup::addValue(const std::string& key, const std::string& value, ConfigurationValueFlags flags) {
    return addValueInternal(key, value, flags);
}
#endif

}}

#endif
