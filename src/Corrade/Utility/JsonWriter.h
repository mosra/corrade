#ifndef Corrade_Utility_JsonWriter_h
#define Corrade_Utility_JsonWriter_h
/*
    This file is part of Corrade.

    Copyright © 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016,
                2017, 2018, 2019, 2020, 2021, 2022
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
 * @brief Class @ref Corrade::Utility::JsonWriter
 * @m_since_latest
 */

#include <cstdint>

#include "Corrade/Containers/Pointer.h"
#include "Corrade/Containers/EnumSet.h"
#include "Corrade/Utility/visibility.h"

namespace Corrade { namespace Utility {

/**
@brief JSON writer
@m_since_latest

A counterpart to @ref Json for saving JSON files, including whitespace control.
Supports writing of 32-bit floating-point numbers and 32-bit and 52-/53-bit
unsigned and signed integer types in addition to the general 64-bit
floating-point representation.

To optimize for writing performance and minimal memory usage, the class
provides a write-only stream-like interface, formatting the JSON string on the
fly. It is thus not possible to go for example go back and add values to
existing objects or arrays --- if that's desired, users are encouraged to build
an intermediate mutable storage first and only then feed it to this class.

@section Utility-JsonWriter-usage Usage

The following writes a very minimal glTF file, pretty-printed with two-space
indentation. Objects are created with matching @ref beginObject() and
@ref endObject() calls, @ref writeKey() adds an object key and @ref write() a
value. Similarly, arrays are delimited with @ref beginArray() and
@ref endArray(). For convenience the calls can be chained after each other.

@m_div{m-row}
@m_div{m-col-m-12}
@snippet Utility.cpp JsonWriter-usage1
@m_enddiv
@m_enddiv

@m_class{m-row}

@parblock

@m_div{m-col-m-8}
@snippet Utility.cpp JsonWriter-usage2
@m_enddiv

@m_div{m-col-m-4}
@code{.json}
{
  "asset": {
    "version": "2.0"
  },
  "nodes": [
    {
      "name": "Fox",
      "mesh": 5
    }
  ]
}
@endcode
@m_enddiv

@endparblock

@m_div{m-row}
@m_div{m-col-m-12}
@snippet Utility.cpp JsonWriter-usage3
@m_enddiv
@m_enddiv

To avoid errors, each call checks that it's indeed made when given token is
expected. The final @ref toFile() or @ref toString() can only be called once
all objects and arrays are completed. While a JSON commonly has a top-level
object or array, a single top-level literal, number or string is allowed as
well. There has to be exactly one top-level value, empty files are not allowed.

@subsection Utility-JsonWriter-usage-object-array-scope Array and object scopes

The @ref beginObjectScope() and @ref beginArrayScope() functions return a
@ref Containers::ScopeGuard object that will automatically perform a matching
call to @ref endObject() or @ref endArray() at the end of scope, which may be
useful when writing deeply nested hierarchies:

@snippet Utility.cpp JsonWriter-usage-object-array-scope
*/
class CORRADE_UTILITY_EXPORT JsonWriter {
    public:
        /**
         * @brief Pretty-printing option
         *
         * @see @ref Options, @ref JsonWriter(Options, std::uint32_t)
         */
        enum class Option {
            /**
             * Wrap object and array contents. Turns
             *
             * @code{.json}
             * [[1,2,null],"hello",{"key":"value","another":true}]
             * @endcode
             *
             * @m_class{m-noindent}
             *
             * into the following, including also a final newline at document
             * end:
             *
             * @code{.json}
             * [
             *   [
             *     1,
             *     2,
             *     null
             *   ],
             *   "hello",
             *   {
             *      "key":"value",
             *      "another":true
             *   }
             * ]
             * @endcode
             *
             * Indentation before object keys and array values is controlled
             * with the @p indentation parameter passed to the
             * @ref JsonWriter(Options, std::uint32_t) constructor. Nested
             * object and array values use one indentation level more for their
             * contents. Use @ref Option::TypographicalSpace to add a space
             * after the `:` in object keys.
             */
            Wrap = 1 << 0,

            /**
             * Puts a typographical space after `:` in object keys, and also
             * after `,` if @ref Option::Wrap is not used. Turns
             *
             * @code{.json}
             * [[1,2,null],"hello",{"key":"value","another":true}]
             * @endcode
             *
             * @m_class{m-noindent}
             *
             * into the following:
             *
             * @code{.json}
             * [[1, 2, null], "hello", {"key": "value", "another": true}]
             * @endcode
             *
             * No spaces are added before a `:`, before `,` or inside `[]` and
             * `{}` braces.
             */
            TypographicalSpace = 1 << 1
        };

        /**
         * @brief Pretty-printing options
         *
         * @see @ref JsonWriter(Options, std::uint32_t)
         */
        typedef Containers::EnumSet<Option> Options;

        CORRADE_ENUMSET_FRIEND_OPERATORS(Options)

        /**
         * @brief Construct a pretty-printing JSON writer
         * @param options       Pretty-printing options
         * @param indentation   Number of spaces used for each indentation
         *      level. Has no effect if @ref Option::Wrap is not set. Expected
         *      to be at most @cpp 8 @ce.
         */
        explicit JsonWriter(Options options, std::uint32_t indentation);

        /**
         * @brief Construct a compact JSON writer
         *
         * Equivalent to calling @ref JsonWriter(Options, std::uint32_t)
         * with an empty @ref Options and @cpp 0 @ce for @p indentation.
         */
        explicit JsonWriter(): JsonWriter{{}, 0} {}

        /** @brief Copying is not allowed */
        JsonWriter(const JsonWriter&) = delete;

        /** @brief Move constructor */
        JsonWriter(JsonWriter&&) noexcept;

        /**
         * @brief Destructor
         *
         * Compared to @ref toString() or @ref toFile(), it isn't an error if
         * a writer instance with an incomplete JSON gets destructed.
         */
        ~JsonWriter();

        /** @brief Copying is not allowed */
        JsonWriter& operator=(const JsonWriter&) = delete;

        /** @brief Move assignment */
        JsonWriter& operator=(JsonWriter&&) noexcept;

        /**
         * @brief Begin an object
         * @return Reference to self (for method chaining)
         *
         * Writes `{` to the output, separated by `,` if there's another value
         * before, with spacing and indentation as appropriate. Expected to not
         * be called after the top-level JSON value was closed and not when an
         * object key is expected.
         * @see @ref endObject(), @ref writeKey(), @ref beginObjectScope()
         */
        JsonWriter& beginObject();

        /**
         * @brief End an object
         * @return Reference to self (for method chaining)
         *
         * Writes `}` to the output, with spacing and indentation as
         * appropriate. Expected to be called only if @ref beginObject() was
         * called before with no unclosed array in the meantime and not when an
         * object value is expected.
         */
        JsonWriter& endObject();

        /**
         * @brief Begin an array
         * @return Reference to self (for method chaining)
         *
         * Writes `[` to the output, separated by `,` if there's another value
         * before, with spacing and indentation as appropriate. Expected to not
         * be called after the top-level JSON value was closed and not when an
         * object key is expected.
         * @see @ref endArray(), @ref beginArrayScope()
         */
        JsonWriter& beginArray();

        /**
         * @brief End an array
         * @return Reference to self (for method chaining)
         *
         * Writes `]` to the output, with spacing and indentation as
         * appropriate. Expected to be called only if @ref beginArray() was
         * called before with no unclosed object in the meantime.
         */
        JsonWriter& endArray();

        /**
         * @brief Begin an object scope
         *
         * Calls @ref beginObject() and returns a scope guard instance that
         * calls @ref endObject() at the end of the scope. See
         * @ref Utility-JsonWriter-usage-object-array-scope for an example.
         */
        Containers::ScopeGuard beginObjectScope();

        /**
         * @brief Begin an object scope
         *
         * Calls @ref beginArray() and returns a scope guard instance that
         * calls @ref endArray() at the end of the scope. See
         * @ref Utility-JsonWriter-usage-object-array-scope for an example.
         */
        Containers::ScopeGuard beginArrayScope();

        /**
         * @brief Write an object key
         * @return Reference to self (for method chaining)
         *
         * Writes the key as a JSON string literal to the output, separated by
         * `,` if there's another value before, followed by a `:`, with spacing
         * and indentation as appropriate. Expected to be called only inside an
         * object scope either at the beginning or after a value for the
         * previous key was written. Escaping behavior is the same as with
         * @ref write(Containers::StringView).
         */
        JsonWriter& writeKey(Containers::StringView key);

        /**
         * @brief Write a null value
         * @return Reference to self (for method chaining)
         *
         * Writes @cb{.json} null @ce to the output, separated by `,` if
         * there's another value before, with spacing and indentation as
         * appropriate. Expected to not be called after the top-level JSON
         * value was closed and not when an object key is expected.
         */
        JsonWriter& write(std::nullptr_t);

        /**
         * @brief Write a bool value
         * @return Reference to self (for method chaining)
         *
         * Writes @cb{.json} false @ce or @cb{.json} true @ce to the output,
         * separated by `,` if there's another value before, with spacing and
         * indentation as appropriate. Expected to not be called after the
         * top-level JSON value was closed and not when an object key is
         * expected.
         */
        #ifdef DOXYGEN_GENERATING_OUTPUT
        JsonWriter& writeValue(bool value);
        #else
        template<class T> typename std::enable_if<std::is_same<T, bool>::value, JsonWriter&>::type write(T value) {
            return writeBoolInternal(value);
        }
        #endif

        /**
         * @brief Write a 32-bit floating-point value
         * @return Reference to self (for method chaining)
         *
         * Writes the value as a JSON number literal to the output, separated
         * by `,` if there's another value before, with spacing and indentation
         * as appropriate. Expected to not be called after the top-level JSON
         * value was closed and not when an object key is expected. The value
         * is expected to not be a NaN or an infinity and is printed with 6
         * significant digits, consistently with @ref Debug or @ref format().
         * If you need a larger precision, use @ref write(double).
         */
        JsonWriter& write(float value);

        /**
         * @brief Write a 64-bit floating-point value
         * @return Reference to self (for method chaining)
         *
         * Writes the value as a JSON number literal to the output, separated
         * by `,` if there's another value before, with spacing and indentation
         * as appropriate. Expected to not be called after the top-level JSON
         * value was closed and not when an object key is expected. The value
         * is expected to not be a NaN or an infinity and is printed with 15
         * significant digits, consistently with @ref Debug or @ref format().
         */
        JsonWriter& write(double value);

        /**
         * @brief Write an unsigned 32-bit integer value
         * @return Reference to self (for method chaining)
         *
         * Writes the value as a JSON number literal to the output, separated
         * by `,` if there's another value before, with spacing and indentation
         * as appropriate. Expected to not be called after the top-level JSON
         * value was closed and not when an object key is expected.
         */
        JsonWriter& write(std::uint32_t value);

        /**
         * @brief Write a signed 32-bit integer value
         * @return Reference to self (for method chaining)
         *
         * Writes the value as a JSON number literal to the output, separated
         * by `,` if there's another value before, with spacing and indentation
         * as appropriate. Expected to not be called after the top-level JSON
         * value was closed and not when an object key is expected.
         */
        JsonWriter& write(std::int32_t value);

        /**
         * @brief Write an unsigned 52-bit integer value
         * @return Reference to self (for method chaining)
         *
         * Writes the value as a JSON number literal to the output, separated
         * by `,` if there's another value before, with spacing and indentation
         * as appropriate. Expected to not be called after the top-level JSON
         * value was closed and not when an object key is expected. The value
         * is expected to fit into 52 bits, which is the representable unsigned
         * integer range in a JSON.
         */
        #ifdef DOXYGEN_GENERATING_OUTPUT
        JsonWriter& write(std::uint64_t value);
        #else
        JsonWriter& write(unsigned long long value);
        JsonWriter& write(unsigned long value) {
            /* Hey, C and C++, your types *and* your typedefs are stupid! */
            return write(static_cast<typename std::conditional<sizeof(unsigned long) == 8, unsigned long long, unsigned int>::type>(value));
        }
        #endif

        /**
         * @brief Write a signed 53-bit integer value
         * @return Reference to self (for method chaining)
         *
         * Writes the value as a JSON number literal to the output, separated
         * by `,` if there's another value before, with spacing and indentation
         * as appropriate. Expected to not be called after the top-level JSON
         * value was closed and not when an object key is expected. The value
         * is expected to fit into 52 bits, excluding the sign, which is the
         * representable signed integer range in a JSON.
         */
        #ifdef DOXYGEN_GENERATING_OUTPUT
        JsonWriter& write(std::int64_t value);
        #else
        JsonWriter& write(long long value);
        JsonWriter& write(long value) {
            /* Hey, C and C++, your types *and* your typedefs are stupid! */
            return write(static_cast<typename std::conditional<sizeof(long) == 8, long long, int>::type>(value));
        }
        #endif

        /**
         * @brief Write a string value
         * @return Reference to self (for method chaining)
         *
         * Writes the value as a JSON string literal to the output, separated
         * by `,` if there's another value before, with spacing and indentation
         * as appropriate. Expected to not be called after the top-level JSON
         * value was closed and not when an object key is expected --- use
         * @ref writeKey() in that case instead. The string is expected to be
         * in UTF-8 but its validity isn't checked. Only the `"`, `\`, `/`,
         * bell (@cpp '\b' @ce), form feed (@cpp '\f' @ce), newline
         * (@cpp '\n' @ce), tab (@cpp '\t' @ce) and carriage return
         * (@cpp '\r' @ce) values are escaped, UTF-8 bytes are written verbatim
         * without escaping.
         */
        JsonWriter& write(Containers::StringView value);

        /**
         * @brief Write a raw JSON string
         * @return Reference to self (for method chaining)
         *
         * The string is expected to be non-empty and a valid and closed JSON
         * value, i.e., a null, bool numeric or a string literal, a complete
         * object or a complete array, but its validity isn't checked.
         * Internally it's treated as writing a single value, separated by `,`
         * if there's another value before, with outside spacing and
         * indentation as appropriate, but no spacing or indentation performed
         * inside the string.
         */
        JsonWriter& writeJson(Containers::StringView json);

        /**
         * @brief Get the result as a string
         *
         * Expected to be called only once a complete top-level JSON value is
         * written. The returned view has
         * @ref Containers::StringViewFlag::NullTerminated set, points to data
         * owned by the @ref JsonWriter instance and is valid until the end of
         * its lifetime.
         * @see @ref toFile()
         */
        Containers::StringView toString() const;

        /**
         * @brief Save the result into a file
         *
         * Expected to be called only once a complete top-level JSON value is
         * written. Returns @cpp false @ce if the file can't be written.
         * @see @ref toString()
         */
        bool toFile(Containers::StringView filename) const;

    private:
        struct State;

        /* Because unguarded writeValue(bool) would get a preference over
           writeValue(Containers::StringView) for const char*, sigh */
        JsonWriter& writeBoolInternal(bool value);

        /* Writes a comma if not at the start or at when object value is
           expected and indents if not when object value is expected */
        CORRADE_UTILITY_LOCAL void writeCommaNewlineIndentInternal();
        CORRADE_UTILITY_LOCAL void finalizeDocument();

        /* Writes a string, without and comma, newline or indent. Used by
           writeString() and writeObjectKey(). */
        CORRADE_UTILITY_LOCAL void writeStringLiteralInternal(Containers::StringView string);

        /* Writes a raw piece of JSON, including a potential comma before and
           indentation. Used by all writeValue() APIs except strings and
           arrays. */
        CORRADE_UTILITY_LOCAL JsonWriter& writeInternal(Containers::StringView literal);

        Containers::Pointer<State> _state;
};

}}

#endif
