#ifndef Corrade_Containers_String_h
#define Corrade_Containers_String_h
/*
    This file is part of Corrade.

    Copyright © 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016,
                2017, 2018, 2019, 2020, 2021, 2022, 2023, 2024, 2025
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
/* std::declval() is said to be in <utility> but libstdc++, libc++ and MSVC STL
   all have it directly in <type_traits> because it just makes sense */
#include <type_traits>

#include "Corrade/Containers/Containers.h"
/* StringView.h needs to be included for comparison operators */
#include "Corrade/Containers/StringView.h"
#include "Corrade/Utility/visibility.h"

namespace Corrade { namespace Containers {

namespace Implementation {
    template<class> struct StringConverter;

    enum: std::size_t {
        SmallStringBit = 0x40,
        SmallStringSize = sizeof(std::size_t)*3 - 1
    };
}

/**
@brief Allocated initialization tag type
@m_since_latest

Used to distinguish @ref String construction that bypasses small string
optimization.
@see @ref AllocatedInit, @ref Containers-String-usage-sso
*/
struct AllocatedInitT {
    #ifndef DOXYGEN_GENERATING_OUTPUT
    struct Init {};
    /* Explicit constructor to avoid ambiguous calls when using {} */
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
@ref StringView and a @ref MutableStringView, implemented using a generic
@ref BasicStringView.

@section Containers-String-usage Usage

It's recommended to prefer using @ref StringView / @ref MutableStringView in
most cases, and only create a @ref String instance if you need to extend
lifetime of the data or perform an operation that can't be done by mutating a
view in-place. The @ref String is implicitly convertible from C string
literals, but the designated way to instantiate a string is using the
@link operator""_s() @endlink literal. While both expressions are *mostly*
equivalent, the implicit conversion has some runtime impact due to
@ref std::strlen(), and it won't preserve zero bytes inside the string:

@snippet Containers.cpp String-usage-literal-null

The @ref String class provides access, slicing and lookup APIs similar to
@ref BasicStringView "StringView", see @ref Containers-BasicStringView-usage "its usage docs"
for details. All @ref String slicing APIs return a (mutable)
@ref BasicStringView "StringView", additionally @ref String instances are
implicitly convertible from and to (mutable) @ref BasicStringView "StringView".
All instances (including an empty string) are guaranteed to be null-terminated,
which means a conversion to @ref BasicStringView "StringView" will always have
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
using @ref isSmall().

@attention For consistency with @ref StringView and in order to allow the small
    string optimization, on 32-bit systems the size is limited to 1 GB. That
    should be more than enough for real-world strings (as opposed to arbitrary
    binary data), if you need more please use an @ref Array instead.

In cases where SSO isn't desired --- for example when strings are stored in a
growable array, are externally referenced via @ref StringView instances or
@cpp char* @ce and pointer stability is required after a reallocation --- the
string can be constructed with @ref String(AllocatedInitT, const char*) and
related APIs using the @ref AllocatedInit tag, which bypasses this optimization
and always allocates. This property is then also preserved on all moves and
copies regardless of the actual string size, i.e., small strings don't suddenly
become SSO instances if the growable array gets reallocated. An
@ref AllocatedInit small string can be turned into an SSO instance again by
explicitly using the @ref String(StringView) constructor:

@snippet Containers.cpp String-usage-sso-copy

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

Similarly to @ref Array, by default the class makes all allocations using
@cpp operator new[] @ce and deallocates using @cpp operator delete[] @ce. It's
however also possible to wrap an externally allocated string using
@ref String(char*, std::size_t, Deleter) together with specifying which
function to use for deallocation.

For example, properly deallocating a string allocated using @ref std::malloc():

@snippet Containers.cpp String-usage-wrapping

@subsection Containers-String-usage-c-string-conversion Converting String instances to null-terminated C strings

If possible when interacting with 3rd party APIs, passing a string together
with the size information is always preferable to passing just a plain
@cpp const char* @ce. Apart from saving an unnecessary @ref std::strlen() call
it can avoid unbounded memory reads in security-critical scenarios.

As said above, a @ref String is guaranteed to always be null-terminated, even
in case it's empty. However, unlike with @ref Array, there's no implicit
conversion to @cpp const char* @ce, because the string can still contain a
@cpp '\0' @ce anywhere in the middle --- thus you have to get the pointer
explicitly using @ref data(). In case your string can contain null bytes, you
should only pass it together with @ref size() or as a range of pointers using
@ref begin() and @ref end() instead, assuming the target API supports such
input.

Extra attention is needed when the originating @ref String instance can move
after the C string pointer got stored somewhere. Pointers to heap-allocated
strings will not get invalidated but @ref Containers-String-usage-sso "SSO strings"
will, leading to nasty crashes when accessing the original pointer. Apart from
ensuring the instances won't get moved, another solution is to force the
strings to be always allocated with @ref String(AllocatedInitT, String&&) and
other variants using the @ref AllocatedInit tag. For example:

@snippet Containers.cpp String-c-string-allocatedinit

See also @ref Containers-BasicStringView-usage-c-string-conversion.

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

Example:

@snippet Containers-stl17.cpp String

The @ref std::string_view type doesn't have any mutable counterpart, so there's
no differentiation for a @cpp const @ce variant. While creating a
@ref std::string_view from a @ref String creates a non-owning reference without
allocations or copies, converting the other way involves a data copy. To
mitigate the conversion impact, it's recommended to convert
@ref std::string_view instances to @ref BasicStringView "StringView" instead
where possible.

Finally, the @ref Corrade/Containers/StringStlHash.h header provides a
@ref std::hash specialization for @ref String, making it usable in
@ref std::unordered_map and @ref std::unordered_set. It's *also* separate, due
to dependency on @cpp #include <functional> @ce which is among the heaviest STL
headers in existence, and which is only really needed when you deal with
unordered containers.

<b></b>

@m_class{m-block m-success}

@par Single-header version
    This class, together with @ref BasicStringView "StringView" /
    @ref MutableStringView, is also available as a single-header
    [CorradeString.hpp](https://github.com/mosra/magnum-singles/tree/master/CorradeString.hpp)
    library in the Magnum Singles repository for easier integration into your
    projects. It depends on [CorradeEnumSet.h](https://github.com/mosra/magnum-singles/tree/master/CorradeEnumSet.h).
    See @ref corrade-singles for more information.
@par
    The library has a separate non-inline implementation part which
    additionally depends on [CorradeCpu.hpp](https://github.com/mosra/magnum-singles/tree/master/CorradeCpu.hpp)
    and [CorradePair.h](https://github.com/mosra/magnum-singles/tree/master/CorradePair.h),
    enable it *just once* like this:
@par
    @code{.cpp}
    #define CORRADE_STRING_IMPLEMENTATION
    #include <CorradeString.hpp>
    @endcode
@par
    If you need the deinlined symbols to be exported from a shared library,
    @cpp #define CORRADE_UTILITY_EXPORT @ce and
    @cpp #define CORRADE_UTILITY_LOCAL @ce as appropriate. Runtime CPU dispatch
    for the implementation done by the @ref Cpu library is enabled by default,
    you can disable it with @cpp #define CORRADE_NO_CPU_RUNTIME_DISPATCH @ce
    before including the file in both the headers and the implementation. To
    enable the @ref Cpu-usage-automatic-cached-dispatch "IFUNC functionality"
    for CPU runtime dispatch, @cpp #define CORRADE_CPU_USE_IFUNC @ce before
    including the file.
@par
    The above mentioned STL compatibility is included as well, but disabled by
    default. Enable it by specifying
    @cpp #define CORRADE_STRING_STL_COMPATIBILITY @ce and for
    @ref std::string_view by compiling as C++17 and specifying
    @cpp #define CORRADE_STRING_STL_VIEW_COMPATIBILITY @ce before including the
    file. Including it multiple times with different macros defined works as
    well.

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
         * involved, propagating also @ref StringViewFlag::Global to
         * @ref viewFlags() if present. Otherwise creates a null-terminated
         * owning copy using @ref String(StringView).
         *
         * This function is primarily meant for efficiently passing
         * @ref BasicStringView "StringView" instances to APIs that expect
         * null-terminated @cpp const char* @ce. Mutating the result in any way
         * is undefined behavior.
         * @see @ref nullTerminatedGlobalView(),
         *      @ref nullTerminatedView(AllocatedInitT, StringView),
         *      @ref Containers-String-usage-c-string-conversion
         */
        static String nullTerminatedView(StringView view);

        /**
         * @brief Turn a view into a null-terminated string, bypassing SSO
         *
         * Compared to @ref nullTerminatedView(StringView) the null-terminated
         * copy is always allocated.
         * @see @ref nullTerminatedGlobalView(AllocatedInitT, StringView),
         *      @ref Containers-String-usage-sso,
         *      @ref Containers-String-usage-c-string-conversion
         */
        static String nullTerminatedView(AllocatedInitT, StringView view);

        /**
         * @brief Turn a view into a null-terminated global string
         *
         * If the view is both @ref StringViewFlag::NullTerminated and
         * @ref StringViewFlag::Global, returns a non-owning reference to it
         * without any extra allocations or copies involved, propagating also
         * @ref StringViewFlag::Global to @ref viewFlags(). Otherwise creates
         * a null-terminated owning copy using @ref String(StringView).
         *
         * This function is primarily meant for efficiently storing
         * @ref BasicStringView "StringView" instances, ensuring the
         * memory stays in scope and then passing them to APIs that expect
         * null-terminated @cpp const char* @ce. Mutating the result in any way
         * is undefined behavior.
         * @see @ref nullTerminatedView(),
         *      @ref nullTerminatedGlobalView(AllocatedInitT, StringView)
         */
        static String nullTerminatedGlobalView(StringView view);

        /**
         * @brief Turn a view into a null-terminated global string, bypassing SSO
         *
         * Compared to @ref nullTerminatedGlobalView(StringView) the
         * null-terminated copy is always allocated.
         * @see @ref nullTerminatedView(AllocatedInitT, StringView),
         *      @ref Containers-String-usage-sso,
         *      @ref Containers-String-usage-c-string-conversion
         */
        static String nullTerminatedGlobalView(AllocatedInitT, StringView view);

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
        #ifndef CORRADE_SINGLES_NO_ADVANCED_STRING_APIS
        /*implicit*/ String(ArrayView<const char> view); /**< @overload */
        #endif
        /* Without these there's ambiguity between StringView / ArrayView and
           char* */
        /*implicit*/ String(MutableStringView view); /**< @overload */
        #ifndef CORRADE_SINGLES_NO_ADVANCED_STRING_APIS
        /*implicit*/ String(ArrayView<char> view); /**< @overload */
        #endif

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
        #ifdef DOXYGEN_GENERATING_OUTPUT
        /*implicit*/ String(const char* data);
        #else
        /* To avoid ambiguity in certain cases of passing 0 to overloads that
           take either a String or std::size_t. See the
           constructZeroNullPointerAmbiguity() test for more info. FFS, zero as
           null pointer was deprecated in C++11 already, why is this still a
           problem?! */
        template<class T, typename std::enable_if<std::is_convertible<T, const char*>::value && !std::is_convertible<T, std::size_t>::value, int>::type = 0> /*implicit*/ String(T data): String{nullptr, nullptr, nullptr, data} {}
        #endif

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
         * @see @ref Containers-String-usage-sso,
         *      @ref Containers-String-usage-c-string-conversion
         */
        explicit String(AllocatedInitT, StringView view);
        #ifndef CORRADE_SINGLES_NO_ADVANCED_STRING_APIS
        explicit String(AllocatedInitT, ArrayView<const char> view); /**< @overload */
        #endif
        /* Without these there's ambiguity between StringView / ArrayView and
           char* */
        explicit String(AllocatedInitT, MutableStringView view); /**< @overload */
        #ifndef CORRADE_SINGLES_NO_ADVANCED_STRING_APIS
        explicit String(AllocatedInitT, ArrayView<char> view); /**< @overload */
        #endif

        /**
         * @brief Create a string instance bypassing SSO
         *
         * If @p other already has allocated data, the data ownership is
         * transferred. Otherwise a copy is allocated.
         * @see @ref Containers-String-usage-sso,
         *      @ref Containers-String-usage-c-string-conversion
         */
        explicit String(AllocatedInitT, String&& other);
        explicit String(AllocatedInitT, const String& other); /**< @overload */

        /**
         * @brief Construct from a null-terminated C string, bypassing SSO
         *
         * Compared to @ref String(const char*) the data is always allocated.
         * @see @ref Containers-String-usage-sso,
         *      @ref Containers-String-usage-c-string-conversion
         */
        explicit String(AllocatedInitT, const char* data);

        /**
         * @brief Construct from a sized C string
         *
         * Compared to @ref String(const char*, std::size_t) the data is always
         * allocated.
         * @see @ref Containers-String-usage-sso,
         *      @ref Containers-String-usage-c-string-conversion
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
         * @brief Take ownership of an external data array with implicit size
         *
         * Calculates the size using @ref std::strlen() and calls
         * @ref String(char*, std::size_t, Deleter).
         */
        #ifdef DOXYGEN_GENERATING_OUTPUT
        explicit String(char* data, Deleter deleter) noexcept;
        #else
        /* Gets ambigous when calling String{ptr, 0}. FFS, zero as null pointer
           was deprecated in C++11 already, why is this still a problem?! */
        template<class T, typename std::enable_if<std::is_convertible<T, Deleter>::value && !std::is_convertible<T, std::size_t>::value, int>::type = 0> String(char* data, T deleter) noexcept: String{deleter, nullptr, data} {}
        #endif

        /**
         * @brief Take ownership of an immutable external data array
         *
         * Casts away the @cpp const @ce and delegates to
         * @ref String(char*, std::size_t, Deleter). This constructor is
         * provided mainly to allow a @ref String instance to reference global
         * immutable data (such as C string literals) without having to make a
         * copy, it's the user responsibility to avoid mutating the data in any
         * way.
         * @see @ref nullTerminatedView(), @ref nullTerminatedGlobalView()
         */
        explicit String(const char* data, std::size_t size, Deleter deleter) noexcept: String{const_cast<char*>(data), size, deleter} {}

        /**
         * @brief Take ownership of an external data array with implicit size
         *
         * Calculates the size using @ref std::strlen() and calls
         * @ref String(const char*, std::size_t, Deleter).
         */
        #ifdef DOXYGEN_GENERATING_OUTPUT
        explicit String(const char* data, Deleter deleter) noexcept;
        #else
        /* Gets ambigous when calling String{ptr, 0}. FFS, zero as null pointer
           was deprecated in C++11 already, why is this still a problem?! */
        template<class T, typename std::enable_if<std::is_convertible<T, Deleter>::value && !std::is_convertible<T, std::size_t>::value, int>::type = 0> String(const char* data, T deleter) noexcept: String{deleter, nullptr, const_cast<char*>(data)} {}
        #endif

        /**
         * @brief Taking ownership of a null pointer is not allowed
         *
         * Since the @ref String class provides a guarantee of null-terminated
         * strings, @p data *can't* be @cpp nullptr @ce.
         */
        explicit String(std::nullptr_t, std::size_t size, Deleter deleter) = delete;

        /**
         * @brief Taking ownership of a null pointer is not allowed
         *
         * Since the @ref String class provides a guarantee of null-terminated
         * strings, @p data *can't* be @cpp nullptr @ce.
         */
        #ifdef DOXYGEN_GENERATING_OUTPUT
        explicit String(std::nullptr_t, Deleter deleter) = delete;
        #else
        /* Gets ambigous when calling String{nullptr, 0}. FFS, zero as null
           pointer was deprecated in C++11 already, why is this still a
           problem?! */
        template<class T, typename std::enable_if<std::is_convertible<T, Deleter>::value && !std::is_convertible<T, std::size_t>::value, int>::type = 0> String(std::nullptr_t, T) noexcept = delete;
        #endif

        /**
         * @brief Create a zero-initialized string
         * @param size      Size excluding the null terminator
         *
         * @see @relativeref{Corrade,ValueInit},
         *      @ref String(NoInitT, std::size_t),
         *      @ref String(DirectInitT, std::size_t, char)
         */
        explicit String(Corrade::ValueInitT, std::size_t size);

        /**
         * @brief Create an uninitialized string
         * @param size      Size excluding the null terminator
         *
         * While the string contents are left untouched, the null terminator
         * *does* get initialized to @cpp '\0' @ce. Useful if you're going to
         * overwrite the contents anyway.
         * @see @relativeref{Corrade,NoInit},
         *      @ref String(ValueInitT, std::size_t),
         *      @ref String(DirectInitT, std::size_t, char)
         */
        explicit String(Corrade::NoInitT, std::size_t size);

        /**
         * @brief Create a string initialized to a particular character
         * @param size      Size excluding the null terminator
         * @param c         Character value
         *
         * @see @relativeref{Corrade,DirectInit},
         *      @ref String(ValueInitT, std::size_t),
         *      @ref String(NoInitT, std::size_t)
         */
        explicit String(Corrade::DirectInitT, std::size_t size, char c);

        /** @todo combined AllocatedInit + Value/Direct/NoInit constructors */

        /**
         * @brief Construct from an external representation
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

        /**
         * @brief Copy constructor
         *
         * If @p other is a SSO instance, the copy is as well, otherwise a copy
         * is allocated using the default @cpp operator new[] @ce. The actual
         * string size isn't taken into account. See
         * @ref Containers-String-usage-sso for more information.
         */
        String(const String& other);

        /** @brief Move constructor */
        String(String&& other) noexcept;

        /**
         * @brief Copy assignment
         *
         * If @p other is a SSO instance, the copy is as well, otherwise a copy
         * is allocated using the default @cpp operator new[] @ce. The actual
         * string size isn't taken into account. See
         * @ref Containers-String-usage-sso for more information.
         */
        String& operator=(const String& other);

        /** @brief Move assignment */
        String& operator=(String&& other) noexcept;

        #ifndef CORRADE_SINGLES_NO_ADVANCED_STRING_APIS
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
         * and a default deleter is used. The string then resets data pointer,
         * size and deleter to be equivalent to a default-constructed instance.
         * In both the allocated and the SSO case the returned array contains a
         * sentinel null terminator (i.e., not counted into its size). Note
         * that with custom deleters the array is not guaranteed to be actually
         * mutable.
         * @see @ref release(), @ref isSmall()
         */
        /*implicit*/ operator Array<char>() &&;
        #endif

        /**
         * @brief Convert the string to external representation
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
         * @brief Whether the string is non-empty
         *
         * Returns @cpp true @ce if the string is non-empty, @cpp false @ce
         * otherwise. Compared to @ref BasicStringView::operator bool(), a
         * @ref String can never be @cpp nullptr @ce, so the pointer value
         * isn't taken into account here.
         * @see @ref isEmpty(), @ref size()
         */
        explicit operator bool() const;

        /**
         * @brief Whether the string is stored using small string optimization
         *
         * It's not allowed to call @ref deleter() or @ref release() on a SSO
         * instance. See @ref Containers-String-usage-sso for more information.
         */
        bool isSmall() const {
            return _small.size & Implementation::SmallStringBit;
        }

        /**
         * @brief View flags
         *
         * A @ref BasicStringView "StringView" constructed from this instance
         * will have these flags. @ref StringViewFlag::NullTerminated is
         * present always, @ref StringViewFlag::Global if the string was
         * originally created from a global null-terminated view with
         * @ref nullTerminatedView() or @ref nullTerminatedGlobalView().
         */
        StringViewFlags viewFlags() const;

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

        /**
         * @brief Whether the string is empty
         *
         * @see @ref operator bool(), @ref size()
         */
        bool isEmpty() const;

        /**
         * @brief String size
         *
         * Excludes the null terminator.
         * @see @ref isEmpty(), @ref operator bool()
         */
        std::size_t size() const;

        /**
         * @brief Pointer to the first byte
         *
         * @see @ref front(), @ref operator[]()
         */
        char* begin();
        const char* begin() const; /**< @overload */
        const char* cbegin() const; /**< @overload */

        /**
         * @brief Pointer to (one item after) the last byte
         *
         * @see @ref back(), @ref operator[]()
         */
        char* end();
        const char* end() const; /**< @overload */
        const char* cend() const; /**< @overload */

        /**
         * @brief First byte
         *
         * Expects there is at least one byte.
         * @see @ref isEmpty(), @ref begin(), @ref operator[]()
         */
        char& front();
        char front() const; /**< @overload */

        /**
         * @brief Last byte
         *
         * Expects there is at least one byte.
         * @see @ref isEmpty(), @ref end(), @ref operator[]()
         */
        char& back();
        char back() const; /**< @overload */

        /**
         * @brief Element access
         *
         * Expects that @p i is less than or equal to @ref size().
         * @see @ref front(), @ref back()
         */
        char& operator[](std::size_t i);
        char operator[](std::size_t i) const; /**< @overload */

        /**
         * @brief View on a slice
         *
         * Equivalent to @ref BasicStringView::slice(). Both arguments are
         * expected to be in range. If @p end points to (one item after) the
         * end of the original (null-terminated) string, the result has
         * @ref StringViewFlag::NullTerminated set.
         * @m_keywords{substr()}
         */
        /* Unlike sliceSize(T*, size_t), prefix(T*) and suffix(T*) this doesn't
           have ambiguity prevention for slice(0, 0) as such use case is rather
           rare I think */
        MutableStringView slice(char* begin, char* end);
        StringView slice(const char* begin, const char* end) const; /**< @overload */
        MutableStringView slice(std::size_t begin, std::size_t end); /**< @overload */
        StringView slice(std::size_t begin, std::size_t end) const; /**< @overload */

        /**
         * @brief View on a slice of given size
         *
         * Equivalent to @ref BasicStringView::sliceSize(). Both arguments are
         * expected to be in range. If `begin + size` points to (one item
         * after) the end of the original (null-terminated) string, the result
         * has @ref StringViewFlag::NullTerminated set.
         * @m_keywords{substr()}
         */
        #ifdef DOXYGEN_GENERATING_OUTPUT
        MutableStringView sliceSize(char* begin, std::size_t size);
        #else
        /* To avoid ambiguity when calling sliceSize(0, ...). FFS, zero as null
           pointer was deprecated in C++11 already, why is this still a
           problem?! */
        template<class T, typename std::enable_if<std::is_convertible<T, char*>::value && !std::is_convertible<T, std::size_t>::value, int>::type = 0> MutableStringView sliceSize(T begin, std::size_t size) {
            return sliceSizePointerInternal(begin, size);
        }
        #endif
        #ifdef DOXYGEN_GENERATING_OUTPUT
        StringView sliceSize(const char* begin, std::size_t size) const; /**< @overload */
        #else
        /* To avoid ambiguity when calling sliceSize(0, ...). FFS, zero as null
           pointer was deprecated in C++11 already, why is this still a
           problem?! */
        template<class T, typename std::enable_if<std::is_convertible<T, const char*>::value && !std::is_convertible<T, std::size_t>::value, int>::type = 0> StringView sliceSize(T begin, std::size_t size) const {
            return sliceSizePointerInternal(begin, size);
        }
        #endif
        MutableStringView sliceSize(std::size_t begin, std::size_t size); /**< @overload */
        StringView sliceSize(std::size_t begin, std::size_t size) const; /**< @overload */

        /**
         * @brief View on a prefix until a pointer
         *
         * Equivalent to @ref BasicStringView::prefix(T*) const. If @p end
         * points to (one item after) the end of the original (null-terminated)
         * string, the result has @ref StringViewFlag::NullTerminated set.
         */
        #ifdef DOXYGEN_GENERATING_OUTPUT
        MutableStringView prefix(char* end);
        #else
        /* To avoid ambiguity when calling sliceSize(0, ...). FFS, zero as null
           pointer was deprecated in C++11 already, why is this still a
           problem?! */
        template<class T, typename std::enable_if<std::is_convertible<T, char*>::value && !std::is_convertible<T, std::size_t>::value, int>::type = 0> MutableStringView prefix(T end) {
            return prefixPointerInternal(end);
        }
        #endif
        #ifdef DOXYGEN_GENERATING_OUTPUT
        StringView prefix(const char* end) const; /**< @overload */
        #else
        /* To avoid ambiguity when calling sliceSize(0, ...). FFS, zero as null
           pointer was deprecated in C++11 already, why is this still a
           problem?! */
        template<class T, typename std::enable_if<std::is_convertible<T, const char*>::value && !std::is_convertible<T, std::size_t>::value, int>::type = 0> StringView prefix(T end) const {
            return prefixPointerInternal(end);
        }
        #endif

        /**
         * @brief View on a suffix after a pointer
         *
         * Equivalent to @ref BasicStringView::suffix(T*) const. The result has
         * always @ref StringViewFlag::NullTerminated set.
         * @todo once non-deprecated suffix(std::size_t size) is a thing, add
         *      the ambiguity-preventing template here as well
         */
        MutableStringView suffix(char* begin);
        /**
         * @overload
         * @todo once non-deprecated suffix(std::size_t size) is a thing, add
         *      the ambiguity-preventing template here as well
         */
        StringView suffix(const char* begin) const;

        /**
         * @brief View on the first @p size bytes
         *
         * Equivalent to @ref BasicStringView::prefix(std::size_t) const. If
         * @p size is equal to @ref size(), the result has
         * @ref StringViewFlag::NullTerminated set.
         */
        MutableStringView prefix(std::size_t size);
        StringView prefix(std::size_t size) const; /**< @overload */

        /* Here will be suffix(std::size_t size), view on the last size
           bytes, once the deprecated suffix(std::size_t begin) is gone and
           enough time passes to not cause silent breakages in existing code. */

        /**
         * @brief View except the first @p size bytes
         *
         * Equivalent to @ref BasicStringView::exceptPrefix(). The result has
         * always @ref StringViewFlag::NullTerminated set.
         */
        MutableStringView exceptPrefix(std::size_t size);
        StringView exceptPrefix(std::size_t size) const; /**< @overload */

        #ifdef CORRADE_BUILD_DEPRECATED
        /** @copybrief exceptPrefix()
         * @m_deprecated_since_latest Use @ref exceptPrefix() instead.
         */
        CORRADE_DEPRECATED("use exceptPrefix() instead") MutableStringView suffix(std::size_t begin);
        /** @copybrief exceptPrefix()
         * @m_deprecated_since_latest Use @ref exceptPrefix() instead.
         */
        CORRADE_DEPRECATED("use exceptPrefix() instead") StringView suffix(std::size_t begin) const;
        #endif

        /**
         * @brief View except the last @p size bytes
         *
         * Equivalent to @ref BasicStringView::exceptSuffix(). If
         * @p size is @cpp 0 @ce, the result has
         * @ref StringViewFlag::NullTerminated set.
         */
        MutableStringView exceptSuffix(std::size_t size);
        StringView exceptSuffix(std::size_t size) const; /**< @overload */

        #ifdef CORRADE_BUILD_DEPRECATED
        /**
         * @copybrief exceptSuffix()
         * @m_deprecated_since_latest Use @ref exceptSuffix() instead.
         */
        CORRADE_DEPRECATED("use exceptSuffix() instead") MutableStringView except(std::size_t count);
        /**
         * @overload
         * @m_deprecated_since_latest Use @ref exceptSuffix() instead.
         */
        CORRADE_DEPRECATED("use exceptSuffix() instead") StringView except(std::size_t count) const;
        #endif

        #ifndef CORRADE_SINGLES_NO_ADVANCED_STRING_APIS
        /**
         * @brief Split on given character
         *
         * Equivalent to @ref BasicStringView::split(char) const.
         */
        Array<MutableStringView> split(char delimiter);
        Array<StringView> split(char delimiter) const; /**< @overload */

        /** @todo split(T*) / split(std::size_t) returning a Pair<StringView, StringView>
            (used frequently in Path::split*(), would save repetitive
            assertions), how to distinguish from split(char)?? rename split to
            splitOn()?? */

        /**
         * @brief Split on given substring
         *
         * Equivalent to @ref BasicStringView::split(StringView) const.
         */
        Array<MutableStringView> split(StringView delimiter);
        Array<StringView> split(StringView delimiter) const; /**< @overload */

        /**
         * @brief Split on given character, removing empty parts
         *
         * Equivalent to @ref BasicStringView::splitWithoutEmptyParts(char) const.
         */
        Array<MutableStringView> splitWithoutEmptyParts(char delimiter);
        Array<StringView> splitWithoutEmptyParts(char delimiter) const; /**< @overload */

        /** @todo once the deprecated splitWithoutEmptyParts(StringView) is
            removed and enough time passes, reintroduce it and make it work
            similarly to split(StringView), i.e. taking the delimiter as a
            whole */

        /**
         * @brief Split on any character from given set, removing empty parts
         *
         * Equivalent to @ref BasicStringView::splitOnAnyWithoutEmptyParts(StringView) const.
         */
        Array<MutableStringView> splitOnAnyWithoutEmptyParts(StringView delimiters);
        Array<StringView> splitOnAnyWithoutEmptyParts(StringView delimiters) const; /**< @overload */

        #ifdef CORRADE_BUILD_DEPRECATED
        /** @brief @copybrief splitOnAnyWithoutEmptyParts()
         * @m_deprecated_since_latest Use @ref splitOnAnyWithoutEmptyParts()
         *      instead.
         */
        CORRADE_DEPRECATED("use splitOnAnyWithoutEmptyParts() instead") Array<MutableStringView> splitWithoutEmptyParts(StringView delimiters);

        /** @overload
         * @m_deprecated_since_latest Use @ref splitOnAnyWithoutEmptyParts()
         *      instead.
         */
        CORRADE_DEPRECATED("use splitOnAnyWithoutEmptyParts() instead") Array<StringView> splitWithoutEmptyParts(StringView delimiters) const;
        #endif

        /**
         * @brief Split on whitespace, removing empty parts
         *
         * Equivalent to @ref BasicStringView::splitOnWhitespaceWithoutEmptyParts() const.
         */
        Array<MutableStringView> splitOnWhitespaceWithoutEmptyParts();
        Array<StringView> splitOnWhitespaceWithoutEmptyParts() const; /**< @overload */

        #ifdef CORRADE_BUILD_DEPRECATED
        /** @brief @copybrief splitOnWhitespaceWithoutEmptyParts()
         * @m_deprecated_since_latest Use @ref splitOnWhitespaceWithoutEmptyParts()
         *      instead.
         */
        CORRADE_DEPRECATED("use splitOnWhitespaceWithoutEmptyParts() instead") Array<MutableStringView> splitWithoutEmptyParts();

        /** @overload
         * @m_deprecated_since_latest Use @ref splitOnWhitespaceWithoutEmptyParts()
         *      instead.
         */
        CORRADE_DEPRECATED("use splitOnWhitespaceWithoutEmptyParts() instead") Array<StringView> splitWithoutEmptyParts() const;
        #endif

        /**
         * @brief Partition on a character
         *
         * Equivalent to @ref BasicStringView::partition(char) const. The last
         * returned value has always @ref StringViewFlag::NullTerminated set.
         */
        Array3<MutableStringView> partition(char separator);
        Array3<StringView> partition(char separator) const; /**< @overload */

        /**
         * @brief Partition on a substring
         *
         * Equivalent to @ref BasicStringView::partition(StringView) const. The
         * last returned value has always @ref StringViewFlag::NullTerminated
         * set.
         */
        Array3<MutableStringView> partition(StringView separator);
        Array3<StringView> partition(StringView separator) const; /**< @overload */

        /**
         * @brief Partition on a last occurence of a character
         *
         * Equivalent to @ref BasicStringView::partition(char) const. The last
         * returned value has always @ref StringViewFlag::NullTerminated set.
         */
        Array3<MutableStringView> partitionLast(char separator);
        Array3<StringView> partitionLast(char separator) const; /**< @overload */

        /**
         * @brief Partition on a last occurence of a substring
         *
         * Equivalent to @ref BasicStringView::partition(StringView) const. The
         * last returned value has always @ref StringViewFlag::NullTerminated
         * set.
         */
        Array3<MutableStringView> partitionLast(StringView separator);
        Array3<StringView> partitionLast(StringView separator) const; /**< @overload */

        /** @todo change these to return a Triple? it's a smaller header */

        /**
         * @brief Join strings with this view as the delimiter
         *
         * Equivalent to @ref BasicStringView::join().
         * @todo a mutable && overload that reuses the growable string storage
         *      instead of allocating new, when growable strings are a thing
         */
        String join(const StringIterable& strings) const;

        /**
         * @brief Join strings with this view as the delimiter, skipping empty parts
         *
         * Equivalent to @ref BasicStringView::joinWithoutEmptyParts().
         */
        String joinWithoutEmptyParts(const StringIterable& strings) const;
        #endif

        /**
         * @brief Whether the string begins with given prefix
         *
         * Equivalent to @ref BasicStringView::hasPrefix().
         */
        bool hasPrefix(StringView prefix) const;
        bool hasPrefix(char prefix) const; /**< @overload */

        /**
         * @brief Whether the string ends with given suffix
         *
         * Equivalent to @ref BasicStringView::hasSuffix().
         */
        bool hasSuffix(StringView suffix) const;
        bool hasSuffix(char suffix) const; /**< @overload */

        /**
         * @brief View with given prefix stripped
         *
         * Equivalent to @ref BasicStringView::exceptPrefix().
         */
        MutableStringView exceptPrefix(StringView prefix);
        StringView exceptPrefix(StringView prefix) const; /**< @overload */

        #ifdef DOXYGEN_GENERATING_OUTPUT
        /**
         * @brief Using char literals for prefix stripping is not allowed
         *
         * To avoid accidentally interpreting a @cpp char @ce literal as a size
         * and calling @ref exceptPrefix(std::size_t) instead, or vice versa,
         * you have to always use a string literal to call this function.
         */
        MutableStringView exceptPrefix(char prefix) = delete;
        StringView exceptPrefix(char prefix) const = delete; /**< @overload */
        #else
        template<class T, typename std::enable_if<std::is_same<typename std::decay<T>::type, char>::value, int>::type = 0> MutableStringView exceptPrefix(T&& prefix) = delete;
        template<class T, typename std::enable_if<std::is_same<typename std::decay<T>::type, char>::value, int>::type = 0> StringView exceptPrefix(T&& prefix) const = delete;
        #endif

        #ifdef CORRADE_BUILD_DEPRECATED
        /**
         * @brief @copybrief exceptPrefix()
         * @m_deprecated_since_latest Deprecated due to confusing naming that
         *      could imply the original instance gets modified. Use
         *      @ref exceptPrefix() instead.
         */
        CORRADE_DEPRECATED("use exceptPrefix() instead") MutableStringView stripPrefix(StringView prefix) {
            return exceptPrefix(prefix);
        }

        /**
         * @brief @copybrief exceptPrefix()
         * @m_deprecated_since_latest Deprecated due to confusing naming that
         *      could imply the original instance gets modified. Use
         *      @ref exceptPrefix() instead.
         */
        CORRADE_DEPRECATED("use exceptPrefix() instead") StringView stripPrefix(StringView prefix) const {
            return exceptPrefix(prefix);
        }
        #endif

        /**
         * @brief View with given suffix stripped
         *
         * Equivalent to @ref BasicStringView::exceptSuffix().
         */
        MutableStringView exceptSuffix(StringView suffix);
        StringView exceptSuffix(StringView suffix) const; /**< @overload */

        #ifdef DOXYGEN_GENERATING_OUTPUT
        /**
         * @brief Using char literals for suffix stripping is not allowed
         *
         * To avoid accidentally interpreting a @cpp char @ce literal as a size
         * and calling @ref exceptSuffix(std::size_t) instead, or vice versa,
         * you have to always use a string literal to call this function.
         */
        MutableStringView exceptSuffix(char suffix) = delete;
        StringView exceptSuffix(char suffix) const = delete; /**< @overload */
        #else
        template<class T, typename std::enable_if<std::is_same<typename std::decay<T>::type, char>::value, int>::type = 0> MutableStringView exceptSuffix(T&& suffix) = delete;
        template<class T, typename std::enable_if<std::is_same<typename std::decay<T>::type, char>::value, int>::type = 0> StringView exceptSuffix(T&& suffix) const = delete;
        #endif

        #ifdef CORRADE_BUILD_DEPRECATED
        /**
         * @brief @copybrief exceptSuffix()
         * @m_deprecated_since_latest Deprecated due to confusing naming that
         *      could imply the original instance gets modified. Use
         *      @ref exceptSuffix() instead.
         */
        CORRADE_DEPRECATED("use exceptSuffix() instead") MutableStringView stripSuffix(StringView suffix) {
            return exceptSuffix(suffix);
        }

        /**
         * @brief @copybrief exceptSuffix()
         * @m_deprecated_since_latest Deprecated due to confusing naming that
         *      could imply the original instance gets modified. Use
         *      @ref exceptSuffix() instead.
         */
        CORRADE_DEPRECATED("use exceptSuffix() instead") StringView stripSuffix(StringView suffix) const {
            return exceptSuffix(suffix);
        }
        #endif

        /**
         * @brief View with given characters trimmed from prefix and suffix
         *
         * Equivalent to @ref BasicStringView::trimmed(StringView) const.
         */
        MutableStringView trimmed(StringView characters);
        StringView trimmed(StringView characters) const; /**< @overload */

        /**
         * @brief View with whitespace trimmed from prefix and suffix
         *
         * Equivalent to @ref BasicStringView::trimmed() const.
         */
        MutableStringView trimmed();
        StringView trimmed() const; /**< @overload */

        /**
         * @brief View with given characters trimmed from prefix
         *
         * Equivalent to @ref BasicStringView::trimmedPrefix(StringView) const.
         */
        MutableStringView trimmedPrefix(StringView characters);
        StringView trimmedPrefix(StringView characters) const; /**< @overload */

        /**
         * @brief View with whitespace trimmed from prefix
         *
         * Equivalent to @ref BasicStringView::trimmedPrefix() const.
         */
        MutableStringView trimmedPrefix();
        StringView trimmedPrefix() const; /**< @overload */

        /**
         * @brief View with given characters trimmed from suffix
         *
         * Equivalent to @ref BasicStringView::trimmedSuffix(StringView) const.
         */
        MutableStringView trimmedSuffix(StringView characters);
        StringView trimmedSuffix(StringView characters) const; /**< @overload */

        /**
         * @brief View with whitespace trimmed from suffix
         *
         * Equivalent to @ref BasicStringView::trimmedSuffix() const.
         */
        MutableStringView trimmedSuffix();
        StringView trimmedSuffix() const; /**< @overload */

        /**
         * @brief Find a substring
         *
         * Equivalent to @ref BasicStringView::find(StringView) const.
         */
        MutableStringView find(StringView substring);
        StringView find(StringView substring) const; /**< @overload */

        /**
         * @brief Find a substring
         *
         * Equivalent to @ref BasicStringView::find(char) const, which in turn
         * is a specialization of @ref BasicStringView::find(StringView) const.
         */
        MutableStringView find(char character);
        StringView find(char character) const; /**< @overload */

        /**
         * @brief Find a substring with a custom failure pointer
         *
         * Equivalent to @ref BasicStringView::findOr(StringView, T*) const.
         */
        MutableStringView findOr(StringView substring, char* fail);
        StringView findOr(StringView substring, const char* fail) const; /**< @overload */

        /**
         * @brief Find a substring with a custom failure pointer
         *
         * Equivalent to @ref BasicStringView::findOr(char, T*) const, which in
         * turn is a specialization of @ref BasicStringView::findOr(StringView, T*) const.
         */
        MutableStringView findOr(char character, char* fail);
        StringView findOr(char character, const char* fail) const; /**< @overload */

        /**
         * @brief Find the last occurence of a substring
         *
         * Equivalent to @ref BasicStringView::findLast(StringView) const.
         */
        MutableStringView findLast(StringView substring);
        StringView findLast(StringView substring) const; /**< @overload */

        /**
         * @brief Find the last occurence of a substring
         *
         * Equivalent to @ref BasicStringView::findLast(char) const, which in
         * turn is a specialization of @ref BasicStringView::findLast(StringView) const.
         */
        MutableStringView findLast(char character);
        StringView findLast(char character) const; /**< @overload */

        /**
         * @brief Find the last occurence of a substring with a custom failure pointer
         *
         * Equivalent to @ref BasicStringView::findLastOr(StringView, T*) const.
         */
        MutableStringView findLastOr(StringView substring, char* fail);
        StringView findLastOr(StringView substring, const char* fail) const; /**< @overload */

        /**
         * @brief Find the last occurence of a substring with a custom failure pointer
         *
         * Equivalent to @ref BasicStringView::findLastOr(char, T*) const,
         * which in turn is a specialization of @ref BasicStringView::findLastOr(StringView, T*) const.
         */
        MutableStringView findLastOr(char character, char* fail);
        StringView findLastOr(char character, const char* fail) const; /**< @overload */

        /**
         * @brief Whether the string contains a substring
         *
         * Equivalent to @ref BasicStringView::contains(StringView) const.
         */
        bool contains(StringView substring) const;

        /**
         * @brief Whether the string contains a character
         *
         * Equivalent to @ref BasicStringView::contains(char) const.
         */
        bool contains(char character) const;

        /**
         * @brief Find any character from given set
         *
         * Equivalent to @ref BasicStringView::findAny().
         */
        MutableStringView findAny(StringView characters);
        StringView findAny(StringView characters) const; /**< @overload */

        /**
         * @brief Find any character from given set with a custom failure pointer
         *
         * Equivalent to @ref BasicStringView::findAnyOr().
         */
        MutableStringView findAnyOr(StringView characters, char* fail);
        StringView findAnyOr(StringView characters, const char* fail) const; /**< @overload */

        /**
         * @brief Find the last occurence of any character from given set
         *
         * Equivalent to @ref BasicStringView::findLastAny().
         */
        MutableStringView findLastAny(StringView characters);
        StringView findLastAny(StringView characters) const; /**< @overload */

        /**
         * @brief Find the last occurence of any character from given set with a custom failure pointer
         *
         * Equivalent to @ref BasicStringView::findLastAnyOr().
         */
        MutableStringView findLastAnyOr(StringView characters, char* fail);
        StringView findLastAnyOr(StringView characters, const char* fail) const; /**< @overload */

        /**
         * @brief Whether the string contains any character from given set
         *
         * Equivalent to @ref BasicStringView::containsAny().
         */
        bool containsAny(StringView substring) const;

        /**
         * @brief Count of occurences of given character
         *
         * Equivalent to @ref BasicStringView::count(char) const.
         */
        std::size_t count(char character) const;

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
        /* Delegated to from the (templated) String(const char*). THREE extra
           nullptr arguments to avoid accidental ambiguous overloads. */
        explicit String(std::nullptr_t, std::nullptr_t, std::nullptr_t, const char* data);
        /* Delegated to from the (templated) String(char*, Deleter). Argument
           order shuffled together with a null parameter to avoid accidental
           ambiguous overloads. */
        explicit String(Deleter deleter, std::nullptr_t, char* data) noexcept;

        CORRADE_UTILITY_LOCAL void construct(Corrade::NoInitT, std::size_t size);
        CORRADE_UTILITY_LOCAL void construct(const char* data, std::size_t size);
        CORRADE_UTILITY_LOCAL void copyConstruct(const String& other);
        CORRADE_UTILITY_LOCAL void destruct();
        struct Data {
            const char* data;
            std::size_t size;
        };
        CORRADE_UTILITY_LOCAL Data dataInternal() const;

        MutableStringView sliceSizePointerInternal(char* begin, std::size_t size);
        StringView sliceSizePointerInternal(const char* begin, std::size_t size) const;
        MutableStringView prefixPointerInternal(char* end);
        StringView prefixPointerInternal(const char* end) const;

        /* Small string optimization. Following size restrictions from
           StringView (which uses the top two bits for marking global and
           null-terminated views), we can use the second highest bit of the
           size to denote a small string. The highest bit, marked as G in the
           below diagram, is used to preserve StringViewFlag::Global in case of
           a nullTerminatedGlobalView() and a subsequent conversion back to a
           StringView. In case of a large string, there's size, data pointer
           and deleter pointer, either 24 (or 12) bytes in total. In case of a
           small string, we can store the size only in one byte out of 8 (or
           4), which then gives us 23 (or 11) bytes for storing the actual
           data, excluding the null terminator that's at most 22 / 10 ASCII
           chars. With the two topmost bits reserved, it's still 6 bits left
           for the size, which is more than enough in this case.

           On LE the layout is as follows (bits inside a byte are flipped for
           clarity as well). A useful property of this layout is that the SSO
           data are pointer-aligned as well:

            +-------------------------------+---------+
            |             string            | si | 1G |
            |              data             | ze |    |
            |             23B/11B           | 6b | 2b |
            +-------------------+-----+++++++---------+
                                | LSB |||||||   MSB   |
            +---------+---------+-----+++++++---------+
            |  data   |  data   |      size      | 0G |
            | pointer | deleter |                |    |
            |  8B/4B  |  8B/4B  |  56b/24b  | 6b | 2b |
            +---------+---------+-----------+---------+

           On BE it's like this. In this case it's however not possible to
           have both the global/SSO bits in the same positions *and* the SSO
           data aligned. Having consistent access to the G0/G1 bits made more
           sense from the implementation perspective, so that won.

            +---------+-------------------------------+
            | G1 | si |             string            |
            |    | ze |              data             |
            | 2b | 6b |             23B/11B           |
            +---------+++++++-----+-------------------+
            |   MSB   ||||||| LSB |
            +---------+++++++-----+---------+---------+
            | G0 |     size       |  data   |  data   |
            |    |                | pointer | deleter |
            | 2b | 6b |  56b/24b  |  8B/4B  |  8B/4B  |
            +---------+-----------+---------+---------+

           I originally tried storing the "small string" bit in the lowest bit
           of the deleter pointer, but function pointers apparently can have
           odd addresses on some platforms as well:

            https://lists.llvm.org/pipermail/llvm-dev/2018-March/121953.html

           The above approach is consistent with StringView, which is the
           preferrable solution after all. */
        struct Small {
            #ifdef CORRADE_TARGET_BIG_ENDIAN
            unsigned char size;
            char data[Implementation::SmallStringSize];
            #else
            char data[Implementation::SmallStringSize];
            unsigned char size;
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
