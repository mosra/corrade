#ifndef Corrade_Containers_String_h
#define Corrade_Containers_String_h
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
 * @brief Class @ref Corrade::Containers::String
 * @m_since_latest
 */

#include <cstddef>
#include <cstdint>
#include <type_traits>
#include <utility>

#include "Corrade/Containers/Containers.h"
#include "Corrade/Containers/StringView.h" /* needs to be included for
                                              comparison operators */
#include "Corrade/Utility/Utility.h"
#include "Corrade/Utility/visibility.h"

namespace Corrade { namespace Containers {

namespace Implementation {
    template<class> struct StringConverter;

    enum: std::size_t { SmallStringSize = sizeof(std::size_t)*3 - 1 };
}

/**
@brief Allocated initialization tag type
@m_since_latest

Used to distinguish @ref String construction that bypasses small string
optimization.
@see @ref AllocatedInit, @ref Containers-String-sso
*/
/* Explicit constructor to avoid ambiguous calls when using {} */
struct AllocatedInitT {
    #ifndef DOXYGEN_GENERATING_OUTPUT
    struct Init{};
    constexpr explicit AllocatedInitT(Init) {}
    #endif
};

/**
@brief Allocated initialization tag
@m_since_latest

Use for @ref String construction that bypasses small string optimization.
@ref Containers-String-sso
*/
constexpr AllocatedInitT AllocatedInit{AllocatedInitT::Init{}};

/**
@brief String
@m_since_latest

A lightweight non-templated alternative to @ref std::string with support for
custom deleters. Owning counterpart to @ref BasicStringView "StringView".

@section Containers-String-usage Usage

It's recommended to prefer using @ref BasicStringView "StringView" and in most
cases, and only create a @ref String instance if you need to extend lifetime of
the data or mutate it. The @ref String is implicitly convertible from C string
literals, but the designated way to instantiate a string is using the
@link operator""_s() @endlink literal. While both expressions are *mostly*
equivalent, the implicit conversion has some runtime impact due to
@ref std::strlen(), and it won't preserve zero bytes inside the string:

@snippet Containers.cpp String-literal-null

@ref String instances are implicitly convertible from and to
(mutable) @ref BasicStringView "StringView", all instances (including an empty
string) are guaranteed to be null-terminated, which means a conversion to
@ref BasicStringView "StringView" will always have
@ref StringViewFlag::NullTerminated set.

As with @ref BasicStringView "StringView", the class is implicitly convertible
to @ref ArrayView. In addition it's also move-convertible to @ref Array, transferring the ownership of the internal data array to it. Ownership transfer
in the other direction is not provided because it's not possible to implicitly
guarantee null termination of the input @ref Array --- use the explicit
@ref String(char*, std::size_t, Deleter) constructor together with
@ref Array::release() in that case.

@section Containers-String-sso Small string optimization

The class stores data size, data pointer and a deleter pointer, which is 24
bytes on 64-bit platforms (and 12 bytes on 32-bit). To avoid allocations for
small strings, small strings up to 22 bytes on 64-bit (23 including the null
terminator) and up to 10 bytes on 32-bit (11 including the null terminator) are
by default stored inside the class.

Such optimization is completely transparent to the user, the only difference is
that @ref deleter() and @ref release() can't be called on SSO strings, as there
is nothing to delete / release. Presence of SSO on an instance can be queried
using @ref isSmall(). In cases where SSO isn't desired (for example when
storing pointers to string contents stored in a growable array), the string can
be constructed using the @ref AllocatedInit tag (for example with
@ref String(AllocatedInitT, const char*)), which bypasses this optimization and
always allocates.

@attention For consistency with @ref StringView and in order to allow the small
    string optimization, on 32-bit systems the size is limited to 1 GB. That
    should be more than enough for real-world strings (as opposed to arbitrary
    binary data), if you need more please use an @ref Array instead.

@section Containers-String-stl STL compatibility

Instances of @ref String are *implicitly* convertible from and to
@ref std::string if you include @ref Corrade/Containers/StringStl.h. The
conversion is provided in a separate header to avoid unconditional @cpp
#include <string> @ce, which significantly affects compile times. The following
table lists allowed conversions:

Corrade type                    | ↭ | STL type
------------------------------- | - | ---------------------
@ref String                     | ⇆ | @ref std::string

Example:

@snippet Containers-stl.cpp String

Because @ref std::string doesn't provide any way to transfer ownership of its
underlying memory, conversion either way always involves an allocation and a
copy. To mitigate the conversion impact, it's recommended to convert
@ref std::string instances to @ref BasicStringView "StringView" instead where
possible.

@experimental
*/
class CORRADE_UTILITY_EXPORT String {
    public:
        typedef void(*Deleter)(char*, std::size_t); /**< @brief Deleter type */

        /**
         * @brief Turn a view into a null-terminated string
         *
         * If the view is @ref StringViewFlag::NullTerminated, returns a
         * non-owning reference to it without any extra allocations or copies
         * involved. Otherwise allocates a null-terminated owning copy using
         * @ref String(StringView).
         *
         * This function is primarily meant for efficiently passing
         * @ref BasicStringView "StringView" instances to APIs that expect
         * null-terminated @cpp const char* @ce. Mutating the result in any way
         * is undefined behavior.
         * @see @ref nullTerminatedGlobalView()
         */
        static String nullTerminatedView(StringView view);

        /**
         * @brief Turn a view into a null-terminated global string
         *
         * If the view is both @ref StringViewFlag::NullTerminated and
         * @ref StringViewFlag::Global, returns a non-owning reference to it
         * without any extra allocations or copies involved. Otherwise
         * allocates a null-terminated owning copy using
         * @ref String(StringView).
         *
         * This function is primarily meant for efficiently storing
         * @ref BasicStringView "StringView" instances, ensuring the
         * memory stays in scope and then passing them to APIs that expect
         * null-terminated @cpp const char* @ce. Mutating the result in any way
         * is undefined behavior.
         * @see @ref nullTerminatedView()
         */
        static String nullTerminatedGlobalView(StringView view);

        /**
         * @brief Default constructor
         *
         * Creates an empty string.
         */
        /*implicit*/ String() noexcept;

        /**
         * @brief Construct from a string view
         *
         * Creates a null-terminated owning copy of @p view. Contrary to the
         * behavior of @ref std::string, @p view is allowed to be
         * @cpp nullptr @ce, but only if it's size is zero. Depending on the
         * size, it's either stored allocated or in a SSO.
         * @see @ref Containers-String-sso, @ref String(AllocatedInitT, StringView)
         */
        /*implicit*/ String(StringView view);
        /*implicit*/ String(Containers::ArrayView<const char> view); /**< @overload */
        /* Without these there's ambiguity between StringView / ArrayView and
           char* */
        /*implicit*/ String(MutableStringView view); /**< @overload */
        /*implicit*/ String(Containers::ArrayView<char> view); /**< @overload */

        /**
         * @brief Construct from a null-terminated C string
         *
         * Creates a null-terminated owning copy of @p data. Contrary to the
         * behavior of @ref std::string, @p data is allowed to be
         * @cpp nullptr @ce --- in that case an empty string is constructed.
         * Depending on the size, it's either stored allocated or in a SSO.
         * @see @ref Containers-String-sso, @ref String(AllocatedInitT, const char*)
         */
        /*implicit*/ String(const char* data);

        /**
         * @brief Construct from a sized C string
         *
         * Creates a null-terminated owning copy of @p data. Contrary to the
         * behavior of @ref std::string, @p data is allowed to be
         * @cpp nullptr @ce, but only if @p size is zero. Depending on the
         * size, it's either stored allocated or in a SSO.
         * @see @ref Containers-String-sso, @ref String(AllocatedInitT, const char*, std::size_t)
         */
        /*implicit*/ String(const char* data, std::size_t size);

        /**
         * @brief Construct from a string view, bypassing SSO
         *
         * Compared to @ref String(StringView) the data is always allocated.
         * @see @ref Containers-String-sso
         */
        explicit String(AllocatedInitT, StringView view);
        explicit String(AllocatedInitT, Containers::ArrayView<const char> view); /**< @overload */
        /* Without these there's ambiguity between StringView / ArrayView and
           char* */
        explicit String(AllocatedInitT, MutableStringView view); /**< @overload */
        explicit String(AllocatedInitT, Containers::ArrayView<char> view); /**< @overload */

        /**
         * @brief Construct from a null-terminated C string, bypassing SSO
         *
         * Compared to @ref String(const char*) the data is always allocated.
         * @see @ref Containers-String-sso
         */
        explicit String(AllocatedInitT, const char* data);

        /**
         * @brief Construct from a sized C string
         *
         * Compared to @ref String(const char*, std::size_t) the data is always
         * allocated.
         * @see @ref Containers-String-sso
         */
        explicit String(AllocatedInitT, const char* data, std::size_t size);

        /**
         * @brief Take ownership of an external data array
         * @param data      String
         * @param size      Size of the string, excluding the null terminator
         * @param deleter   Deleter. Use @cpp nullptr @ce for the standard
         *      @cpp delete[] @ce.
         *
         * Since the @ref String class provides a guarantee of null-terminated
         * strings, the @p data array is expected to be null-terminated (which
         * implies @p data *can't* be @cpp nullptr @ce), but the null
         * terminator not being included in @p size. For consistency and
         * interoperability with @ref Array this in turn means the size passed
         * to @p deleter is one byte less than the actual memory size, and if
         * the deleter does sized deallocation, it has to account for that.
         */
        explicit String(char* data, std::size_t size, Deleter deleter) noexcept;

        /**
         * @brief Take ownership of an immutable external data array
         *
         * Casts away the @cpp const @ce and delegates to
         * @ref String(char*, std::size_t, Deleter). This constructor is
         * provided mainly to allow a @ref String instance to reference global
         * immutable data (such as C string literals) without having to
         * allocate a copy, it's the user responsibility to avoid mutating the
         * data in any way.
         * @see @ref nullTerminatedView(), @ref nullTerminatedGlobalView()
         */
        explicit String(const char* data, std::size_t size, Deleter deleter) noexcept: String{const_cast<char*>(data), size, deleter} {}

        /**
         * @brief Taking ownership of a null pointer is not allowed
         *
         * Since the @ref String class provides a guarantee of null-terminated
         * strings, @p data *can't* be @cpp nullptr @ce.
         */
        explicit String(std::nullptr_t, std::size_t size, Deleter deleter) noexcept = delete;

        /**
         * @brief Construct a view on an external type / from an external representation
         *
         * @see @ref Containers-String-stl
         */
        /* There's no restriction that would disallow creating StringView from
           e.g. std::string<T>&& because that would break uses like
           `consume(foo());`, where `consume()` expects a view but `foo()`
           returns a std::vector. Besides that, to simplify the implementation,
           there's no const-adding conversion. Instead, the implementer is
           supposed to add an ArrayViewConverter variant for that. */
        template<class T, class = decltype(Implementation::StringConverter<typename std::decay<T&&>::type>::from(std::declval<T&&>()))> /*implicit*/ String(T&& other) noexcept: String{Implementation::StringConverter<typename std::decay<T&&>::type>::from(std::forward<T>(other))} {}

        /**
         * @brief Destructor
         *
         * Calls @ref deleter() on the owned @ref data(); in case of a SSO does
         * nothing.
         * @see @ref Containers-String-sso, @ref isSmall()
         */
        ~String();

        /** @brief Copy constructor */
        String(const String& other);

        /** @brief Move constructor */
        String(String&& other) noexcept;

        /** @brief Copy assignment */
        String& operator=(const String& other);

        /** @brief Move assignment */
        String& operator=(String&& other) noexcept;

        /**
         * @brief Convert to a const @ref ArrayView
         *
         * The resulting view has the same size as this string @ref size() ---
         * the null terminator is not counted into it.
         */
        /*implicit*/ operator ArrayView<const char>() const noexcept;
        /*implicit*/ operator ArrayView<const void>() const noexcept; /**< @overload */

        /**
         * @brief Convert to an @ref ArrayView
         *
         * The resulting view has the same size as this string @ref size() ---
         * the null terminator is not counted into it.
         */
        /*implicit*/ operator ArrayView<char>() noexcept;
        /*implicit*/ operator ArrayView<void>() noexcept; /**< @overload */

        /**
         * @brief Convert the view to external representation
         *
         * @see @ref Containers-String-stl
         */
        /* To simplify the implementation, there's no const-adding conversion.
           Instead, the implementer is supposed to add an StringViewConverter
           variant for that. */
        template<class T, class = decltype(Implementation::StringConverter<T>::to(std::declval<String>()))> /*implicit*/ operator T() const {
            return Implementation::StringConverter<T>::to(*this);
        }

        /**
         * @brief Whether the string is stored using small string optimization
         *
         * It's not allowed to call @ref deleter() or @ref release() on a SSO
         * instance. See @ref Containers-String-sso for more information.
         */
        bool isSmall() const { return _small.size & 0x80; }

        /**
         * @brief String data
         *
         * The pointer is always guaranteed to be non-null and the data to be
         * null-terminated, however note that the actual string might contain
         * null bytes earlier than at the end.
         */
        char* data();
        const char* data() const; /**< @overload */

        /**
         * @brief String deleter
         *
         * If set to @cpp nullptr @ce, the contents are deleted using standard
         * @cpp operator delete[] @ce. Can be called only if the string is not
         * stored using SSO --- see @ref Containers-String-sso for more
         * information.
         * @see @ref String(char*, std::size_t, Deleter)
         */
        Deleter deleter() const;

        /** @brief Whether the string is empty */
        bool isEmpty() const;

        /**
         * @brief String size
         *
         * Excludes the null terminator.
         * @see @ref isEmpty()
         */
        std::size_t size() const;

        /**
         * @brief Pointer to the first byte
         *
         * @see @ref front()
         */
        char* begin();
        const char* begin() const; /**< @overload */
        const char* cbegin() const; /**< @overload */

        /**
         * @brief Pointer to (one item after) the last byte
         *
         * @see @ref back()
         */
        char* end();
        const char* end() const; /**< @overload */
        const char* cend() const; /**< @overload */

        /**
         * @brief First byte
         *
         * Expects there is at least one byte.
         * @see @ref begin()
         */
        char& front();
        char front() const; /**< @overload */

        /**
         * @brief Last byte
         *
         * Expects there is at least one byte.
         * @see @ref end()
         */
        char& back();
        char back() const; /**< @overload */

        /** @brief Element access */
        char& operator[](std::size_t i);
        char operator[](std::size_t i) const; /**< @overload */

        /**
         * @brief String slice
         *
         * Equivalent to @ref BasicStringView::slice(). Both arguments are
         * expected to be in range. If @p end points to (one item after) the
         * end of the original (null-terminated) string, the result has
         * @ref StringViewFlag::NullTerminated set.
         * @m_keywords{substr()}
         */
        MutableStringView slice(char* begin, char* end);
        StringView slice(const char* begin, const char* end) const; /**< @overload */
        MutableStringView slice(std::size_t begin, std::size_t end); /**< @overload */
        StringView slice(std::size_t begin, std::size_t end) const; /**< @overload */

        /**
         * @brief String prefix
         *
         * Equivalent to @ref BasicStringView::prefix().
         */
        MutableStringView prefix(char* end);
        StringView prefix(const char* end) const; /**< @overload */
        MutableStringView prefix(std::size_t end); /**< @overload */
        StringView prefix(std::size_t end) const; /**< @overload */

        /**
         * @brief String suffix
         *
         * Equivalent to @ref BasicStringView::suffix().
         */
        MutableStringView suffix(char* begin);
        StringView suffix(const char* begin) const; /**< @overload */
        MutableStringView suffix(std::size_t begin); /**< @overload */
        StringView suffix(std::size_t begin) const; /**< @overload */

        /**
         * @brief String suffix
         *
         * Equivalent to @ref BasicStringView::except().
         */
        MutableStringView except(std::size_t count);
        StringView except(std::size_t count) const; /**< @overload */

        /**
         * @brief Split on given character
         *
         * Equivalent to @ref BasicStringView::split(). Not allowed to be
         * called on a rvalue since the returned views would become dangling.
         */
        Array<MutableStringView> split(char delimiter) &;
        Array<StringView> split(char delimiter) const &; /**< @overload */

        /**
         * @brief Split on given character, removing empty parts
         *
         * Equivalent to @ref BasicStringView::splitWithoutEmptyParts(char) const.
         * Not allowed to be called on a rvalue since the returned views would
         * become dangling.
         */
        Array<MutableStringView> splitWithoutEmptyParts(char delimiter) &;
        Array<StringView> splitWithoutEmptyParts(char delimiter) const &; /**< @overload */

        /**
         * @brief Split on any character from given set, removing empty parts
         *
         * Equivalent to @ref BasicStringView::splitWithoutEmptyParts(StringView) const.
         * Not allowed to be called on a rvalue since the returned views would
         * become dangling.
         */
        Array<MutableStringView> splitWithoutEmptyParts(StringView delimiters) &;
        Array<StringView> splitWithoutEmptyParts(StringView delimiters) const &; /**< @overload */

        /**
         * @brief Split on whitespace, removing empty parts
         *
         * Equivalent to @ref BasicStringView::splitWithoutEmptyParts() const.
         * Not allowed to be called on a rvalue since the returned views would
         * become dangling.
         */
        Array<MutableStringView> splitWithoutEmptyParts() &;
        Array<StringView> splitWithoutEmptyParts() const &; /**< @overload */

        /**
         * @brief Partition
         *
         * Equivalent to @ref BasicStringView::partition(). Not allowed to be
         * called on a rvalue since the returned views would become dangling.
         */
        Array3<MutableStringView> partition(char separator) &;
        Array3<StringView> partition(char separator) const &; /**< @overload */

        /**
         * @brief Whether the string begins with given prefix
         *
         * Equivalent to @ref BasicStringView::hasPrefix().
         * @see @ref stripPrefix()
         */
        bool hasPrefix(StringView prefix) const;

        /**
         * @brief Whether the string ends with given suffix
         *
         * Equivalent to @ref BasicStringView::hasSuffix().
         * @see @ref stripSuffix()
         */
        bool hasSuffix(StringView suffix) const;

        /**
         * @brief Strip given prefix
         *
         * Equivalent to @ref BasicStringView::stripPrefix(). Not allowed to be
         * called on a rvalue since the returned view would become dangling.
         * @see @ref hasPrefix()
         */
        MutableStringView stripPrefix(StringView prefix) &;
        StringView stripPrefix(StringView prefix) const &; /**< @overload */

        /**
         * @brief Strip given prefix
         *
         * Equivalent to @ref BasicStringView::stripSuffix(). Not allowed to be
         * called on a rvalue since the returned view would become dangling.
         * @see @ref hasSuffix()
         */
        MutableStringView stripSuffix(StringView suffix) &;
        StringView stripSuffix(StringView suffix) const &; /**< @overload */

        /**
         * @brief Release data storage
         *
         * Returns the data pointer and resets data pointer, size and deleter
         * to be equivalent to a default-constructed instance. Can be called
         * only if the string is not stored using SSO --- see
         * @ref Containers-String-sso for more information. Deleting the
         * returned array is user responsibility --- note the string might have
         * a custom @ref deleter() and so @cpp delete[] @ce might not be always
         * appropriate.
         */
        char* release();

    private:
        CORRADE_UTILITY_LOCAL void construct(const char* data, std::size_t size);
        CORRADE_UTILITY_LOCAL void destruct();
        CORRADE_UTILITY_LOCAL std::pair<const char*, std::size_t> dataInternal() const;

        /* Small string optimization. Following size restrictions from
           StringView (which uses the top two bits for marking global and
           null-terminated views), we can use the highest bit to denote a small
           string. The second highest bit is currently unused, as it wouldn't
           make sense to allow a String to be larger than StringView, since
           these are two mutually convertible and interchangeable in many
           cases. In case of a large string, there's size, data pointer and
           deleter pointer, either 24 (or 12) bytes in total. In case of a
           small string, we can store the size only in one byte out of 8 (or
           4), which then gives us 23 (or 11) bytes for storing the actual
           data, excluding the null terminator that's at most 22 / 10 ASCII
           chars. With the two topmost bits reserved, it's still 6 bits left
           for the size, which is more than enough in this case.

           On LE the layout is as follows (bits inside a byte are flipped for
           clarity as well):

            +-------------------------------+---------+
            |             string            | si | 01 |
            |              data             | ze |    |
            |             23B/11B           | 6b | 2b |
            +-------------------+-----+++++++---------+
                                | LSB |||||||   MSB   |
            +---------+---------+-----+++++++---------+
            |  data   |  data   |      size      | 00 |
            | pointer | deleter |                |    |
            |  8B/4B  |  8B/4B  |  56b/24b  | 6b | 2b |
            +---------+---------+-----------+---------+

           On BE it's like this:

            +---------+-------------------------------+
            | 10 | si |             string            |
            |    | ze |              data             |
            | 2b | 6b |             23B/11B           |
            +---------+++++++-----+-------------------+
            |   MSB   ||||||| LSB |
            +---------+++++++-----+---------+---------+
            | 00 |     size       |  data   |  data   |
            |    |                | pointer | deleter |
            | 2b | 6b |  56b/24b  |  8B/4B  |  8B/4B  |
            +---------+-----------+---------+---------+

           I originally tried storing the "small string" bit in the lowest bit
           of the deleter pointer, but function pointers apparently can have
           odd addresses on some platforms as well:

            http://lists.llvm.org/pipermail/llvm-dev/2018-March/121953.html

           The above approach is consistent with StringView, which is the
           preferrable solution after all. */
        struct Small {
            #ifdef CORRADE_TARGET_BIG_ENDIAN
            std::uint8_t size;
            char data[Implementation::SmallStringSize];
            #else
            char data[Implementation::SmallStringSize];
            std::uint8_t size;
            #endif
        };
        struct Large {
            #ifdef CORRADE_TARGET_BIG_ENDIAN
            std::size_t size;
            char* data;
            void(*deleter)(char*, std::size_t);
            #else
            char* data;
            void(*deleter)(char*, std::size_t);
            std::size_t size;
            #endif
        };
        union {
            Small _small;
            Large _large;
        };
};

/* operator<<(Debug&, const String&) implemented directly in Debug; comparison
   operators used from StringView (otherwise we would need several overloads
   enumerating all possible combinations including conversion from char arrays
   and external representations to avoid ambiguity, which is not feasible) */

}}

#endif
