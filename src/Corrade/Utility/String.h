#ifndef Corrade_Utility_String_h
#define Corrade_Utility_String_h
/*
    This file is part of Corrade.

    Copyright © 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015
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
 * @brief Class @ref Corrade::Utility::String
 */

#include <cstddef>
#include <string>
#include <vector>

#include "Corrade/configure.h"
#include "Corrade/Containers/ArrayView.h"
#include "Corrade/Utility/visibility.h"

namespace Corrade { namespace Utility {

/** @brief String utilities */
class CORRADE_UTILITY_EXPORT String {
    public:
        String() = delete;

        /**
         * @brief Safely construct string from char array
         *
         * If @p string is `nullptr`, returns empty string.
         */
        static std::string fromArray(const char* string) {
            return string ? std::string{string} : std::string{};
        }

        /**
         * @brief Safely construct string from char array
         *
         * If @p string is `nullptr`, returns empty string. Otherwise takes
         * also @p length into account.
         */
        static std::string fromArray(const char* string, std::size_t length) {
            return string ? std::string{string, length} : std::string{};
        }

        /**
         * @brief Trim leading characters from string
         * @param string        String to be trimmed
         * @param characters    Characters which will be trimmed
         *
         * @see @ref rtrim(), @ref trim()
         */
        static std::string ltrim(std::string string, const std::string& characters) {
            return ltrimInternal(std::move(string), {characters.data(), characters.size()});
        }

        /** @overload */
        template<std::size_t size> static std::string ltrim(std::string string, const char(&characters)[size]) {
            return ltrimInternal(std::move(string), {characters, size - 1});
        }

        /**
         * @brief Trim leading whitespace from string
         *
         * Equivalent to calling the above function with <tt>" \t\f\v\r\n"</tt>
         * as second parameter.
         * @see @ref rtrim(), @ref trim()
         */
        static std::string ltrim(std::string string);

        /**
         * @brief Trim trailing characters from string
         * @param string        String to be trimmed
         * @param characters    Characters which will be trimmed
         *
         * @see @ref ltrim(), @ref trim()
         */
        static std::string rtrim(std::string string, const std::string& characters) {
            return rtrimInternal(std::move(string), {characters.data(), characters.size()});
        }

        /** @overload */
        template<std::size_t size> static std::string rtrim(std::string string, const char(&characters)[size]) {
            return rtrimInternal(std::move(string), {characters, size - 1});
        }

        /**
         * @brief Trim trailing whitespace from string
         *
         * Equivalent to calling the above function with <tt>" \t\f\v\r\n"</tt>
         * as second parameter.
         * @see @ref ltrim(), @ref trim()
         */
        static std::string rtrim(std::string string);

        /**
         * @brief Trim leading and trailing characters from string
         * @param string        String to be trimmed
         * @param characters    Characters which will be trimmed
         *
         * Equivalent to `ltrim(rtrim(string))`.
         */
        static std::string trim(std::string string, const std::string& characters) {
            return trimInternal(std::move(string), {characters.data(), characters.size()});
        }

        /** @overload */
        template<std::size_t size> static std::string trim(std::string string, const char(&characters)[size]) {
            return trimInternal(std::move(string), {characters, size - 1});
        }

        /**
         * @brief Trim leading and trailing whitespace from string
         *
         * Equivalent to calling the above function with <tt>" \t\f\v\r\n"</tt>
         * as second parameter.
         */
        static std::string trim(std::string string);

        /**
         * @brief Split string on given character
         * @param string            String to split
         * @param delimiter         Delimiter
         */
        static std::vector<std::string> split(const std::string& string, char delimiter);

        /**
         * @brief Split string on given character and remove empty parts
         * @param string            String to split
         * @param delimiter         Delimiter
         */
        static std::vector<std::string> splitWithoutEmptyParts(const std::string& string, char delimiter);

        /**
         * @brief Split string on any character from given set and remove empty parts
         * @param string            String to split
         * @param delimiters        Delimiter characters
         */
        static std::vector<std::string> splitWithoutEmptyParts(const std::string& string, const std::string& delimiters) {
            return splitWithoutEmptyPartsInternal(string, {delimiters.data(), delimiters.size()});
        }

        /** @overload */
        template<std::size_t size> static std::vector<std::string> splitWithoutEmptyParts(const std::string& string, const char(&delimiters)[size]) {
            return splitWithoutEmptyPartsInternal(string, {delimiters, size - 1});
        }

        /**
         * @brief Split string on whitespaces and remove empty parts
         *
         * Equivalent to calling the above function with <tt>" \t\f\v\r\n"</tt>
         * as second parameter.
         */
        static std::vector<std::string> splitWithoutEmptyParts(const std::string& string);

        /**
         * @brief Join strings with given character
         * @param strings           Strings to join
         * @param delimiter         Delimiter
         */
        static std::string join(const std::vector<std::string>& strings, char delimiter);

        /**
         * @brief Join strings with given character and remove empty parts
         * @param strings           Strings to join
         * @param delimiter         Delimiter
         */
        static std::string joinWithoutEmptyParts(const std::vector<std::string>& strings, char delimiter);

        /**
         * @brief Convert string to lowercase
         *
         * @attention Doesn't work with UTF-8.
         */
        static std::string lowercase(std::string string);

        /**
         * @brief Convert string to uppercase
         *
         * @attention Doesn't work with UTF-8.
         */
        static std::string uppercase(std::string string);

        /** @brief Whether the string has given prefix */
        static bool beginsWith(const std::string& string, const std::string& prefix) {
            return beginsWithInternal(string, {prefix.data(), prefix.size()});
        }

        /** @overload */
        template<std::size_t size> static bool beginsWith(const std::string& string, const char(&prefix)[size]) {
            return beginsWithInternal(string, {prefix, size - 1});
        }

        /** @brief Whether the string has given suffix */
        static bool endsWith(const std::string& string, const std::string& suffix) {
            return endsWithInternal(string, {suffix.data(), suffix.size()});
        }

        /** @overload */
        template<std::size_t size> static bool endsWith(const std::string& string, const char(&suffix)[size]) {
            return endsWithInternal(string, {suffix, size - 1});
        }

    private:
        static std::string ltrimInternal(std::string string, Containers::ArrayView<const char> characters);
        static std::string rtrimInternal(std::string string, Containers::ArrayView<const char> characters);
        static std::string trimInternal(std::string string, Containers::ArrayView<const char> characters);

        static std::vector<std::string> splitWithoutEmptyPartsInternal(const std::string& string, Containers::ArrayView<const char> delimiters);

        static bool beginsWithInternal(const std::string& string, Containers::ArrayView<const char> prefix);
        static bool endsWithInternal(const std::string& string, Containers::ArrayView<const char> suffix);
};

}}

#endif
