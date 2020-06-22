#ifndef Corrade_Utility_ConfigurationGroup_h
#define Corrade_Utility_ConfigurationGroup_h
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
 * @brief Class @ref Corrade::Utility::ConfigurationGroup
 */

#include <utility>
#include <string>
#include <vector>

#include "Corrade/Containers/StringStl.h"
#include "Corrade/Utility/ConfigurationValue.h"
#include "Corrade/Utility/Utility.h"
#include "Corrade/Utility/visibility.h"

namespace Corrade { namespace Utility {

/**
@brief Group of values in configuration file

Provides access to values and subgroups. See @ref Configuration class
documentation for usage example.
@todo Faster access to elements via multimap, find() and equal_range()
*/
class CORRADE_UTILITY_EXPORT ConfigurationGroup {
    friend Configuration;

    public:
        /**
         * @brief Default constructor
         *
         * Pointer to enclosing configuration is set to @cpp nullptr @ce, call
         * @ref addGroup() to add it somewhere.
         */
        explicit ConfigurationGroup();

        /**
         * @brief Copy constructor
         *
         * Pointer to enclosing configuration is set to @cpp nullptr @ce, call
         * @ref addGroup() to add it somewhere.
         */
        ConfigurationGroup(const ConfigurationGroup& other);

        /**
         * @brief Move constructor
         *
         * Pointer to enclosing configuration is set to @cpp nullptr @ce, call
         * @ref addGroup() to add it somewhere.
         */
        ConfigurationGroup(ConfigurationGroup&& other);

        ~ConfigurationGroup();

        /**
         * @brief Copy assignment
         *
         * Pointer to enclosing configuration stays the same as in original
         * object.
         * @see @ref configuration()
         */
        ConfigurationGroup& operator=(const ConfigurationGroup& other);

        /**
         * @brief Move assignment
         *
         * Pointer to enclosing configuration stays the same as in original
         * object.
         * @see @ref configuration()
         */
        ConfigurationGroup& operator=(ConfigurationGroup&& other);

        /**
         * @brief Enclosing configuration
         *
         * Returns @cpp nullptr @ce if the group is not part of any
         * configuration.
         * @see @ref addGroup(const std::string&, ConfigurationGroup*)
         */
        Configuration* configuration() { return _configuration; }
        const Configuration* configuration() const { return _configuration; } /**< @overload */

        /**
         * @brief Whether the group is empty
         *
         * If the group is empty, there aren't any values, subgroups, empty
         * lines or comments.
         * @see @ref hasGroups(), @ref hasValues()
         */
        bool isEmpty() const { return _values.empty() && _groups.empty(); }

        /** @{ @name Group operations */

        /**
         * @brief Whether this group has subgroups
         *
         * @see @ref isEmpty(), @ref hasGroup(), @ref groupCount(),
         *      @ref hasValues()
         */
        bool hasGroups() const { return !_groups.empty(); }

        /**
         * @brief Count of all subgroups
         *
         * @see @ref hasGroups(), @ref valueCount()
         */
        unsigned int groupCount() const {
            return static_cast<unsigned int>(_groups.size());
        }

        /**
         * @brief Whether given group exists
         * @param name      Name
         * @param index     Group index. Default is first found group.
         *
         * @see @ref isEmpty(), @ref hasGroups(), @ref groupCount(),
         *      @ref hasValue()
         */
        bool hasGroup(const std::string& name, unsigned int index = 0) const;

        /**
         * @brief Count of groups with given name
         *
         * @see @ref hasGroup(), @ref valueCount()
         */
        unsigned int groupCount(const std::string& name) const;

        /**
         * @brief Group of given name
         * @param name      Name
         * @param index     Group index. Default is first found group.
         *
         * Returns pointer to group on success, @cpp nullptr @ce otherwise.
         * @see @ref groups()
         */
        ConfigurationGroup* group(const std::string& name, unsigned int index = 0);
        const ConfigurationGroup* group(const std::string& name, unsigned int index = 0) const; /**< @overload */

        /** @brief All groups with given name */
        std::vector<ConfigurationGroup*> groups(const std::string& name);
        std::vector<const ConfigurationGroup*> groups(const std::string& name) const; /**< @overload */

        /**
         * @brief Add new group
         * @param name      Name. The name must not be empty and must not
         *      contain newline or any of `[]/` characters.
         * @param group     Existing group.
         *
         * Adds given group at the end of current group. The group must not be
         * part of any existing configuration.
         * @see @ref configuration()
         */
        void addGroup(const std::string& name, ConfigurationGroup* group);

        /**
         * @brief Add new group
         * @param name      Name. The name must not be empty and must not
         *      contain newline or any of `[]/` characters.
         *
         * Adds new group at the end of file, returns newly created group.
         */
        ConfigurationGroup* addGroup(const std::string& name);

        /**
         * @brief Remove group
         * @param name      Name of the group
         * @param index     Group index. Default is first found group.
         *
         * Returns @cpp true @ce if given group was found and removed,
         * @cpp false @ce otherwise.
         * @see @ref removeAllGroups(), @ref clear()
         */
        bool removeGroup(const std::string& name, unsigned int index = 0);

        /**
         * @brief Remove group
         *
         * Returns @cpp true @ce if given group was found and removed,
         * @cpp false @ce otherwise.
         * @see @ref removeAllGroups(), @ref clear()
         */
        bool removeGroup(ConfigurationGroup* group);

        /**
         * @brief Remove all groups with given name
         *
         * @see @ref removeGroup(), @ref clear()
         */
        void removeAllGroups(const std::string& name);

        /* Since 1.8.17, the original short-hand group closing doesn't work
           anymore. FFS. */
        /**
         * @}
         */

        /** @{ @name Value operations */

        /**
         * @brief Whether this group has any values
         *
         * @see @ref isEmpty(), @ref hasValue(), @ref valueCount(),
         *      @ref hasGroups()
         */
        bool hasValues() const;

        /**
         * @brief Count of all values in the group
         *
         * @see @ref hasValues(), @ref groupCount()
         */
        unsigned int valueCount() const;

        /**
         * @brief Whether value exists
         * @param key       Key
         * @param index     Value index. Default is first found value.
         *
         * @see @ref isEmpty(), @ref hasValues(), @ref valueCount(),
         *      @ref hasGroup()
         */
        bool hasValue(const std::string& key, unsigned int index = 0) const;

        /**
         * @brief Count of values with given key
         *
         * @see @ref hasValue(), @ref groupCount()
         */
        unsigned int valueCount(const std::string& key) const;

        /**
         * @brief Value
         * @param key       Key
         * @param index     Value index. Default is first found value.
         * @param flags     Flags
         *
         * Directly returns the value. If the key is not found, returns
         * default constructed value. If @p T is not @ref std::string, uses
         * @ref ConfigurationValue::fromString() to convert the value to given
         * type.
         * @see @ref hasValue()
         */
        template<class T = std::string> T value(const std::string& key, unsigned int index = 0, ConfigurationValueFlags flags = ConfigurationValueFlags()) const;

        /** @overload
         * Calls the above with @p index set to `0`.
         */
        template<class T = std::string> T value(const std::string& key, ConfigurationValueFlags flags) const {
            return value<T>(key, 0, flags);
        }

        /**
         * @brief All values with given key
         * @param key       Key
         * @param flags     Flags
         *
         * If @p T is not @ref std::string, uses
         * @ref ConfigurationValue::fromString() to convert the value to given
         * type.
         */
        template<class T = std::string> std::vector<T> values(const std::string& key, ConfigurationValueFlags flags = ConfigurationValueFlags()) const;

        /**
         * @brief Set string value
         * @param key       Key. The key must not be empty and must not contain
         *      newline or `=` character.
         * @param value     Value
         * @param index     Value index. Default is first found value.
         * @param flags     Flags
         *
         * If the key already exists, changes it to new value. If the key
         * doesn't exist, adds a new key with given name. Returns
         * @cpp false @ce if @p index is larger than actual value count,
         * @cpp true @ce otherwise.
         */
        bool setValue(const std::string& key, std::string value, unsigned int index = 0, ConfigurationValueFlags flags = ConfigurationValueFlags()) {
            return setValueInternal(key, std::move(value), index, flags);
        }

        /** @overload */
        bool setValue(const std::string& key, const char* value, unsigned int index = 0, ConfigurationValueFlags flags = ConfigurationValueFlags()) {
            return setValueInternal(key, value, index, flags);
        }

        /** @overload
         * Calls the above with @p index set to `0`.
         */
        bool setValue(const std::string& key, std::string value, ConfigurationValueFlags flags) {
            return setValue(key, std::move(value), 0, flags);
        }

        /** @overload
         * Calls the above with @p index set to `0`.
         */
        bool setValue(const std::string& key, const char* value, ConfigurationValueFlags flags) {
            return setValue(key, value, 0, flags);
        }

        /**
         * @brief Set value converted from given type
         *
         * Uses @ref ConfigurationValue::toString() to convert the value from
         * given type. See @ref setValue(const std::string&, std::string, unsigned int, ConfigurationValueFlags)
         * for more information.
         */
        template<class T> bool setValue(const std::string& key, const T& value, unsigned int index = 0, ConfigurationValueFlags flags = ConfigurationValueFlags()) {
            return setValueInternal(key, ConfigurationValue<T>::toString(value, flags), index, flags);
        }

        /** @overload
         * Calls the above with @p index set to `0`.
         */
        template<class T> bool setValue(const std::string& key, const T& value, ConfigurationValueFlags flags) {
            return setValue<T>(key, value, 0, flags);
        }

        /**
         * @brief Add new value
         * @param key       Key. The key must not be empty and must not contain
         *      newline or `=` character.
         * @param value     Value
         * @param flags     Flags
         *
         * Adds new key/value pair at the end of current group (it means also
         * after all comments).
         */
        void addValue(std::string key, std::string value, ConfigurationValueFlags flags = ConfigurationValueFlags()) {
            addValueInternal(std::move(key), std::move(value), flags);
        }

        /** @overload */
        void addValue(std::string key, const char* value, ConfigurationValueFlags flags = ConfigurationValueFlags()) {
            addValueInternal(std::move(key), value, flags);
        }

        /**
         * @brief Add new value
         *
         * Uses @ref ConfigurationValue::toString() to convert the value from
         * given type. See @ref addValue(std::string, std::string, ConfigurationValueFlags)
         * for more information.
         */
        template<class T> void addValue(std::string key, const T& value, ConfigurationValueFlags flags = ConfigurationValueFlags()) {
            addValueInternal(std::move(key), ConfigurationValue<T>::toString(value, flags), flags);
        }

        /**
         * @brief Remove value
         * @param key       Key
         * @param index     Value index. Default is first found value.
         *
         * Returns @cpp true @ce if given value was found and removed,
         * @cpp false @ce otherwise.
         * @see @ref removeAllValues(), @ref clear()
         */
        bool removeValue(const std::string& key, unsigned int index = 0);

        /**
         * @brief Remove all values with given key
         *
         * @see @ref removeValue(), @ref clear()
         */
        void removeAllValues(const std::string& key);

        /* Since 1.8.17, the original short-hand group closing doesn't work
           anymore. FFS. */
        /**
         * @}
         */

        /**
         * @brief Clear group
         *
         * Removes all values and subgroups.
         * @see @ref removeAllValues(), @ref removeAllGroups()
         */
        void clear();

    private:
        struct CORRADE_UTILITY_LOCAL Value {
            std::string key, value;
        };

        struct CORRADE_UTILITY_LOCAL Group {
            std::string name;
            ConfigurationGroup* group;
        };

        CORRADE_UTILITY_LOCAL explicit ConfigurationGroup(Configuration* configuration);

        CORRADE_UTILITY_LOCAL std::vector<Group>::iterator findGroup(const std::string& name, unsigned int index);
        CORRADE_UTILITY_LOCAL std::vector<Group>::const_iterator findGroup(const std::string& name, unsigned int index) const;
        CORRADE_UTILITY_LOCAL std::vector<Value>::iterator findValue(const std::string& key, unsigned int index);
        CORRADE_UTILITY_LOCAL std::vector<Value>::const_iterator findValue(const std::string& key, unsigned int index) const;

        /* Returns nullptr in case the key is not found */
        const std::string* valueInternal(const std::string& key, unsigned int index, ConfigurationValueFlags flags) const;
        std::vector<std::string> valuesInternal(const std::string& key, ConfigurationValueFlags flags) const;
        bool setValueInternal(const std::string& key, std::string value, unsigned int number, ConfigurationValueFlags flags);
        void addValueInternal(std::string key, std::string value, ConfigurationValueFlags flags);

        std::vector<Value> _values;
        std::vector<Group> _groups;

        Configuration* _configuration;
};

#ifndef DOXYGEN_GENERATING_OUTPUT
/* Shorthand template specialization for string values, delete unwanted ones */
template<> bool ConfigurationGroup::setValue(const std::string&, const std::string&, unsigned int, ConfigurationValueFlags) = delete;
template<> void ConfigurationGroup::addValue(std::string, const std::string&, ConfigurationValueFlags) = delete;
template<> inline std::string ConfigurationGroup::value(const std::string& key, unsigned int index, const ConfigurationValueFlags flags) const {
    const std::string* value = valueInternal(key, index, flags);
    return value ? *value : std::string{};
}
template<> inline std::vector<std::string> ConfigurationGroup::values(const std::string& key, const ConfigurationValueFlags flags) const {
    return valuesInternal(key, flags);
}
#endif

template<class T> inline T ConfigurationGroup::value(const std::string& key, const unsigned int index, const ConfigurationValueFlags flags) const {
    const std::string* value = valueInternal(key, index, flags);
    /* Can't do value ? *value : std::string{} BECAUSE THAT MAKES A COPY! C++
       YOU'RE FIRED */
    const std::string empty;
    return ConfigurationValue<T>::fromString(value ? *value : empty, flags);
}

template<class T> std::vector<T> ConfigurationGroup::values(const std::string& key, const ConfigurationValueFlags flags) const {
    std::vector<std::string> stringValues = valuesInternal(key, flags);
    std::vector<T> values;
    values.reserve(stringValues.size());
    for(std::vector<std::string>::const_iterator it = stringValues.begin(); it != stringValues.end(); ++it)
        values.push_back(ConfigurationValue<T>::fromString(*it, flags));

    return values;
}

}}

#endif
