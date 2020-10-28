#ifndef Corrade_Containers_StringView_h
#define Corrade_Containers_StringView_h
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
allocations.

@section Containers-BasicStringView-usage Usage

The class is meant to be used through either the @ref StringView or
@ref MutableStringView typedefs. It's implicitly convertible from C string
literals, but the recommended way is using the @link operator""_s() @endlink
literal:

@snippet Containers.cpp StringView-literal

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

@snippet Containers.cpp StringView-literal-null

C string literals are implicitly immutable, in order to create a mutable one
you need to assign the literal to a @cpp char[] @ce (instead of
@cpp const char* @ce) and then create a @ref MutableStringView in a second
step. For example:

@snippet Containers.cpp StringView-mutable

This class is implicitly convertible from and to @ref ArrayView, however note
that the conversion will not preserve the global / null-terminated annotations.

@attention In order to allow the above-mentioned optimizations, on 32-bit
    systems the size is limited to 1 GB. That should be more than enough for
    real-world strings (as opposed to arbitrary binary data), if you need more
    please use an @ref ArrayView instead.

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
         * @ref BasicStringView(T*)
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
         * set, it's assumed that @cpp data[size] == '\0' @ce. That can avoid
         * copies and allocations in code that passes such string to APIs that
         * expect null-terminated strings (such as @ref std::fopen()).
         *
         * If you're unsure about data origin, the safe bet is to keep flags at
         * their default. On the other hand, C string literals are always
         * global and null-terminated --- for those, the recommended way is to
         * use the @link operator""_s() @endlink literal instead.
         */
        constexpr /*implicit*/ BasicStringView(T* data, std::size_t size, StringViewFlags flags = {}) noexcept: _data{data}, _size{
            (CORRADE_CONSTEXPR_ASSERT(size < std::size_t{1} << (sizeof(std::size_t)*8 - 2),
                "Containers::StringView: string expected to be smaller than 2^" << Utility::Debug::nospace << sizeof(std::size_t)*8 - 2 << "bytes, got" << size),
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
         * If @p data is not @cpp nullptr @ce, the resulting instance has
         * @ref StringViewFlag::NullTerminated set.
         *
         * The @ref BasicStringView(std::nullptr_t) overload (which is a
         * default constructor) is additionally @cpp constexpr @ce and has
         * @ref StringViewFlag::Global set instead of
         * @ref StringViewFlag::NullTerminated.
         */
        /*implicit*/ BasicStringView(T* data) noexcept;

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
         * @brief Whether the string begins with given prefix
         *
         * For an empty string returns @cpp true @ce only if @p prefix is empty
         * as well.
         * @see @ref stripPrefix()
         */
        bool hasPrefix(StringView prefix) const;

        /**
         * @brief Whether the string ends with given suffix
         *
         * For an empty string returns @cpp true @ce only if @p suffix is empty
         * as well.
         * @see @ref stripSuffix()
         */
        bool hasSuffix(StringView suffix) const;

        /**
         * @brief Strip given prefix
         *
         * Expects that the string actually begins with given prefix. The
         * function uses @ref slice() internally, meaning it propagates the
         * @ref flags() as appropriate. Additionally, the resulting view is
         * @cpp nullptr @ce only if the input is @cpp nullptr @ce, otherwise
         * the view always points to existing memory.
         * @see @ref hasPrefix()
         */
        BasicStringView<T> stripPrefix(StringView prefix) const;

        /**
         * @brief Strip given suffix
         *
         * Expects that the string actually ends with given suffix. The
         * function uses @ref slice() internally, meaning it propagates the
         * @ref flags() as appropriate. Additionally, the resulting view is
         * @cpp nullptr @ce only if the input is @cpp nullptr @ce, otherwise
         * the view always points to existing memory.
         * @see @ref hasSuffix()
         */
        BasicStringView<T> stripSuffix(StringView suffix) const;

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
