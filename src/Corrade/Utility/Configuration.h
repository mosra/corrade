#ifndef Corrade_Utility_Configuration_h
#define Corrade_Utility_Configuration_h
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

@m_keywords{Serialization Parsers}

Provides hierarchical configuration storage for key/value pairs with support
for parsing and loading into an textual INI-style file format. Basic API usage
example:

@snippet Utility.cpp Configuration-usage

@section Utility-Configuration-usage File syntax and usage

File syntax is based on the well-known INI format, consisting of three basic
elements:

- group header
- key/value pair
- comment / empty line

@subsection Utility-Configuration-usage-whitespace Whitespace and comments

Lines can have leading/trailing whitespace, which will be stripped on parsing
and saving. Whitespace is preserved in values enclosed in `"` and in multi-line
values. Comments and empty lines are preserved unless @ref Flag::SkipComments
is set in constructor or a group containing the comment gets deleted. It's not
possible to add comments or empty lines programatically.

Comments begin with @cb{.ini} # @ce or @cb{.ini} ; @ce character and continue
to the end of line. Each line of a multiline comment must begin with one of
these characters.

@code{.ini}
# A comment
  # on
  # multiple
# lines

; Another type of comment
@endcode

@subsection Utility-Configuration-usage-values Key/value pairs

Key/value pair consist of key name string, zero or more whitespace characters,
the `=` character, zero or more whitespace characters and the value. Whitespace
around the key is always stripped. Whitespace around the value is stripped on
parsing, if you want to preserve it, enclose the value in @cb{.py} " @ce
characters. Calling @ref setValue() with a string that contains
leading/trailing whitespace will cause it to be automatically wrapped in quotes
on save.

@code{.ini}
a=    a string with leading whitespace trimmed
b="    a string with leading whitespace preserved"
@endcode

All values are internally stored as strings, parsing to and saving from custom
types is done by calling an appropriate @ref ConfigurationValue. See its
documentation for a guide to integrating your own type, see
@ref corrade-configurationvalues for a list of additional parsers implemented
in Corrade itself.

Multi-line values are enclosed in @cb{.py} """ @ce alone on the line, first and
last line break is ignored. Calling @ref setValue() with a string that contains
newline characters will cause it to be automatically enclosed in
@cb{.py} """ @ce on save. The following value is parsed as two lines:

@code{.py}
value="""
Here is the value.
Spanning multiple lines.
"""
@endcode

It's allowed to have more than one value with the same key. You can access all
values for given key name using @ref values().

@code{.ini}
buy=bread
buy=milk
buy=apples
buy=onions
@endcode

@subsection Utility-Configuration-usage-groups Value groups

Value group header is enclosed in `[` and `]`. Group name is a non-empty
sequence of characters. As with values, it's allowed to have more than one
group with the same name.

@code{.ini}
[customer]
name=John
buys=bread

[customer]
name=Jacqueline
buys=cookies
@endcode

Hierarchic group names are separated with the `/` character. A group first
contains all values and then optional subgroups. A group is a subgroup of the
previous group if it has the previous group header as a prefix. In the
following snippet, *Jake* has *Mary* and *Max* as parents, while *Joanna*
and *Ferdinand* are his grandparents, Mary's parents.

@code{.ini}
name=Jake
likes=cycling

[parent]
name=Mary
likes=nature

[parent/parent]
name=Joanna
likes=cooking

[parent/parent]
name=Ferdinand
likes=smoking pipe

[parent]
name=Max
likes=books
@endcode

For a shorter syntax, it's possible to omit names of parent groups if they
contain only subgroups and given group is the first in the list. The following
two snippets are equivalent:

@code{.ini}
[org]
[org/java]
[org/java/lang]
class=AbstractBeanFactoryListener
class=AbstractFactoryListenerProviderDelegate

[org/java/system]
[org/java/system/services]
class=BeanFactoryListenerProviderDelegateGarbageAllocator
@endcode

@code{.ini}
[org/java/lang]
class=AbstractBeanFactoryListener
class=AbstractFactoryListenerProviderDelegate

[org/java/system/services]
class=BeanFactoryListenerProviderDelegateGarbageAllocator
@endcode

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
         * @see @ref Configuration(Flags) "Configuration()"
         */
        /* For some reason @ref Configuration() doesn't work since 1.8.17 */
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
         * @brief Save copy of the configuration to another file
         * @param filename  Filename in UTF-8
         *
         * The original @ref filename() is left untouched. Returns
         * @cpp true @ce on success, @cpp false @ce otherwise.
         */
        bool save(const std::string& filename);

        /** @brief Save configuration to stream */
        void save(std::ostream& out);

        /**
         * @brief Save configuration
         *
         * If @ref filename() is not empty, writes configuration back to the
         * file. Returns @cpp true @ce on success, @cpp false @ce otherwise.
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

        CORRADE_UTILITY_LOCAL bool parse(Containers::ArrayView<const char> in);
        CORRADE_UTILITY_LOCAL std::pair<Containers::ArrayView<const char>, const char*> parse(Containers::ArrayView<const char> in, ConfigurationGroup* group, const std::string& fullPath);
        CORRADE_UTILITY_LOCAL void save(std::ostream& out, const std::string& eol, ConfigurationGroup* group, const std::string& fullPath) const;

        CORRADE_UTILITY_LOCAL void setConfigurationPointer(ConfigurationGroup* group);

        std::string _filename;
        InternalFlags _flags;
};

CORRADE_ENUMSET_OPERATORS(Configuration::Flags)

}}

#endif
