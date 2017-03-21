#ifndef Corrade_Utility_Configuration_h
#define Corrade_Utility_Configuration_h
/*
    This file is part of Corrade.

    Copyright © 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016,
                2017 Vladimír Vondruš <mosra@centrum.cz>

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
 * @brief Class @ref Corrade::Utility::Configuration
 */

#include <cstdint>
#include <string>
#include <iosfwd>

#include "Corrade/Containers/EnumSet.h"
#include "Corrade/Utility/ConfigurationGroup.h"
#include "Corrade/Utility/visibility.h"

namespace Corrade { namespace Utility {

/**
@brief Parser and writer for configuration files

Provides hierarchical configuration storage. The key/value pairs are stored in
hierarchical groups, this class acts as an root configuration group. Supported
are either non-unique or unique group and key names, uniqueness can be enforced
via flag in constructor. See @ref ConfigurationGroup class documentation for
accessing, deleting, adding and setting groups and values.

Values can be saved and retrieved using templated function, so it's possible to
implement saving for any type. See @ref ConfigurationValue documentation for an
example.

## Example usage

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

## File syntax¨

File syntax is based on INI syntax, consisting of three basic elements:

- group header
- key/value pair
- comment / empty line

Elements can have leading/trailing whitespaces, they will be stripped on
parsing and saving. Whitespace is preserved in values enclosed in `"` and
multi-line values. Comments and empty lines are preserved, unless the comment
is in group which was deleted.

Configuration group header is enclosed in `[` and `]`, hierarchic group names
are separated with `/` character. No group name can be empty.

Key/value pair consist of key name string, zero or more whitespaces, `=`
character, zero or more whitespaces and value. Whitespaces around the value are
stripped on parsing, if you want to preserve them, enclose the value in `"`
characters.

Multi-line values are enclosed in `"""` alone on the line, first and last line
break is ignored. The following value is parsed as two lines:

    value="""
    Here is the value.
    Spanning multiple lines.
    """

Comments begin with `#` or `;` character and continue to the end of line. Each
line of multiline comments must begin with these characters.

Example file:

    # Hierarchic group
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

@todo Renaming, copying groups
@todo EOL autodetection according to system on unsure/new files (default is
    preserve)
@todo Test that the configurationValueToString() isn't called with string type
    (e.g. value with spaces)
*/
class CORRADE_UTILITY_EXPORT Configuration: public ConfigurationGroup {
    friend ConfigurationGroup;

    public:
        /**
         * @brief Flag for opening configuration file
         *
         * @see @ref Flags
         */
        enum class Flag: std::uint32_t {
            /**
             * Preserve Byte-Order-Mark in UTF-8 files, if present. Otherwise
             * the BOM will not be saved back into the file.
             */
            PreserveBom     = 1 << 0,

            /**
             * Force Unix line-endings (LF). Default behavior is to preserve
             * original. If original EOL type cannot be distinguished, Unix is
             * used.
             */
            ForceUnixEol    = 1 << 1,

            /** Force Windows line endings (CR+LF). */
            ForceWindowsEol = 1 << 2,

            /**
             * Don't load any configuration from the file. On saving writes
             * only newly created values.
             */
            Truncate        = 1 << 3,

            /**
             * No comments or empty lines will be preserved on saving. Useful
             * for memory saving and faster access. See also
             * @ref Flag::ReadOnly.
             */
            SkipComments    = 1 << 4,

            /**
             * Open the file read-only, which means faster access to elements
             * and less memory used. Filename is not saved to avoid overwriting
             * the file with @ref save(). See also @ref Flag::SkipComments.
             */
            ReadOnly        = 1 << 5
        };

        /**
         * @brief Flags for opening configuration file
         *
         * @see @ref Configuration::Configuration()
         */
        typedef Containers::EnumSet<Flag> Flags;

        /**
         * @brief Default constructor
         *
         * Creates empty configuration with no filename.
         */
        explicit Configuration(Flags flags = Flags());

        /**
         * @brief Constructor
         * @param filename  Filename in UTF-8
         * @param flags     Flags
         *
         * Opens the file and loads it according to specified flags. If file
         * cannot be opened or parsed, the configuration is empty and filename
         * is not saved to avoid overwriting the file with @ref save().
         */
        explicit Configuration(const std::string& filename, Flags flags = Flags());

        /**
         * @brief Constructor
         * @param in        Input stream
         * @param flags     Flags
         *
         * Creates configuration from given input stream.
         */
        explicit Configuration(std::istream& in, Flags flags = Flags());

        /** @brief Copying is not allowed */
        Configuration(const Configuration&) = delete;

        /** @brief Move constructor */
        Configuration(Configuration&& other);

        /**
         * @brief Destructor
         *
         * If the configuration has been changed, calls @ref save().
         */
        ~Configuration();

        /** @brief Copying is not allowed */
        Configuration& operator=(const Configuration&) = delete;

        /** @brief Move assignment */
        Configuration& operator=(Configuration&& other);

        /** @brief Filename in UTF-8 */
        std::string filename() const;

        /**
         * @brief Set filename
         *
         * The filename is used by @ref save() and is expected to be UTF-8
         * encoded.
         */
        void setFilename(std::string filename);

        /** @brief Whether the file was successfully parsed */
        bool isValid() const { return bool(_flags & InternalFlag::IsValid); }

        /**
         * @brief Save configuration to given file
         * @param filename  Filename in UTF-8
         *
         * Returns `true` on success, `false` otherwise.
         */
        bool save(const std::string& filename);

        /** @brief Save configuration to stream */
        void save(std::ostream& out);

        /**
         * @brief Save configuration
         *
         * If @ref filename() is not empty, writes configuration back to the
         * file. Returns `true` on success, `false` otherwise.
         */
        bool save();

    private:
        enum class InternalFlag: std::uint32_t {
            PreserveBom     = std::uint32_t(Flag::PreserveBom),
            ForceUnixEol    = std::uint32_t(Flag::ForceUnixEol),
            ForceWindowsEol = std::uint32_t(Flag::ForceWindowsEol),
            Truncate        = std::uint32_t(Flag::Truncate),
            SkipComments    = std::uint32_t(Flag::SkipComments),
            ReadOnly        = std::uint32_t(Flag::ReadOnly),

            IsValid = 1 << 16,
            HasBom = 1 << 17,
            WindowsEol = 1 << 18,
            Changed = 1 << 19
        };

        typedef Containers::EnumSet<InternalFlag> InternalFlags;

        CORRADE_ENUMSET_FRIEND_OPERATORS(InternalFlags)

        CORRADE_UTILITY_LOCAL bool parse(std::istream& in);
        CORRADE_UTILITY_LOCAL std::string parse(std::istream& in, ConfigurationGroup* group, const std::string& fullPath);
        CORRADE_UTILITY_LOCAL void save(std::ostream& out, const std::string& eol, ConfigurationGroup* group, const std::string& fullPath) const;

        CORRADE_UTILITY_LOCAL void setConfigurationPointer(ConfigurationGroup* group);

        std::string _filename;
        InternalFlags _flags;
};

CORRADE_ENUMSET_OPERATORS(Configuration::Flags)
CORRADE_ENUMSET_OPERATORS(Configuration::InternalFlags)

}}

#endif
