#ifndef Corrade_Containers_String_h
#define Corrade_Containers_String_h
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
 * @brief Class @ref Corrade::Containers::String, tag type @ref Corrade::Containers::AllocatedInitT, tag @ref Corrade::Containers::AllocatedInit
 * @m_since_latest
 */

#include <cstddef>
#include <cstdint>
#include <type_traits>

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
@see @ref AllocatedInit, @ref Containers-String-usage-sso
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
@ref Containers-String-usage-sso
*/
constexpr AllocatedInitT AllocatedInit{AllocatedInitT::Init{}};

/**
@brief String
@m_since_latest

A lightweight non-templated alternative to @ref std::string with support for
custom deleters. A non-owning version of this container is a
@ref BasicStringView "StringView".

@section Containers-String-usage Usage

It's recommended to prefer using @ref BasicStringView "StringView" in most
cases, and only create a @ref String instance if you need to extend lifetime of
the data or mutate it. The @ref String is implicitly convertible from C string
literals, but the designated way to instantiate a string is using the
@link operator""_s() @endlink literal. While both expressions are *mostly*
equivalent, the implicit conversion has some runtime impact due to
@ref std::strlen(), and it won't preserve zero bytes inside the string:

@snippet Containers.cpp String-usage-literal-null

@ref String instances are implicitly convertible from and to
(mutable) @ref BasicStringView "StringView", all instances (including an empty
string) are guaranteed to be null-terminated, which means a conversion to
@ref BasicStringView "StringView" will always have
@ref StringViewFlag::NullTerminated set.

As with @ref BasicStringView "StringView", the class is implicitly convertible
to @ref ArrayView. In addition it's also move-convertible to @ref Array, transferring the ownership of the internal data array to it. Ownership transfer
in the other direction is not provided because it's not possible to implicitly
guarantee null termination of the input @ref Array. In that case use the
explicit @ref String(char*, std::size_t, Deleter) constructor together with
@ref Array::release() and @ref Array::deleter(). See also
@ref Containers-String-usage-wrapping below.

@subsection Containers-String-usage-sso Small string optimization

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

@subsection Containers-String-usage-initialization String initialization

In addition to creating a @ref String from an existing string (literal) or
wrapping an externally allocated memory as mentioned above, explicit
initialization constructors are provided, similarly to the @ref Array class:

-   @ref String(ValueInitT, std::size_t) zero-initializes the string, meaning
    each of its characters is @cpp '\0' @ce. For heap-allocated strings this is
    equivalent to @cpp new char[size + 1]{} @ce (the one extra character is
    for the null terminator).
-   @ref String(DirectInitT, std::size_t, char) fills the whole string with
    given character and zero-initializes the null terminator. For
    heap-allocated strings this is equivalent to
    @cpp new char[size + 1]{c, c, c, …, '\0'} @ce.
-   @ref String(NoInitT, std::size_t) keeps the contents uninitialized, except
    for the null terminator. Equivalent to @cpp new char[size + 1] @ce followed
    by @cpp string[size] = '\0' @ce.

@subsection Containers-String-usage-wrapping Wrapping externally allocated strings

Similarly to an @ref Array, by default the class makes all allocations using
@cpp operator new[] @ce and deallocates using @cpp operator delete[] @ce. It's
however also possible to wrap an externally allocated string using
@ref String(char*, std::size_t, Deleter) together with specifying which
function to use for deallocation.

For example, properly deallocating a string allocated using @ref std::malloc():

@snippet Containers.cpp String-usage-wrapping

@section Containers-String-stl STL compatibility

Instances of @ref String are *implicitly* convertible from and to
@ref std::string if you include @ref Corrade/Containers/StringStl.h. The
conversion is provided in a separate header to avoid unconditional @cpp
#include <string> @ce, which significantly affects compile times. The following
table lists allowed conversions:

Corrade type                    | ↭ | STL type
------------------------------- | - | ---------------------
@ref String                     | ⇆ | @ref std::string (data copy)

Example:

@snippet Containers-stl.cpp String

Because @ref std::string doesn't provide any way to transfer ownership of its
underlying memory, conversion either way always involves a data copy. To
mitigate the conversion impact, it's recommended to convert @ref std::string
instances to @ref BasicStringView "StringView" instead where possible.

On compilers that support C++17 and @ref std::string_view, *implicit*
conversion from and to it is provided in @ref Corrade/Containers/StringStlView.h.
For similar reasons, it's a dedicated header to avoid unconditional
@cpp #include <string_view> @ce, but this one is even significantly heavier
than the @ref string "<string>" include on certain implementations, so it's
separate from a @ref std::string as well. The following table lists allowed
conversions:

Corrade type                    | ↭ | STL type
------------------------------- | - | ---------------------
@ref String                     | ← | @ref std::string_view (data copy)
@ref String                     | → | @ref std::string_view

The @ref std::string_view type doesn't have any mutable counterpart, so there's
no differentiation for a @cpp const @ce variant. While creating a
@ref std::string_view from a @ref String creates a non-owning reference without
allocations or copies, converting the other way involves a data copy. To
mitigate the conversion impact, it's recommended to convert
@ref std::string_view instances to @ref BasicStringView "StringView" instead
where possible.

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
         * @see @ref Containers-String-usage-sso,
         *      @ref String(AllocatedInitT, StringView)
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
         * @see @ref Containers-String-usage-sso,
         *      @ref String(AllocatedInitT, const char*)
         */
        /*implicit*/ String(const char* data);

        /**
         * @brief Construct from a sized C string
         *
         * Creates a null-terminated owning copy of @p data. Contrary to the
         * behavior of @ref std::string, @p data is allowed to be
         * @cpp nullptr @ce, but only if @p size is zero. Depending on the
         * size, it's either stored allocated or in a SSO.
         * @see @ref Containers-String-usage-sso,
         *      @ref String(AllocatedInitT, const char*, std::size_t)
         */
        /*implicit*/ String(const char* data, std::size_t size);

        /**
         * @brief Construct from a string view, bypassing SSO
         *
         * Compared to @ref String(StringView) the data is always allocated.
         * @see @ref Containers-String-usage-sso
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
         * @see @ref Containers-String-usage-sso
         */
        explicit String(AllocatedInitT, const char* data);

        /**
         * @brief Construct from a sized C string
         *
         * Compared to @ref String(const char*, std::size_t) the data is always
         * allocated.
         * @see @ref Containers-String-usage-sso
         */
        explicit String(AllocatedInitT, const char* data, std::size_t size);

        /**
         * @brief Take ownership of an external data array
         * @param data      String. Can't be @cpp nullptr @ce.
         * @param size      Size of the string, excluding the null terminator
         * @param deleter   Deleter. Use @cpp nullptr @ce for the standard
         *      @cpp delete[] @ce.
         *
         * Since the @ref String class provides a guarantee of null-terminated
         * strings, the @p data array is expected to be null-terminated (which
         * implies @p data *can't* be @cpp nullptr @ce), but the null
         * terminator not being included in @p size. For consistency and
         * interoperability with @ref Array (i.e., an empty string turning to a
         * zero-sized array) this in turn means the size passed to @p deleter
         * is one byte less than the actual memory size, and if the deleter
         * does sized deallocation, it has to account for that.
         *
         * The @p deleter will be *unconditionally* called on destruction with
         * @p data and @p size as an argument. In particular, it will be also
         * called if @p size is @cpp 0 @ce (@p data isn't allowed to be
         * @cpp nullptr @ce).
         *
         * In case of a moved-out instance, the deleter gets reset to a
         * default-constructed value alongside the array pointer and size. It
         * effectively means @cpp delete[] nullptr @ce gets called when
         * destructing a moved-out instance (which is a no-op).
         * @see @ref Containers-String-usage-wrapping
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
        explicit String(std::nullptr_t, std::size_t size, Deleter deleter) = delete;

        /**
         * @brief Create a zero-initialized string of given size
         * @param size      Size excluding the null terminator
         *
         * A @ref DefaultInitT overload isn't provided to prevent accidents ---
         * its behavior would be the same to @ref String(NoInitT, std::size_t).
         * @see @ref String(DirectInitT, std::size_t, char)
         */
        explicit String(Corrade::ValueInitT, std::size_t size);

        /**
         * @brief Create a string initialized to a particular character
         * @param size      Size excluding the null terminator
         * @param c         Character value
         *
         * @see @ref String(ValueInitT, std::size_t),
         *      @ref String(NoInitT, std::size_t)
         */
        explicit String(Corrade::DirectInitT, std::size_t size, char c);

        /**
         * @brief Create an uninitialized string
         * @param size      Size excluding the null terminator
         *
         * While the string contents are left untouched, the null terminator
         * *does* get initialized to @cpp '\0' @ce. Useful if you're going to
         * overwrite the contents anyway.
         */
        explicit String(Corrade::NoInitT, std::size_t size);

        /** @todo combined AllocatedInit + Value/Direct/NoInit constructors */

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
        template<class T, class = decltype(Implementation::StringConverter<typename std::decay<T&&>::type>::from(std::declval<T&&>()))> /*implicit*/ String(T&& other) noexcept: String{Implementation::StringConverter<typename std::decay<T&&>::type>::from(Utility::forward<T>(other))} {}

        /**
         * @brief Destructor
         *
         * Calls @ref deleter() on the owned @ref data(); in case of a SSO does
         * nothing.
         * @see @ref Containers-String-usage-sso, @ref isSmall()
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
         * the null terminator is not counted into it. Note that with custom
         * deleters the returned view is not guaranteed to be actually mutable.
         */
        /*implicit*/ operator ArrayView<char>() noexcept;
        /*implicit*/ operator ArrayView<void>() noexcept; /**< @overload */

        /**
         * @brief Move-convert to an @ref Array
         *
         * The data and the corresponding @ref deleter() is transferred to the
         * returned array. In case of a SSO, a copy of the string is allocated
         * including the null terminator and a default deleter is used. The
         * string then resets data pointer, size and deleter to be equivalent
         * to a default-constructed instance. Note that with custom deleters
         * the returned array is not guaranteed to be actually mutable.
         * @see @ref release(), @ref isSmall()
         */
        /*implicit*/ operator Array<char>() &&;

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
         * instance. See @ref Containers-String-usage-sso for more information.
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
         * stored using SSO --- see @ref Containers-String-usage-sso for more
         * information.
         * @see @ref String(char*, std::size_t, Deleter), @ref isSmall()
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
        Array<MutableStringView> splitOnAnyWithoutEmptyParts(StringView delimiters) &;
        Array<StringView> splitOnAnyWithoutEmptyParts(StringView delimiters) const &; /**< @overload */

        #ifdef CORRADE_BUILD_DEPRECATED
        /** @brief @copybrief splitOnAnyWithoutEmptyParts()
         * @m_deprecated_since_latest Use @ref splitOnAnyWithoutEmptyParts()
         *      instead.
         */
        CORRADE_DEPRECATED("use splitOnAnyWithoutEmptyParts() instead") Array<MutableStringView> splitWithoutEmptyParts(StringView delimiters) &;

        /** @overload
         * @m_deprecated_since_latest Use @ref splitOnAnyWithoutEmptyParts()
         *      instead.
         */
        CORRADE_DEPRECATED("use splitOnAnyWithoutEmptyParts() instead") Array<StringView> splitWithoutEmptyParts(StringView delimiters) const &;
        #endif

        /**
         * @brief Split on whitespace, removing empty parts
         *
         * Equivalent to @ref BasicStringView::splitWithoutEmptyParts() const.
         * Not allowed to be called on a rvalue since the returned views would
         * become dangling.
         */
        Array<MutableStringView> splitOnWhitespaceWithoutEmptyParts() &;
        Array<StringView> splitOnWhitespaceWithoutEmptyParts() const &; /**< @overload */

        #ifdef CORRADE_BUILD_DEPRECATED
        /** @brief @copybrief splitOnWhitespaceWithoutEmptyParts()
         * @m_deprecated_since_latest Use @ref splitOnWhitespaceWithoutEmptyParts()
         *      instead.
         */
        CORRADE_DEPRECATED("use splitOnWhitespaceWithoutEmptyParts() instead") Array<MutableStringView> splitWithoutEmptyParts() &;

        /** @overload
         * @m_deprecated_since_latest Use @ref splitOnWhitespaceWithoutEmptyParts()
         *      instead.
         */
        CORRADE_DEPRECATED("use splitOnWhitespaceWithoutEmptyParts() instead") Array<StringView> splitWithoutEmptyParts() const &;
        #endif

        /**
         * @brief Partition
         *
         * Equivalent to @ref BasicStringView::partition(). Not allowed to be
         * called on a rvalue since the returned views would become dangling.
         */
        Array3<MutableStringView> partition(char separator) &;
        Array3<StringView> partition(char separator) const &; /**< @overload */

        /**
         * @brief Join strings with this view as the delimiter
         *
         * Equivalent to @ref BasicStringView::join().
         * @todo a mutable && overload that reuses the growable string storage
         *      instead of allocating new, when growable strings are a thing
         */
        String join(ArrayView<const StringView> strings) const;

        /** @overload */
        String join(std::initializer_list<StringView> strings) const;

        /**
         * @brief Join strings with this view as the delimiter, skipping empty parts
         *
         * Equivalent to @ref BasicStringView::joinWithoutEmptyParts().
         */
        String joinWithoutEmptyParts(ArrayView<const StringView> strings) const;

        /** @overload */
        String joinWithoutEmptyParts(std::initializer_list<StringView> strings) const;

        /**
         * @brief Whether the string begins with given prefix
         *
         * Equivalent to @ref BasicStringView::hasPrefix().
         * @see @ref exceptPrefix()
         */
        bool hasPrefix(StringView prefix) const;

        /**
         * @brief Whether the string ends with given suffix
         *
         * Equivalent to @ref BasicStringView::hasSuffix().
         * @see @ref exceptSuffix()
         */
        bool hasSuffix(StringView suffix) const;

        /**
         * @brief View with given prefix stripped
         *
         * Equivalent to @ref BasicStringView::exceptPrefix(). Not allowed to
         * be called on a r-value since the returned view would become
         * dangling.
         * @see @ref hasPrefix()
         */
        MutableStringView exceptPrefix(StringView prefix) &;
        StringView exceptPrefix(StringView prefix) const &; /**< @overload */

        #ifdef CORRADE_BUILD_DEPRECATED
        /**
         * @brief @copybrief exceptPrefix()
         * @m_deprecated_since_latest Deprecated due to confusing naming that
         *      could imply the original instance gets modified. Use
         *      @ref exceptPrefix() instead.
         */
        CORRADE_DEPRECATED("use exceptPrefix() instead") MutableStringView stripPrefix(StringView prefix) & {
            return exceptPrefix(prefix);
        }

        /**
         * @brief @copybrief exceptPrefix()
         * @m_deprecated_since_latest Deprecated due to confusing naming that
         *      could imply the original instance gets modified. Use
         *      @ref exceptPrefix() instead.
         */
        CORRADE_DEPRECATED("use exceptPrefix() instead") StringView stripPrefix(StringView prefix) const & {
            return exceptPrefix(prefix);
        }
        #endif

        /**
         * @brief View with given suffix stripped
         *
         * Equivalent to @ref BasicStringView::exceptSuffix(). Not allowed to
         * be called on a r-value since the returned view would become
         * dangling.
         * @see @ref hasSuffix()
         */
        MutableStringView exceptSuffix(StringView suffix) &;
        StringView exceptSuffix(StringView suffix) const &; /**< @overload */

        #ifdef CORRADE_BUILD_DEPRECATED
        /**
         * @brief @copybrief exceptSuffix()
         * @m_deprecated_since_latest Deprecated due to confusing naming that
         *      could imply the original instance gets modified. Use
         *      @ref exceptSuffix() instead.
         */
        CORRADE_DEPRECATED("use exceptSuffix() instead") MutableStringView stripSuffix(StringView suffix) & {
            return exceptSuffix(suffix);
        }

        /**
         * @brief @copybrief exceptSuffix()
         * @m_deprecated_since_latest Deprecated due to confusing naming that
         *      could imply the original instance gets modified. Use
         *      @ref exceptSuffix() instead.
         */
        CORRADE_DEPRECATED("use exceptSuffix() instead") StringView stripSuffix(StringView suffix) const & {
            return exceptSuffix(suffix);
        }
        #endif

        /**
         * @brief View with given characters trimmed from prefix and suffix
         *
         * Equivalent to @ref BasicStringView::trimmed(StringView) const. Not
         * allowed to be called on a r-value since the returned view would
         * become dangling.
         * @see @ref trimmedPrefix(), @ref trimmedSuffix()
         */
        MutableStringView trimmed(StringView characters) &;
        StringView trimmed(StringView characters) const &; /**< @overload */

        /**
         * @brief View with whitespace trimmed from prefix and suffix
         *
         * Equivalent to @ref BasicStringView::trimmed() const. Not allowed to
         * be called on a r-value since the returned view would become
         * dangling.
         * @see @ref trimmedPrefix(), @ref trimmedSuffix()
         */
        MutableStringView trimmed() &;
        StringView trimmed() const &; /**< @overload */

        /**
         * @brief View with given characters trimmed from prefix
         *
         * Equivalent to @ref BasicStringView::trimmedPrefix(StringView) const.
         * Not allowed to be called on a r-value since the returned view would
         * become dangling.
         * @see @ref trimmed(), @ref trimmedSuffix()
         */
        MutableStringView trimmedPrefix(StringView characters) &;
        StringView trimmedPrefix(StringView characters) const &; /**< @overload */

        /**
         * @brief View with whitespace trimmed from prefix
         *
         * Equivalent to @ref BasicStringView::trimmedPrefix() const. Not
         * allowed to be called on a r-value since the returned view would
         * become dangling.
         * @see @ref trimmed(), @ref trimmedSuffix()
         */
        MutableStringView trimmedPrefix() &;
        StringView trimmedPrefix() const &; /**< @overload */

        /**
         * @brief View with given characters trimmed from suffix
         *
         * Equivalent to @ref BasicStringView::trimmedSuffix(StringView) const.
         * Not allowed to be called on a r-value since the returned view would
         * become dangling.
         * @see @ref trimmed(), @ref trimmedPrefix()
         */
        MutableStringView trimmedSuffix(StringView characters) &;
        StringView trimmedSuffix(StringView characters) const &; /**< @overload */

        /**
         * @brief View with whitespace trimmed from suffix
         *
         * Equivalent to @ref BasicStringView::trimmedSuffix() const. Not
         * allowed to be called on a r-value since the returned view would
         * become dangling.
         * @see @ref trimmed(), @ref trimmedPrefix()
         */
        MutableStringView trimmedSuffix() &;
        StringView trimmedSuffix() const &; /**< @overload */

        /**
         * @brief Find a substring
         *
         * Equivalent to @ref BasicStringView::find(StringView) const. Not
         * allowed to be called on a r-value since the returned view would
         * become dangling.
         * @see @ref contains()
         */
        MutableStringView find(StringView substring) &;
        StringView find(StringView substring) const &; /**< @overload */

        /**
         * @brief Find a substring
         *
         * Equivalent to @ref BasicStringView::find(char) const. Not allowed to
         * be called on a r-value since the returned view would become
         * dangling.
         * @see @ref contains()
         */
        MutableStringView find(char character) &;
        StringView find(char character) const &; /**< @overload */

        /**
         * @brief Whether the view contains a substring
         *
         * Equivalent to @ref BasicStringView::contains(StringView) const.
         */
        bool contains(StringView substring) const;

        /**
         * @brief Whether the view contains a character
         *
         * Equivalent to @ref BasicStringView::contains(char) const.
         */
        bool contains(char character) const;

        /**
         * @brief Release data storage
         *
         * Returns the data pointer and resets data pointer, size and deleter
         * to be equivalent to a default-constructed instance. Can be called
         * only if the string is not stored using SSO --- see
         * @ref Containers-String-usage-sso for more information. Deleting the
         * returned array is user responsibility --- note the string might have
         * a custom @ref deleter() and so @cpp delete[] @ce might not be always
         * appropriate. Note also that with custom deleters the returned
         * pointer is not guaranteed to be actually mutable.
         * @see @ref operator Array<char>(), @ref isSmall()
         */
        char* release();

    private:
        CORRADE_UTILITY_LOCAL void construct(Corrade::NoInitT, std::size_t size);
        CORRADE_UTILITY_LOCAL void construct(const char* data, std::size_t size);
        CORRADE_UTILITY_LOCAL void destruct();
        CORRADE_UTILITY_LOCAL Containers::Pair<const char*, std::size_t> dataInternal() const;

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
