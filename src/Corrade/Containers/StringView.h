#ifndef Corrade_Containers_StringView_h
#define Corrade_Containers_StringView_h
/*
    This file is part of Corrade.

    Copyright © 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016,
                2017, 2018, 2019, 2020, 2021
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
 * @brief Class @ref Corrade::Containers::BasicStringView, typedef @ref Corrade::Containers::StringView, @ref Corrade::Containers::MutableStringView, literal @link Corrade::Containers::Literals::operator""_s() @endlink
 * @m_since_latest
 * @experimental
 */

#include <cstddef>
#include <utility>

#include "Corrade/Containers/Containers.h"
#include "Corrade/Containers/EnumSet.h"
#include "Corrade/Utility/Assert.h"
#include "Corrade/Utility/Utility.h"
#include "Corrade/Utility/visibility.h"

namespace Corrade { namespace Containers {

namespace Implementation {
    template<class, class> struct StringViewConverter;
}

/**
@brief String view flag
@m_since_latest
@experimental

@see @ref StringViewFlags, @ref BasicStringView
*/
enum class StringViewFlag: std::size_t {
    /**
     * The referenced string is global, i.e., with an unlimited lifetime.
     * Enabling this flag can avoid needless allocations and copies in cases
     * where the consumer needs to extend lifetime of passed string view.
     */
    Global = std::size_t{1} << (sizeof(std::size_t)*8 - 1),

    /**
     * The referenced string is null-terminated. Enabling this flag  can avoid
     * needless allocations and copies in cases where a string is passed to an
     * API that expects only null-terminated strings.
     */
    NullTerminated = std::size_t{1} << (sizeof(std::size_t)*8 - 2)
};

namespace Implementation {
    enum: std::size_t {
        StringViewSizeMask = std::size_t(StringViewFlag::NullTerminated)|std::size_t(StringViewFlag::Global)
    };
}

/**
@brief String view flags
@m_since_latest
@experimental

@see @ref BasicStringView
*/
typedef EnumSet<StringViewFlag
    #ifndef DOXYGEN_GENERATING_OUTPUT
    , Implementation::StringViewSizeMask
    #endif
> StringViewFlags;

CORRADE_ENUMSET_OPERATORS(StringViewFlags)

/** @debugoperatorclassenum{BasicStringView,StringViewFlag} */
CORRADE_UTILITY_EXPORT Utility::Debug& operator<<(Utility::Debug& debug, StringViewFlag value);

/** @debugoperatorclassenum{BasicStringView,StringViewFlags} */
CORRADE_UTILITY_EXPORT Utility::Debug& operator<<(Utility::Debug& debug, StringViewFlags value);

/**
@brief Base for string views
@m_since_latest

@m_keywords{StringView MutableStringView}

A lighter alternative to C++17 @ref std::string_view that has also a mutable
variant and additional optimizations for reducing unnecessary copies and
allocations. An owning version of this container is a @ref String.

@section Containers-BasicStringView-usage Usage

The class is meant to be used through either the @ref StringView or
@ref MutableStringView typedefs. It's implicitly convertible from C string
literals, but the recommended way is using the @link operator""_s() @endlink
literal:

@snippet Containers.cpp StringView-usage-literal

While both expressions are *mostly* equivalent, the literal is
@cpp constexpr @ce so you can use it in a compile-time context (and on the
other hand, the implicit conversion uses @ref std::strlen() which has some
runtime impact). The main difference is however that the literal will annotate
the view as @ref StringViewFlag::Global "global" and
@ref StringViewFlag::NullTerminated "null-terminated", which can help avoid
copies and allocations when lifetime of the data needs to be extended or when
dealing with APIs that expect null-terminated strings. Additionally, the
literal will also preserve zero bytes inside the string, while implicit
conversion from a C string won't:

@snippet Containers.cpp StringView-usage-literal-null

C string literals are implicitly immutable, in order to create a mutable one
you need to assign the literal to a @cpp char[] @ce (instead of
@cpp const char* @ce) and then create a @ref MutableStringView in a second
step. For example:

@snippet Containers.cpp StringView-usage-mutable

This class is implicitly convertible from and to @ref ArrayView, however note
that the conversion will not preserve the global / null-terminated annotations.

@attention In order to allow the above-mentioned optimizations, on 32-bit
    systems the size is limited to 1 GB. That should be more than enough for
    real-world strings (as opposed to arbitrary binary data), if you need more
    please use an @ref ArrayView instead.

@subsection Containers-BasicStringView-usage-slicing String view slicing

The string view class inherits the slicing APIs of @ref ArrayView ---
@ref slice(), @ref prefix(), @ref suffix() and @ref except() --- and in
addition it provides string-specific utilities. These are are all derived from
the slicing APIs, which means they also return sub-views of the original
string:

<ul>
<li>@ref split() and @ref splitWithoutEmptyParts() split the view on given set
of delimiter characters</li>
<li>@ref join() and @ref joinWithoutEmptyParts() is an inverse of the
above</li>
<li>@ref partition() is similar to @ref split(), but always returning three
elements with a clearly defined behavio, which can make certain code more
robust while reducing the amount of possible error states</li>
<li>@ref trimmed() (and its variants @ref trimmedPrefix() /
@ref trimmedSuffix()), commonly used to remove leading and trailing
whitespace</li>
<li>@ref exceptPrefix() / @ref exceptSuffix() checks that a view starts (or
ends) with given string and then removes it:

@snippet Containers.cpp StringView-usage-slicing
</li>
</ul>

@section Containers-BasicStringView-stl STL compatibility

Instances of @ref StringView and @ref BasicStringView are *implicitly*
convertible from and to @ref std::string if you include
@ref Corrade/Containers/StringStl.h. The conversion is provided in a separate
header to avoid unconditional @cpp #include <string> @ce, which significantly
affects compile times. The following table lists allowed conversions:

Corrade type                    | ↭ | STL type
------------------------------- | - | ---------------------
@ref StringView                 | ⇆ | @ref std::string
@ref StringView                 | ⇆ | @ref std::string "const std::string"
@ref MutableStringView          | ⇆ | @ref std::string
@ref MutableStringView          | → | @ref std::string "const std::string"

Example:

@snippet Containers-stl.cpp StringView

Creating a @ref std::string instance always involves an allocation and a copy,
while going the other way always creates a non-owning reference without
allocations or copies. @ref StringView / @ref MutableStringView created from a
@ref std::string always have @ref StringViewFlag::NullTerminated set, but the
usual conditions regarding views apply --- if the original string is modified,
view pointer, size or the null termination property may not be valid anymore.

@experimental
*/
template<class T> class CORRADE_UTILITY_EXPORT BasicStringView {
    public:
        /**
         * @brief Default constructor
         *
         * A default-constructed instance has @ref StringViewFlag::Global set.
         * @see @ref BasicStringView(T*, StringViewFlags)
         */
        constexpr /*implicit*/ BasicStringView(std::nullptr_t = nullptr) noexcept: _data{}, _size{std::size_t(StringViewFlag::Global)} {}

        /**
         * @brief Construct from a C string of known size
         * @param data      C string
         * @param size      Size of the C string, excluding the null terminator
         * @param flags     Flags describing additional string properties
         *
         * If @ref StringViewFlag::Global is set, the data pointer is assumed
         * to never go out of scope, which can avoid copies and allocations in
         * code using the instance. If @ref StringViewFlag::NullTerminated is
         * set, it's expected that `data` is not @cpp nullptr @ce and
         * @cpp data[size] == '\0' @ce. That can avoid copies and allocations
         * in code that passes such string to APIs that expect null-terminated
         * strings (such as @ref std::fopen()).
         *
         * If you're unsure about data origin, the safe bet is to keep flags at
         * their default. On the other hand, C string literals are always
         * global and null-terminated --- for those, the recommended way is to
         * use the @link operator""_s() @endlink literal instead.
         * @see @ref BasicStringView(T*, StringViewFlags)
         */
        constexpr /*implicit*/ BasicStringView(T* data, std::size_t size, StringViewFlags flags = {}) noexcept: _data{data}, _size{
            (CORRADE_CONSTEXPR_ASSERT(size < std::size_t{1} << (sizeof(std::size_t)*8 - 2),
                "Containers::StringView: string expected to be smaller than 2^" << Utility::Debug::nospace << sizeof(std::size_t)*8 - 2 << "bytes, got" << size),
            CORRADE_CONSTEXPR_ASSERT(data || !(flags & StringViewFlag::NullTerminated),
                "Containers::StringView: can't use StringViewFlag::NullTerminated with null data"),
            size|(std::size_t(flags) & Implementation::StringViewSizeMask))} {}

        /**
         * @brief Construct from a @ref String
         *
         * The resulting view has @ref StringViewFlag::NullTerminated set.
         */
        /*implicit*/ BasicStringView(String& data) noexcept;

        /**
         * @brief Construct from a const @ref String
         *
         * Enabled only if the view is not mutable. The resulting view has
         * @ref StringViewFlag::NullTerminated set.
         */
        template<class U = T, class = typename std::enable_if<std::is_const<U>::value>::type> /*implicit*/ BasicStringView(const String& data) noexcept;

        /**
         * @brief Construct from an @ref ArrayView
         *
         * The resulting view has the same size as @p other, by default no
         * null-termination is assumed.
         */
        /* Not constexpr/inline to avoid header dependency on ArrayView.h */
        /*implicit*/ BasicStringView(ArrayView<T> data, StringViewFlags flags = {}) noexcept;

        /** @brief Construct a @ref StringView from a @ref MutableStringView */
        template<class U, class = typename std::enable_if<std::is_same<const U, T>::value>::type> constexpr /*implicit*/ BasicStringView(BasicStringView<U> mutable_) noexcept: _data{mutable_._data}, _size{mutable_._size} {}

        /**
         * @brief Construct from a null-terminated C string
         *
         * Contrary to the behavior of @ref std::string, @p data is allowed to
         * be @cpp nullptr @ce --- in that case an empty view is constructed.
         *
         * Calls @ref BasicStringView(T*, std::size_t, StringViewFlags) with
         * @p size set to @ref std::strlen() of @p data if @p data is not
         * @cpp nullptr @ce. If @p data is @cpp nullptr @ce, @p size is set to
         * @cpp 0 @ce. In addition to @p extraFlags, if @p data is not
         * @cpp nullptr @ce, @ref StringViewFlag::NullTerminated is set,
         * otherwise @ref StringViewFlag::Global is set.
         *
         * The @ref BasicStringView(std::nullptr_t) overload (which is a
         * default constructor) is additionally @cpp constexpr @ce.
         */
        /*implicit*/ BasicStringView(T* data, StringViewFlags extraFlags = {}) noexcept;

        /**
         * @brief Construct a view on an external type / from an external representation
         *
         * @see @ref Containers-BasicStringView-stl
         */
        /* There's no restriction that would disallow creating StringView from
           e.g. std::string<T>&& because that would break uses like
           `consume(foo());`, where `consume()` expects a view but `foo()`
           returns a std::vector. Besides that, to simplify the implementation,
           there's no const-adding conversion. Instead, the implementer is
           supposed to add an ArrayViewConverter variant for that. */
        template<class U, class = decltype(Implementation::StringViewConverter<T, typename std::decay<U&&>::type>::from(std::declval<U&&>()))> constexpr /*implicit*/ BasicStringView(U&& other) noexcept: BasicStringView{Implementation::StringViewConverter<T, typename std::decay<U&&>::type>::from(std::forward<U>(other))} {}

        /**
         * @brief Convert to an @ref ArrayView
         *
         * The resulting view has the same size as this string @ref size() ---
         * the null terminator, if any, is not counted into it.
         */
        operator ArrayView<T>() const noexcept;
        operator ArrayView<typename std::conditional<std::is_const<T>::value, const void, void>::type>() const noexcept; /**< @overload */

        /** @todo convert mutable to const ArrayView, how to do without having
            to do it via a template (and thus including ArrayView?) */

        /**
         * @brief Convert the view to external representation
         *
         * @see @ref Containers-BasicStringView-stl
         */
        /* To simplify the implementation, there's no const-adding conversion.
           Instead, the implementer is supposed to add an StringViewConverter
           variant for that. */
        template<class U, class = decltype(Implementation::StringViewConverter<T, U>::to(std::declval<BasicStringView<T>>()))> constexpr /*implicit*/ operator U() const {
            return Implementation::StringViewConverter<T, U>::to(*this);
        }

        /** @brief Flags */
        constexpr StringViewFlags flags() const {
            return StringViewFlag(_size & Implementation::StringViewSizeMask);
        }

        /**
         * @brief String data
         *
         * The pointer is not guaranteed to be null-terminated, use
         * @ref flags() and @ref StringViewFlag::NullTerminated to check for
         * the presence of a null terminator.
         */
        constexpr T* data() const { return _data; }

        /**
         * @brief String size
         *
         * Excludes the null terminator.
         * @see @ref isEmpty()
         */
        constexpr std::size_t size() const {
            return _size & ~Implementation::StringViewSizeMask;
        }

        /** @brief Whether the string is empty */
        constexpr bool isEmpty() const {
            return !(_size & ~Implementation::StringViewSizeMask);
        }

        /**
         * @brief Pointer to the first byte
         *
         * @see @ref front()
         */
        constexpr T* begin() const { return _data; }
        constexpr T* cbegin() const { return _data; } /**< @overload */

        /**
         * @brief Pointer to (one item after) the last byte
         *
         * @see @ref back()
         */
        constexpr T* end() const {
            return _data + (_size & ~Implementation::StringViewSizeMask);
        }
        constexpr T* cend() const {
            return _data + (_size & ~Implementation::StringViewSizeMask);
        } /**< @overload */

        /**
         * @brief First byte
         *
         * Expects there is at least one byte.
         * @see @ref begin()
         */
        T& front() const;

        /**
         * @brief Last byte
         *
         * Expects there is at least one byte.
         * @see @ref end()
         */
        T& back() const;

        /** @brief Element access */
        constexpr T& operator[](std::size_t i) const { return _data[i]; }

        /**
         * @brief String slice
         *
         * Both arguments are expected to be in range. Propagates the
         * @ref StringViewFlag::Global flag and if @p end points to (one item
         * after) the end of the original null-terminated string, the result
         * has @ref StringViewFlag::NullTerminated also.
         * @m_keywords{substr()}
         */
        constexpr BasicStringView<T> slice(T* begin, T* end) const;

        /** @overload */
        constexpr BasicStringView<T> slice(std::size_t begin, std::size_t end) const;

        /**
         * @brief String prefix
         *
         * Equivalent to @cpp string.slice(string.begin(), end) @ce. If @p end
         * is @cpp nullptr @ce, returns zero-sized @cpp nullptr @ce view.
         * @see @ref slice(T*, T*) const
         */
        constexpr BasicStringView<T> prefix(T* end) const {
            return end ? slice(_data, end) : BasicStringView<T>{};
        }

        /**
         * @brief String prefix
         *
         * Equivalent to @cpp string.slice(0, end) @ce.
         * @see @ref slice(std::size_t, std::size_t) const
         */
        constexpr BasicStringView<T> prefix(std::size_t end) const {
            return slice(0, end);
        }

        /**
         * @brief String suffix
         *
         * Equivalent to @cpp string.slice(begin, string.end()) @ce. If
         * @p begin is @cpp nullptr @ce and the original view isn't, returns a
         * zero-sized @cpp nullptr @ce view.
         * @see @ref slice(T*, T*) const
         */
        constexpr BasicStringView<T> suffix(T* begin) const {
            return _data && !begin ? BasicStringView<T>{} : slice(begin, _data + (_size & ~Implementation::StringViewSizeMask));
        }

        /**
         * @brief String suffix
         *
         * Equivalent to @cpp string.slice(begin, string.size()) @ce.
         * @see @ref slice(std::size_t, std::size_t) const
         */
        constexpr BasicStringView<T> suffix(std::size_t begin) const {
            return slice(begin, _size & ~Implementation::StringViewSizeMask);
        }

        /**
         * @brief String prefix except the last @p count items
         *
         * Equivalent to @cpp string.slice(0, string.size() - count) @ce.
         * @see @ref slice(std::size_t, std::size_t) const
         */
        constexpr BasicStringView<T> except(std::size_t count) const {
            return slice(0, (_size & ~Implementation::StringViewSizeMask) - count);
        }

        /**
         * @brief Split on given character
         *
         * If @p delimiter is not found, returns a single-item array containing
         * the full input string. If the string is empty, returns an empty
         * array. The function uses @ref slice() internally, meaning it
         * propagates the @ref flags() as appropriate.
         * @see @ref splitWithoutEmptyParts(), @ref partition()
         */
        Array<BasicStringView<T>> split(char delimiter) const;

        /**
         * @brief Split on given character, removing empty parts
         *
         * If @p delimiter is not found, returns a single-item array containing
         * the full input string. If the string is empty or consists just of
         * @p delimiter characters, returns an empty array. The function uses
         * @ref slice() internally, meaning it propagates the @ref flags() as
         * appropriate.
         * @see @ref split(), @ref splitWithoutEmptyParts(StringView) const,
         *      @ref partition()
         */
        Array<BasicStringView<T>> splitWithoutEmptyParts(char delimiter) const;

        /**
         * @brief Split on any character from given set, removing empty parts
         *
         * If no characters from @p delimiters are found, returns a single-item
         * array containing the full input string. If the string is empty or
         * consists just of characters from @p delimiters, returns an empty
         * array. The function uses @ref slice() internally, meaning it
         * propagates the @ref flags() as appropriate.
         * @see @ref split(), @ref splitWithoutEmptyParts() const
         */
        Array<BasicStringView<T>> splitWithoutEmptyParts(StringView delimiters) const;

        /**
         * @brief Split on whitespace, removing empty parts
         *
         * Equivalent to calling @ref splitWithoutEmptyParts(StringView) const
         * with @cpp " \t\f\v\r\n" @ce passed to @p delimiters.
         */
        Array<BasicStringView<T>> splitWithoutEmptyParts() const;

        /**
         * @brief Partition
         *
         * Equivalent to Python's @m_class{m-doc-external} [str.partition()](https://docs.python.org/3/library/stdtypes.html#str.partition).
         * Splits @p string at the first occurence of @p separator. First
         * returned value is the part before the separator, second the
         * separator, third a part after the separator. If the separator is not
         * found, returns the input string followed by two empty strings.
         *
         * The function uses @ref slice() internally, meaning it propagates the
         * @ref flags() as appropriate. Additionally, the resulting views are
         * @cpp nullptr @ce only if the input is @cpp nullptr @ce, otherwise
         * the view always points to existing memory.
         * @see @ref split()
         */
        Array3<BasicStringView<T>> partition(char separator) const;

        /**
         * @brief Join strings with this view as the delimiter
         *
         * Similar in usage to Python's @m_class{m-doc-external} [str.join()](https://docs.python.org/3/library/stdtypes.html#str.join) --- the
         * following produces @cpp "hello, world" @ce:
         *
         * @snippet Containers.cpp StringView-join
         */
        String join(ArrayView<const StringView> strings) const;

        /** @overload */
        String join(std::initializer_list<StringView> strings) const;

        /**
         * @brief Join strings with this view as the delimiter, skipping empty parts
         *
         * Like @ref join(), but empty views in @p strings are skipped instead
         * of causing multiple repeated delimiters in the output.
         */
        String joinWithoutEmptyParts(ArrayView<const StringView> strings) const;

        /** @overload */
        String joinWithoutEmptyParts(std::initializer_list<StringView> strings) const;

        /**
         * @brief Whether the string begins with given prefix
         *
         * For an empty string returns @cpp true @ce only if @p prefix is empty
         * as well.
         * @see @ref exceptPrefix()
         */
        bool hasPrefix(StringView prefix) const;

        /**
         * @brief Whether the string ends with given suffix
         *
         * For an empty string returns @cpp true @ce only if @p suffix is empty
         * as well.
         * @see @ref exceptSuffix()
         */
        bool hasSuffix(StringView suffix) const;

        /**
         * @brief View with given prefix stripped
         *
         * Expects that the string actually begins with given prefix. The
         * function uses @ref slice() internally, meaning it propagates the
         * @ref flags() as appropriate. Additionally, the resulting view is
         * @cpp nullptr @ce only if the input is @cpp nullptr @ce, otherwise
         * the view always points to existing memory.
         * @see @ref hasPrefix()
         */
        BasicStringView<T> exceptPrefix(StringView prefix) const;

        #ifdef CORRADE_BUILD_DEPRECATED
        /**
         * @brief @copybrief exceptPrefix()
         * @m_deprecated_since_latest Deprecated due to confusing naming that
         *      could imply the original instance gets modified. Use
         *      @ref exceptPrefix() instead.
         */
        CORRADE_DEPRECATED("use exceptPrefix() instead") BasicStringView<T> stripPrefix(StringView prefix) const {
            return exceptPrefix(prefix);
        }
        #endif

        /**
         * @brief View with given suffix stripped
         *
         * Expects that the string actually ends with given suffix. The
         * function uses @ref slice() internally, meaning it propagates the
         * @ref flags() as appropriate. Additionally, the resulting view is
         * @cpp nullptr @ce only if the input is @cpp nullptr @ce, otherwise
         * the view always points to existing memory.
         * @see @ref hasSuffix()
         */
        BasicStringView<T> exceptSuffix(StringView suffix) const;

        #ifdef CORRADE_BUILD_DEPRECATED
        /**
         * @brief @copybrief exceptSuffix()
         * @m_deprecated_since_latest Deprecated due to confusing naming that
         *      could imply the original instance gets modified. Use
         *      @ref exceptSuffix() instead.
         */
        CORRADE_DEPRECATED("use exceptSuffix() instead") BasicStringView<T> stripSuffix(StringView suffix) const {
            return exceptSuffix(suffix);
        }
        #endif

        /**
         * @brief View with given characters trimmed from prefix and suffix
         *
         * The function uses @ref slice() internally, meaning it propagates the
         * @ref flags() as appropriate. Additionally, the resulting view is
         * @cpp nullptr @ce only if the input is @cpp nullptr @ce, otherwise
         * the view always points to existing memory.
         * @see @ref trimmed() const, @ref trimmedPrefix(StringView) const,
         *      @ref trimmedSuffix(StringView) const
         */
        BasicStringView<T> trimmed(StringView characters) const;

        /**
         * @brief View with whitespace trimmed from prefix and suffix
         *
         * Equivalent to calling @ref trimmed(StringView) const with
         * @cpp " \t\f\v\r\n" @ce passed to @p characters.
         * @see @ref trimmedPrefix() const, @ref trimmedSuffix() const
         */
        BasicStringView<T> trimmed() const;

        /**
         * @brief View with given characters trimmed from prefix
         *
         * The function uses @ref slice() internally, meaning it propagates the
         * @ref flags() as appropriate. Additionally, the resulting view is
         * @cpp nullptr @ce only if the input is @cpp nullptr @ce, otherwise
         * the view always points to existing memory.
         * @see @ref trimmedPrefix() const, @ref trimmed(StringView) const,
         *      @ref trimmedSuffix(StringView) const
         */
        BasicStringView<T> trimmedPrefix(StringView characters) const;

        /**
         * @brief View with whitespace trimmed from prefix
         *
         * Equivalent to calling @ref trimmedPrefix(StringView) const with
         * @cpp " \t\f\v\r\n" @ce passed to @p characters.
         * @see @ref trimmed() const, @ref trimmedSuffix() const
         */
        BasicStringView<T> trimmedPrefix() const;

        /**
         * @brief View with given characters trimmed from suffix
         *
         * The function uses @ref slice() internally, meaning it propagates the
         * @ref flags() as appropriate. Additionally, the resulting view is
         * @cpp nullptr @ce only if the input is @cpp nullptr @ce, otherwise
         * the view always points to existing memory.
         * @see @ref trimmedSuffix() const, @ref trimmed(StringView) const,
         *      @ref trimmedPrefix(StringView) const
         */
        BasicStringView<T> trimmedSuffix(StringView characters) const;

        /**
         * @brief View with whitespace trimmed from suffix
         *
         * Equivalent to calling @ref trimmedSuffix(StringView) const with
         * @cpp " \t\f\v\r\n" @ce passed to @p characters.
         * @see @ref trimmed() const, @ref trimmedPrefix() const
         */
        BasicStringView<T> trimmedSuffix() const;

        /**
         * @brief Find a substring
         *
         * Returns a view pointing to the first found substring. If not found,
         * an empty @cpp nullptr @ce view is returned. The function uses
         * @ref slice() internally, meaning it propagates the @ref flags() as
         * appropriate.
         *
         * Note that the function operates with a @f$ \mathcal{O}(nm) @f$
         * complexity and as such is meant mainly for one-time searches in
         * non-performance-critical code. For repeated searches or searches of
         * large substrings it's recommended to use the @ref std::search()
         * algorithms, especially @ref std::boyer_moore_searcher and its
         * variants. Those algorithms on the other hand have to perform certain
         * preprocessing of the input and keep extra state and due to that
         * overhead aren't generally suited for one-time searches.
         * @see @ref contains()
         */
        BasicStringView<T> find(StringView substring) const;

        /**
         * @brief Whether the view contains a substring
         *
         * A slightly lighter variant of @ref find() useful when you only want
         * to know if a substring was found or not.
         */
        bool contains(StringView substring) const;

    private:
        /* Needed for mutable/immutable conversion */
        template<class> friend class BasicStringView;
        friend String;

        /* MSVC demands the export macro to be here as well */
        friend CORRADE_UTILITY_EXPORT bool operator==(StringView a, StringView b);
        friend CORRADE_UTILITY_EXPORT bool operator!=(StringView a, StringView b);
        friend CORRADE_UTILITY_EXPORT bool operator<(StringView a, StringView b);
        friend CORRADE_UTILITY_EXPORT bool operator<=(StringView a, StringView b);
        friend CORRADE_UTILITY_EXPORT bool operator>=(StringView a, StringView b);
        friend CORRADE_UTILITY_EXPORT bool operator>(StringView a, StringView b);

        /* Used by slice() to skip unneeded checks in the public constexpr
           constructor */
        constexpr explicit BasicStringView(T* data, std::size_t sizePlusFlags, std::nullptr_t): _data{data}, _size{sizePlusFlags} {}

        T* _data;
        std::size_t _size;
};

/**
@brief String view
@m_since_latest

Immutable, use @ref MutableStringView for mutable access.
*/
typedef BasicStringView<const char> StringView;

/**
@brief Mutable string view
@m_since_latest

@see @ref StringView
*/
typedef BasicStringView<char> MutableStringView;

/**
@brief String view equality comparison
@m_since_latest
*/
CORRADE_UTILITY_EXPORT bool operator==(StringView a, StringView b);

/**
@brief String view non-equality comparison
@m_since_latest
*/
CORRADE_UTILITY_EXPORT bool operator!=(StringView a, StringView b);

/**
@brief String view less-than comparison
@m_since_latest
*/
CORRADE_UTILITY_EXPORT bool operator<(StringView a, StringView b);

/**
@brief String view less-than-or-equal comparison
@m_since_latest
*/
CORRADE_UTILITY_EXPORT bool operator<=(StringView a, StringView b);

/**
@brief String view greater-than-or-equal comparison
@m_since_latest
*/
CORRADE_UTILITY_EXPORT bool operator>=(StringView a, StringView b);

/**
@brief String view greater-than comparison
@m_since_latest
*/
CORRADE_UTILITY_EXPORT bool operator>(StringView a, StringView b);

/* operator<<(Debug&, StringView) implemented directly in Debug */

namespace Literals {

/** @relatesalso Corrade::Containers::BasicStringView
@brief String view literal
@m_since_latest

The returned instance has both @ref StringViewFlag::Global and
@ref StringViewFlag::NullTerminated set. See
@ref Containers-BasicStringView-usage for more information.
@m_keywords{_s s}
*/
constexpr StringView operator"" _s(const char* data, std::size_t size) {
    /* Using plain bit ops instead of EnumSet to speed up debug builds */
    return StringView{data, size, StringViewFlag(std::size_t(StringViewFlag::Global)|std::size_t(StringViewFlag::NullTerminated))};
}

}

template<class T> constexpr BasicStringView<T> BasicStringView<T>::slice(T* const begin, T* const end) const {
    return CORRADE_CONSTEXPR_ASSERT(_data <= begin && begin <= end && end <= _data + (_size & ~Implementation::StringViewSizeMask),
            "Containers::StringView::slice(): slice ["
            << Utility::Debug::nospace << begin - _data
            << Utility::Debug::nospace << ":"
            << Utility::Debug::nospace << end - _data
            << Utility::Debug::nospace << "] out of range for"
            << (_size & ~Implementation::StringViewSizeMask) << "elements"),
        BasicStringView<T>{begin, std::size_t(end - begin)|
            /* Propagate the global flag always */
            (_size & std::size_t(StringViewFlag::Global))|
            /* The null termination flag only if the original is
               null-terminated and end points to the original end */
            ((_size & std::size_t(StringViewFlag::NullTerminated))*(end == _data + (_size & ~Implementation::StringViewSizeMask))),
            nullptr};
}

template<class T> constexpr BasicStringView<T> BasicStringView<T>::slice(const std::size_t begin, const std::size_t end) const {
    return CORRADE_CONSTEXPR_ASSERT(begin <= end && end <= (_size & ~Implementation::StringViewSizeMask),
            "Containers::StringView::slice(): slice ["
            << Utility::Debug::nospace << begin
            << Utility::Debug::nospace << ":"
            << Utility::Debug::nospace << end
            << Utility::Debug::nospace << "] out of range for"
            << (_size & ~Implementation::StringViewSizeMask) << "elements"),
        BasicStringView<T>{_data + begin, (end - begin)|
            /* Propagate the global flag always */
            (_size & std::size_t(StringViewFlag::Global))|
            /* The null termination flag only if the original is
               null-terminated and end points to the original end */
            ((_size & std::size_t(StringViewFlag::NullTerminated))*(end == (_size & ~Implementation::StringViewSizeMask))),
            nullptr};
}

}}

#endif
