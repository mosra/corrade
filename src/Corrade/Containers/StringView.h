#ifndef Corrade_Containers_StringView_h
#define Corrade_Containers_StringView_h
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
 * @brief Class @ref Corrade::Containers::BasicStringView, typedef @ref Corrade::Containers::StringView, @ref Corrade::Containers::MutableStringView, literal @link Corrade::Containers::Literals::StringLiterals::operator""_s() @endlink
 * @m_since_latest
 * @experimental
 */

#include <cstddef>
/* std::declval() is said to be in <utility> but libstdc++, libc++ and MSVC STL
   all have it directly in <type_traits> because it just makes sense */
#include <type_traits>

#include "Corrade/Corrade.h"
#include "Corrade/Containers/Containers.h"
#include "Corrade/Containers/EnumSet.h"
#include "Corrade/Utility/DebugAssert.h"
#include "Corrade/Utility/Move.h"
#ifndef CORRADE_SINGLES_NO_DEBUG
#include "Corrade/Utility/Utility.h"
#endif
#include "Corrade/Utility/visibility.h"
#ifdef CORRADE_BUILD_DEPRECATED
/* For join(), which used to take an ArrayView<StringView> */
#include "Corrade/Containers/StringIterable.h"
#endif

namespace Corrade { namespace Containers {

namespace Implementation {
    template<class, class> struct StringViewConverter;
    #ifndef CORRADE_SINGLES_NO_ARRAYTUPLE_COMPATIBILITY
    /* So ArrayTuple can update the data pointer */
    template<class T>
        #ifndef CORRADE_MSVC2015_COMPATIBILITY
        /* warns that "the inline specifier cannot be used when a friend
           declaration refers to a specialization of a function template" due
           to friend dataRef<>() being used below. AMAZING */
        inline
        #endif
    T*& dataRef(BasicStringView<T>& view) {
        return view._data;
    }
    #endif
}

/**
@brief String view flag
@m_since_latest
@experimental

@see @ref StringViewFlags, @ref BasicStringView
*/
enum class StringViewFlag: std::size_t {
    /**
     * The referenced string is global, i.e., with an unlimited lifetime. A
     * string view with this flag set doesn't need to have a copy allocated in
     * order to ensure it stays in scope.
     */
    Global = std::size_t{1} << (sizeof(std::size_t)*8 - 1),

    /**
     * The referenced string is null-terminated. A string view with this flag
     * set doesn't need to have a null-terminated copy allocated in order to
     * pass to an API that expects only null-terminated strings.
     * @see @ref Containers-BasicStringView-usage-c-string-conversion
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

#ifndef CORRADE_SINGLES_NO_DEBUG
/** @debugoperatorclassenum{BasicStringView,StringViewFlag} */
CORRADE_UTILITY_EXPORT Utility::Debug& operator<<(Utility::Debug& debug, StringViewFlag value);

/** @debugoperatorclassenum{BasicStringView,StringViewFlags} */
CORRADE_UTILITY_EXPORT Utility::Debug& operator<<(Utility::Debug& debug, StringViewFlags value);
#endif

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
literals, but the recommended way is using the @link Literals::StringLiterals::operator""_s() @endlink
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
@ref slice(), @ref sliceSize(), @ref prefix(), @ref suffix(),
@ref exceptPrefix() and @ref exceptSuffix() --- and in addition it provides
string-specific utilities. These are are all derived from the slicing APIs,
which means they also return sub-views of the original string:

<ul>
<li>@ref split() and @ref splitWithoutEmptyParts() split the view on given set
of delimiter characters</li>
<li>@ref join() and @ref joinWithoutEmptyParts() is an inverse of the
above</li>
<li>@ref partition() is similar to @ref split(), but always returning three
elements with a clearly defined behavior, which can make certain code more
robust while reducing the amount of possible error states</li>
<li>@ref trimmed() (and its variants @ref trimmedPrefix() /
@ref trimmedSuffix()), commonly used to remove leading and trailing
whitespace</li>
<li>@ref exceptPrefix(StringView) const / @ref exceptSuffix(StringView) const
checks that a view starts (or ends) with given string and then removes it:

@snippet Containers.cpp StringView-usage-slicing
</li>
</ul>

@subsection Containers-BasicStringView-usage-find Character and substring lookup

@todoc document once also the findNotAny() and findLastNotAny() variants exist

@subsection Containers-BasicStringView-usage-c-string-conversion Converting StringView instances to null-terminated C strings

If possible when interacting with 3rd party APIs, passing a string together
with the size information is always preferable to passing just a plain
@cpp const char* @ce. Apart from saving an unnecessary @ref std::strlen() call
it can avoid unbounded memory reads in security-critical scenarios.

Unlike a @ref String, string views can point to any slice of a larger string
and thus can't guarantee null termination. Because of this and because even a
view with @ref StringViewFlag::NullTerminated can still contain a @cpp '\0' @ce
anywhere in the middle, there's no implicit conversion to @cpp const char* @ce
provided, and the pointer returned by @ref data() should only be used together
with @ref size().

The quickest safe way to get a null-terminated string out of a @ref StringView
is to convert the view to a @ref String and then use @ref String::data().
However, such operation will unconditionally make a copy of the string, which
is unnecessary work if the view was null-terminated already. To avoid that,
there's @ref String::nullTerminatedView(), which will make a copy only if the
view is not already null-terminated, directly referencing the view with a no-op
deleter otherwise. For example, when opening a file using @ref std::fopen():

@snippet Containers.cpp StringView-c-string-nullterminated

Similarly as described in @ref Containers-String-usage-c-string-conversion,
pointers to data in SSO instances will get invalidated when the instance is
moved. With @ref String::nullTerminatedGlobalView(AllocatedInitT, StringView)
the null-terminated copy will be always allocated, avoiding this problem:

@snippet Containers.cpp StringView-c-string-allocatedinit

@section Containers-BasicStringView-array-views Conversion to array views

String views are implicitly convertible to @ref ArrayView as described in the
following table. This also extends to other container types constructibe from
@ref ArrayView, which means for example that a @ref StridedArrayView1D is
implicitly convertible from a string view as well.

String view type                | ↭ | Array view type
------------------------------- | - | ---------------------
@ref StringView                 | → | @ref ArrayView "ArrayView<const char>"
@ref MutableStringView          | → | @ref ArrayView "ArrayView<const char>"
@ref MutableStringView          | → | @ref ArrayView "ArrayView<char>"

@section Containers-BasicStringView-stl STL compatibility

Instances of @ref StringView and @ref BasicStringView are *implicitly*
convertible from and to @ref std::string if you include
@ref Corrade/Containers/StringStl.h. The conversion is provided in a separate
header to avoid unconditional @cpp #include <string> @ce, which significantly
affects compile times. The following table lists allowed conversions:

Corrade type                    | ↭ | STL type
------------------------------- | - | ---------------------
@ref StringView                 | ← | @ref std::string
@ref StringView                 | ← | @ref std::string "const std::string"
@ref StringView                 | → | @ref std::string (data copy)
@ref StringView                 | → | @ref std::string "const std::string" (data copy)
@ref MutableStringView          | ← | @ref std::string
@ref MutableStringView          | → | @ref std::string (data copy)
@ref MutableStringView          | → | @ref std::string "const std::string" (data copy)

Example:

@snippet Containers-stl.cpp StringView

Creating a @ref std::string instance always involves a data copy,
while going the other way always creates a non-owning reference without
allocations or copies. @ref StringView / @ref MutableStringView created from a
@ref std::string always have @ref StringViewFlag::NullTerminated set, but the
usual conditions regarding views apply --- if the original string is modified,
view pointer, size or the null termination property may not be valid anymore.

On compilers that support C++17 and @ref std::string_view, implicit conversion
from and to it is provided in @ref Corrade/Containers/StringStlView.h. For
similar reasons, it's a dedicated header to avoid unconditional
@cpp #include <string_view> @ce, but this one is even significantly heavier
than the @ref string "<string>" include on certain implementations, so it's
separate from a @ref std::string as well. The following table lists allowed
conversions:

Corrade type                    | ↭ | STL type
------------------------------- | - | ---------------------
@ref StringView                 | ⇆ | @ref std::string_view
@ref MutableStringView          | → | @ref std::string_view

Example:

@snippet Containers-stl17.cpp StringView

The @ref std::string_view type doesn't have any mutable counterpart, so there's
no possibility to create a @ref MutableStringView out of it. Because
@ref std::string_view doesn't preserve any information about the string origin,
neither @ref StringViewFlag::NullTerminated nor @ref StringViewFlag::Global is
set in a @ref StringView converted from it.

Finally, the @ref Corrade/Containers/StringStlHash.h header provides a
@ref std::hash specialization for @ref StringView / @ref MutableStringView,
making it usable in @ref std::unordered_map and @ref std::unordered_set. It's
* *also* separate, due to dependency on @cpp #include <functional> @ce which is
among the heaviest STL headers in existence, and which is only really needed
when you deal with unordered containers.

<b></b>

@m_class{m-block m-success}

@par Single-header version
    This class, together with @ref String, is also available as a single-header
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
    file in both the headers and the implementation. Including it multiple
    times with different macros defined works as well.

@experimental
*/
/* All member functions are const because the view doesn't own the data */
template<class T> class
#ifndef CORRADE_TARGET_MSVC
/* If it's here, MSVC complains that the out-of-class inline functions have a
   definition while being dllimport'd. If I remove it, GCC then complains that
   the export in StringView.cpp is ignored as the type is already defined,
   proceeding with a linker error. */
CORRADE_UTILITY_EXPORT
#endif
BasicStringView {
    public:
        /**
         * @brief Default constructor
         *
         * A default-constructed instance has @ref StringViewFlag::Global set.
         * @see @ref BasicStringView(T*, StringViewFlags)
         */
        #ifdef DOXYGEN_GENERATING_OUTPUT
        constexpr /*implicit*/ BasicStringView(std::nullptr_t = nullptr) noexcept;
        #else
        /* To avoid ambiguity in certain cases of passing 0 to overloads that
           take either a StringView or std::size_t. See the
           constructZeroNullPointerAmbiguity() test for more info. FFS, zero as
           null pointer was deprecated in C++11 already, why is this still a
           problem?! */
        template<class U, typename std::enable_if<std::is_same<std::nullptr_t, U>::value, int>::type = 0> constexpr /*implicit*/ BasicStringView(U) noexcept: _data{}, _sizePlusFlags{std::size_t(StringViewFlag::Global)} {}
        constexpr /*implicit*/ BasicStringView() noexcept: _data{}, _sizePlusFlags{std::size_t(StringViewFlag::Global)} {}
        #endif

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
         * use the @link Literals::StringLiterals::operator""_s() @endlink
         * literal instead.
         * @see @ref BasicStringView(T*, StringViewFlags)
         */
        constexpr /*implicit*/ BasicStringView(T* data, std::size_t size, StringViewFlags flags = {}) noexcept: _data{data}, _sizePlusFlags{(
            /* This ends up being called from BasicStringView(T*, Flags), so
               basically on every implicit conversion from a C string, thus
               the release build perf aspect wins over safety. Additionally,
               it makes little sense to check the size constraint on 64-bit, if
               64-bit code happens to go over then it's got bigger problems
               than this assert. */
            #ifdef CORRADE_TARGET_32BIT
            CORRADE_CONSTEXPR_DEBUG_ASSERT(size < std::size_t{1} << (sizeof(std::size_t)*8 - 2),
                "Containers::StringView: string expected to be smaller than 2^" << Utility::Debug::nospace << sizeof(std::size_t)*8 - 2 << "bytes, got" << size),
            #endif
            CORRADE_CONSTEXPR_DEBUG_ASSERT(data || !(flags & StringViewFlag::NullTerminated),
                "Containers::StringView: can't use StringViewFlag::NullTerminated with null data"),
            size|(std::size_t(flags) & Implementation::StringViewSizeMask))} {}

        /**
         * @brief Construct from a @ref String
         *
         * The resulting view has @ref StringViewFlag::NullTerminated set
         * always, and @ref StringViewFlag::Global if the string was originally
         * created from a global null-terminated view with
         * @ref String::nullTerminatedView() or
         * @ref String::nullTerminatedGlobalView().
         */
        /*implicit*/ BasicStringView(String& data) noexcept;

        /**
         * @brief Construct from a const @ref String
         *
         * Enabled only if the view is not mutable. The resulting view has
         * @ref StringViewFlag::NullTerminated set always, and
         * @ref StringViewFlag::Global if the string was created from a global
         * null-terminated view with @ref String::nullTerminatedView() or
         * @ref String::nullTerminatedGlobalView().
         */
        template<class U = T
            #ifndef DOXYGEN_GENERATING_OUTPUT
            /* typename std::enable_if<std::is_const<U>::value, int>::type = 0
               cannot be used because GCC and Clang then have different
               mangling for the deinlined specialization in StringView.cpp,
               which means Corrade built with GCC cannot be used with Clang and
               vice versa. With GCC-built Corrade, Clang wants to link to
                _ZN7Corrade10Containers15BasicStringViewIKcEC1IS2_TnNSt9enable_ifIXsr3std8
               which c++filt cannot even demangle, the other way GCC wants
                Corrade::Containers::BasicStringView<char const>::BasicStringView<char const, 0>(Corrade::Containers::String const&)
               which Clang doesn't export. */
            , class = typename std::enable_if<std::is_const<U>::value>::type
            #endif
        > /*implicit*/ BasicStringView(const String& data) noexcept;

        #ifndef CORRADE_SINGLES_NO_ADVANCED_STRING_APIS
        /**
         * @brief Construct from an @ref ArrayView
         *
         * The resulting view has the same size as @p data, by default no
         * null-termination is assumed.
         */
        #ifdef DOXYGEN_GENERATING_OUTPUT
        /*implicit*/ BasicStringView(ArrayView<T> data, StringViewFlags flags = {}) noexcept;
        #else
        /* This has to accept any type and then delegate to a private
           constructor instead of directly taking ArrayView<T>, due to how
           overload resolution works in copy initialization as opposed to a
           direct constructor/function call. If it would take ArrayView<T>
           directly, `Array<char> -> ArrayView<const char> -> StringView`
           wouldn't work because it's one custom conversion sequence more than
           allowed in a copy initialization, and to make that work, this class
           would have to replicate all ArrayView constructors including
           conversion from Array etc., which isn't feasible. Similar approach
           is chosen in Iterable and StringIterable.

           It's also explicitly disallowing T[] arguments (which are implicitly
           convertible to an ArrayView), because those should be picking the T*
           overload and rely on strlen(), consistently with how C string
           literals work; and disallowing construction from a StringView
           because it'd get preferred over the implicit copy constructor. */
        /** @todo even though the implicit copy constructor would be overriden
            without the is_same part, is_trivially_copyable still says yes?! */
        template<class U, class = typename std::enable_if<!std::is_array<typename std::remove_reference<U&&>::type>::value && !std::is_same<typename std::decay<U&&>::type, BasicStringView<T>>::value && !std::is_same<typename std::decay<U&&>::type, std::nullptr_t>::value, decltype(ArrayView<T>{std::declval<U&&>()})>::type> constexpr /*implicit*/ BasicStringView(U&& data, StringViewFlags flags = {}) noexcept: BasicStringView{flags, ArrayView<T>(data)} {}
        #endif
        #endif

        /** @brief Construct a @ref StringView from a @ref MutableStringView */
        template<class U
            #ifndef DOXYGEN_GENERATING_OUTPUT
            , typename std::enable_if<std::is_same<const U, T>::value, int>::type = 0
            #endif
        > constexpr /*implicit*/ BasicStringView(BasicStringView<U> mutable_) noexcept: _data{mutable_._data}, _sizePlusFlags{mutable_._sizePlusFlags} {}

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
        #ifdef DOXYGEN_GENERATING_OUTPUT
        /*implicit*/ BasicStringView(T* data, StringViewFlags extraFlags = {}) noexcept;
        #else
        /* To avoid ambiguity in certain cases of passing 0 to overloads that
           take either a StringView or std::size_t, *and* avoid ambiguity with
           the other two StringView(U) overloads above. See the
           constructZeroNullPointerAmbiguity() test for more info. FFS, zero as
           null pointer was deprecated in C++11 already, why is this still a
           problem?! */
        template<class U, typename std::enable_if<std::is_pointer<U>::value && std::is_convertible<const U&, T*>::value, int>::type = 0> /*implicit*/ BasicStringView(U data, StringViewFlags extraFlags = {}) noexcept: BasicStringView{data, extraFlags, nullptr} {}
        #endif

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
        template<class U, class = decltype(Implementation::StringViewConverter<T, typename std::decay<U&&>::type>::from(std::declval<U&&>()))> constexpr /*implicit*/ BasicStringView(U&& other) noexcept: BasicStringView{Implementation::StringViewConverter<T, typename std::decay<U&&>::type>::from(Utility::forward<U>(other))} {}

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

        /**
         * @brief Whether the string is non-empty and non-null
         *
         * Returns @cpp true @ce if the string is non-empty *and* the pointer
         * is not @cpp nullptr @ce, @cpp false @ce otherwise. If you rely on
         * just one of these conditions, use @ref isEmpty() and @ref data()
         * instead.
         * @see @ref size()
         */
        constexpr explicit operator bool() const {
            return _data && (_sizePlusFlags & ~Implementation::StringViewSizeMask);
        }

        /** @brief Flags */
        constexpr StringViewFlags flags() const {
            return StringViewFlag(_sizePlusFlags & Implementation::StringViewSizeMask);
        }

        /**
         * @brief String data
         *
         * The pointer is not guaranteed to be null-terminated, use
         * @ref flags() and @ref StringViewFlag::NullTerminated to check for
         * the presence of a null terminator.
         * @see @ref operator bool()
         */
        constexpr T* data() const { return _data; }

        /**
         * @brief String size
         *
         * Excludes the null terminator.
         * @see @ref isEmpty(), @ref operator bool()
         */
        constexpr std::size_t size() const {
            return _sizePlusFlags & ~Implementation::StringViewSizeMask;
        }

        /**
         * @brief Whether the string is empty
         *
         * @see @ref operator bool(), @ref size()
         */
        constexpr bool isEmpty() const {
            return !(_sizePlusFlags & ~Implementation::StringViewSizeMask);
        }

        /**
         * @brief Pointer to the first byte
         *
         * @see @ref front(), @ref operator[]()
         */
        constexpr T* begin() const { return _data; }
        constexpr T* cbegin() const { return _data; } /**< @overload */

        /**
         * @brief Pointer to (one item after) the last byte
         *
         * @see @ref back(), @ref operator[]()
         */
        constexpr T* end() const {
            return _data + (_sizePlusFlags & ~Implementation::StringViewSizeMask);
        }
        constexpr T* cend() const {
            return _data + (_sizePlusFlags & ~Implementation::StringViewSizeMask);
        } /**< @overload */

        /**
         * @brief First byte
         *
         * Expects there is at least one byte.
         * @see @ref isEmpty(), @ref begin(), @ref operator[]()
         */
        constexpr T& front() const;

        /**
         * @brief Last byte
         *
         * Expects there is at least one byte.
         * @see @ref isEmpty(), @ref end(), @ref operator[]()
         */
        constexpr T& back() const;

        /**
         * @brief Element access
         *
         * Expects that @p i is less than @ref size(), or less than or equal to
         * @ref size() if the string is @ref StringViewFlag::NullTerminated.
         * @see @ref front(), @ref back()
         */
        constexpr T& operator[](std::size_t i) const;

        /**
         * @brief View slice
         *
         * Both arguments are expected to be in range. Propagates the
         * @ref StringViewFlag::Global flag and if @p end points to (one item
         * after) the end of the original null-terminated string, the result
         * has @ref StringViewFlag::NullTerminated also.
         * @m_keywords{substr()}
         * @see @ref sliceSize(), @ref prefix(), @ref suffix(),
         *      @ref exceptPrefix(), @ref exceptSuffix(),
         *      @ref slice(std::size_t, std::size_t) const
         */
        /* Unlike sliceSize(T*, size_t), prefix(T*) and suffix(T*) this doesn't
           have ambiguity prevention for slice(0, 0) as such use case is rather
           rare I think */
        constexpr BasicStringView<T> slice(T* begin, T* end) const;

        /** @overload */
        constexpr BasicStringView<T> slice(std::size_t begin, std::size_t end) const;

        /**
         * @brief View slice of given size
         *
         * Equivalent to @cpp data.slice(begin, begin + size) @ce.
         * @see @ref slice(), @ref prefix(), @ref suffix(),
         *      @ref exceptPrefix(), @ref exceptSuffix(),
         *      @ref sliceSize(std::size_t, std::size_t) const
         */
        #ifdef DOXYGEN_GENERATING_OUTPUT
        constexpr BasicStringView<T> sliceSize(T* begin, std::size_t size) const;
        #else
        /* To avoid ambiguity when calling sliceSize(0, ...). FFS, zero as null
           pointer was deprecated in C++11 already, why is this still a
           problem?! */
        template<class U, typename std::enable_if<std::is_convertible<U, T*>::value && !std::is_convertible<U, std::size_t>::value, int>::type = 0> constexpr BasicStringView<T> sliceSize(U begin, std::size_t size) const {
            return slice(begin, begin + size);
        }
        #endif

        /** @overload */
        constexpr BasicStringView<T> sliceSize(std::size_t begin, std::size_t size) const {
            return slice(begin, begin + size);
        }

        /**
         * @brief View prefix until a pointer
         *
         * Equivalent to @cpp string.slice(string.begin(), end) @ce. If @p end
         * is @cpp nullptr @ce, returns zero-sized @cpp nullptr @ce view.
         * @see @ref slice(T*, T*) const, @ref sliceSize(T*, std::size_t) const,
         *      @ref suffix(T*) const, @ref prefix(std::size_t) const
         */
        #ifdef DOXYGEN_GENERATING_OUTPUT
        constexpr BasicStringView<T> prefix(T* end) const;
        #else
        /* To avoid ambiguity when calling prefix(0). FFS, zero as null pointer
           was deprecated in C++11 already, why is this still a problem?! */
        template<class U, typename std::enable_if<std::is_convertible<U, T*>::value && !std::is_convertible<U, std::size_t>::value, int>::type = 0> constexpr BasicStringView<T> prefix(U end) const {
            return static_cast<T*>(end) ? slice(_data, end) : BasicStringView<T>{};
        }
        #endif

        /**
         * @brief View suffix after a pointer
         *
         * Equivalent to @cpp string.slice(begin, string.end()) @ce. If
         * @p begin is @cpp nullptr @ce and the original view isn't, returns a
         * zero-sized @cpp nullptr @ce view.
         * @see @ref slice(T*, T*) const, @ref sliceSize(T*, std::size_t) const,
         *      @ref prefix(T*) const
         * @todoc link to suffix(std::size_t) once it takes size and not begin
         * @todo once non-deprecated suffix(std::size_t size) is a thing, add
         *      the ambiguity-preventing template here as well
         */
        constexpr BasicStringView<T> suffix(T* begin) const {
            return _data && !begin ? BasicStringView<T>{} : slice(begin, _data + (_sizePlusFlags & ~Implementation::StringViewSizeMask));
        }

        /**
         * @brief View on the first @p size bytes
         *
         * Equivalent to @cpp string.slice(0, size) @ce.
         * @see @ref slice(std::size_t, std::size_t) const,
         *      @ref sliceSize(std::size_t, std::size_t) const,
         *      @ref exceptPrefix(), @ref prefix(T*) const
         * @todoc link to suffix(std::size_t) once it takes size and not begin
         */
        constexpr BasicStringView<T> prefix(std::size_t size) const {
            return slice(0, size);
        }

        /* Here will be suffix(std::size_t size), view on the last size
           bytes, once the deprecated suffix(std::size_t begin) is gone and
           enough time passes to not cause silent breakages in existing code. */

        /**
         * @brief View except the first @p size bytes
         *
         * Equivalent to @cpp string.slice(size, string.size()) @ce.
         * @see @ref slice(std::size_t, std::size_t) const,
         *      @ref sliceSize(std::size_t, std::size_t) const,
         *      @ref prefix(std::size_t) const,
         *      @ref exceptSuffix(std::size_t) const,
         *      @ref exceptPrefix(StringView) const
         */
        constexpr BasicStringView<T> exceptPrefix(std::size_t size) const {
            return slice(size, _sizePlusFlags & ~Implementation::StringViewSizeMask);
        }

        #ifdef CORRADE_BUILD_DEPRECATED
        /** @copybrief exceptPrefix()
         * @m_deprecated_since_latest Use @ref exceptPrefix() instead.
         */
        CORRADE_DEPRECATED("use exceptPrefix() instead") constexpr BasicStringView<T> suffix(std::size_t begin) const {
            return slice(begin, _sizePlusFlags & ~Implementation::StringViewSizeMask);
        }
        #endif

        /**
         * @brief View except the last @p size bytes
         *
         * Equivalent to @cpp string.slice(0, string.size() - size) @ce.
         * @see @ref slice(std::size_t, std::size_t) const,
         *      @ref sliceSize(std::size_t, std::size_t) const,
         *      @ref exceptPrefix(std::size_t) const,
         *      @ref exceptSuffix(StringView) const
         * @todoc link to suffix(std::size_t) once it takes size and not begin
         */
        constexpr BasicStringView<T> exceptSuffix(std::size_t size) const {
            return slice(0, (_sizePlusFlags & ~Implementation::StringViewSizeMask) - size);
        }

        #ifdef CORRADE_BUILD_DEPRECATED
        /**
         * @copybrief exceptSuffix()
         * @m_deprecated_since_latest Use @ref exceptSuffix() instead.
         */
        CORRADE_DEPRECATED("use exceptSuffix() instead") constexpr BasicStringView<T> except(std::size_t count) const {
            return slice(0, (_sizePlusFlags & ~Implementation::StringViewSizeMask) - count);
        }
        #endif

        #ifndef CORRADE_SINGLES_NO_ADVANCED_STRING_APIS
        /**
         * @brief Split on given character
         *
         * If @p delimiter is not found, returns a single-item array containing
         * the full input string. If the string is empty, returns an empty
         * array. The function uses @ref slice() internally, meaning it
         * propagates the @ref flags() as appropriate.
         * @see @ref splitWithoutEmptyParts(), @ref partition(char) const
         */
        Array<BasicStringView<T>> split(char delimiter) const;

        /**
         * @brief Split on given substring
         *
         * If @p delimiter is not found, returns a single-item array containing
         * the full input string. If the string is empty, returns an empty
         * array. The function uses @ref slice() internally, meaning it
         * propagates the @ref flags() as appropriate.
         *
         * Note that this function looks for the whole delimiter. If you want
         * to split on any character from a set, use
         * @ref splitOnAnyWithoutEmptyParts() instead.
         * @see @ref partition(StringView) const
         */
        Array<BasicStringView<T>> split(StringView delimiter) const;

        /**
         * @brief Split on given character, removing empty parts
         *
         * If @p delimiter is not found, returns a single-item array containing
         * the full input string. If the string is empty or consists just of
         * @p delimiter characters, returns an empty array. The function uses
         * @ref slice() internally, meaning it propagates the @ref flags() as
         * appropriate.
         * @see @ref split(), @ref splitOnAnyWithoutEmptyParts(StringView) const,
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
         *
         * If you have just a single delimiter character,
         * @ref split(char) const is more efficient. If you need to split on a
         * multi-character delimiter, use @ref split(StringView) const instead.
         * @see @ref splitOnWhitespaceWithoutEmptyParts() const
         */
        Array<BasicStringView<T>> splitOnAnyWithoutEmptyParts(StringView delimiters) const;

        /** @todo once the deprecated splitWithoutEmptyParts(StringView) is
            removed and enough time passes, reintroduce it and make it work
            similarly to split(StringView), i.e. taking the delimiter as a
            whole */

        #ifdef CORRADE_BUILD_DEPRECATED
        /** @brief @copybrief splitOnAnyWithoutEmptyParts()
         * @m_deprecated_since_latest Use @ref splitOnAnyWithoutEmptyParts()
         *      instead.
         */
        CORRADE_DEPRECATED("use splitOnAnyWithoutEmptyParts() instead") Array<BasicStringView<T>> splitWithoutEmptyParts(StringView delimiters) const;
        #endif

        /**
         * @brief Split on whitespace, removing empty parts
         *
         * Equivalent to calling @ref splitOnAnyWithoutEmptyParts(StringView) const
         * with @cpp " \t\f\v\r\n" @ce passed to @p delimiters.
         */
        Array<BasicStringView<T>> splitOnWhitespaceWithoutEmptyParts() const;

        #ifdef CORRADE_BUILD_DEPRECATED
        /** @brief @copybrief splitOnWhitespaceWithoutEmptyParts()
         * @m_deprecated_since_latest Use @ref splitOnWhitespaceWithoutEmptyParts()
         *      instead.
         */
        CORRADE_DEPRECATED("use splitOnWhitespaceWithoutEmptyParts() instead") Array<BasicStringView<T>> splitWithoutEmptyParts() const;
        #endif

        /**
         * @brief Partition on a character
         *
         * Equivalent to Python's @m_class{m-doc-external} [str.partition()](https://docs.python.org/3/library/stdtypes.html#str.partition).
         * Splits @p string at the first occurrence of @p separator. First
         * returned value is the part before the separator, second the
         * separator, third a part after the separator. If the separator is not
         * found, returns the input string followed by two empty strings.
         *
         * The function uses @ref slice() internally, meaning it propagates the
         * @ref flags() as appropriate. Additionally, the resulting views are
         * @cpp nullptr @ce only if the input is @cpp nullptr @ce, otherwise
         * the view always points to existing memory.
         * @see @ref partitionLast(char) const, @ref split(char) const
         */
        Array3<BasicStringView<T>> partition(char separator) const;

        /**
         * @brief Partition on a substring
         *
         * Like @ref partition(char) const, but looks for a whole substring
         * instead of a single character.
         * @see @ref partitionLast(StringView) const,
         *      @ref split(StringView) const
         */
        Array3<BasicStringView<T>> partition(StringView separator) const;

        /**
         * @brief Partition on a last occurence of a character
         *
         * Equivalent to Python's @m_class{m-doc-external} [str.rpartition()](https://docs.python.org/3/library/stdtypes.html#str.rpartition).
         * Splits @p string at the last occurrence of @p separator. First
         * returned value is the part before the separator, second the
         * separator, third a part after the separator. If the separator is not
         * found, returns two empty strings followed by the input string.
         *
         * The function uses @ref slice() internally, meaning it propagates the
         * @ref flags() as appropriate. Additionally, the resulting views are
         * @cpp nullptr @ce only if the input is @cpp nullptr @ce, otherwise
         * the view always points to existing memory.
         * @see @ref partition(char) const, @ref split(char) const
         */
        Array3<BasicStringView<T>> partitionLast(char separator) const;

        /**
         * @brief Partition on a last occurence of a substring
         *
         * Like @ref partitionLast(char) const, but looks for a whole substring
         * instead of a single character.
         * @see @ref partition(StringView) const, @ref split(StringView) const
         */
        Array3<BasicStringView<T>> partitionLast(StringView separator) const;

        /** @todo change these to return a Triple? it's a smaller header */

        /**
         * @brief Join strings with this view as the delimiter
         *
         * Similar in usage to Python's @m_class{m-doc-external} [str.join()](https://docs.python.org/3/library/stdtypes.html#str.join) --- the
         * following produces @cpp "hello, world" @ce:
         *
         * @snippet Containers.cpp StringView-join
         *
         * @see @ref operator+(StringView, StringView),
         *      @ref operator*(StringView, std::size_t)
         */
        String join(const StringIterable& strings) const;

        /**
         * @brief Join strings with this view as the delimiter, skipping empty parts
         *
         * Like @ref join(), but empty items in @p strings are skipped instead
         * of causing multiple repeated delimiters in the output.
         */
        String joinWithoutEmptyParts(const StringIterable& strings) const;
        #endif

        /**
         * @brief Whether the string begins with given prefix
         *
         * For an empty string returns @cpp true @ce only if @p prefix is empty
         * as well.
         * @see @ref exceptPrefix()
         */
        bool hasPrefix(StringView prefix) const;
        bool hasPrefix(char prefix) const; /**< @overload */

        /**
         * @brief Whether the string ends with given suffix
         *
         * For an empty string returns @cpp true @ce only if @p suffix is empty
         * as well.
         * @see @ref exceptSuffix()
         */
        bool hasSuffix(StringView suffix) const;
        bool hasSuffix(char suffix) const; /**< @overload */

        /**
         * @brief View with given prefix stripped
         *
         * Expects that the string actually begins with given prefix. The
         * function uses @ref slice() internally, meaning it propagates the
         * @ref flags() as appropriate. Additionally, the resulting view is
         * @cpp nullptr @ce only if the input is @cpp nullptr @ce, otherwise
         * the view always points to existing memory.
         * @see @ref hasPrefix(), @ref exceptPrefix(std::size_t) const
         */
        BasicStringView<T> exceptPrefix(StringView prefix) const;

        #ifdef DOXYGEN_GENERATING_OUTPUT
        /**
         * @brief Using char literals for prefix stripping is not allowed
         *
         * To avoid accidentally interpreting a @cpp char @ce literal as a size
         * and calling @ref exceptPrefix(std::size_t) const instead, or vice
         * versa, you have to always use a string literal to call this
         * function.
         */
        BasicStringView<T> exceptPrefix(char prefix) const = delete;
        #else
        template<typename std::enable_if<std::is_same<typename std::decay<T>::type, char>::value, int>::type = 0> BasicStringView<T> exceptPrefix(T&& prefix) const = delete;
        #endif

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
         * @see @ref hasSuffix(), @ref exceptSuffix(std::size_t) const
         */
        BasicStringView<T> exceptSuffix(StringView suffix) const;

        #ifdef DOXYGEN_GENERATING_OUTPUT
        /**
         * @brief Using char literals for suffix stripping is not allowed
         *
         * To avoid accidentally interpreting a @cpp char @ce literal as a size
         * and calling @ref exceptSuffix(std::size_t) const instead, or vice
         * versa, you have to always use a string literal to call this
         * function.
         */
        BasicStringView<T> exceptSuffix(char suffix) const = delete;
        #else
        template<typename std::enable_if<std::is_same<typename std::decay<T>::type, char>::value, int>::type = 0> BasicStringView<T> exceptSuffix(T&& suffix) const = delete;
        #endif

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
        BasicStringView<T> trimmed(StringView characters) const {
            return trimmedPrefix(characters).trimmedSuffix(characters);
        }

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
         * appropriate, except in case of a failure, where it always returns no
         * @ref StringViewFlags.
         *
         * Note that the function operates with a @f$ \mathcal{O}(nm) @f$
         * complexity and as such is meant mainly for one-time searches in
         * non-performance-critical code. For repeated searches or searches of
         * large substrings it's recommended to use the @ref std::search()
         * algorithms, especially @ref std::boyer_moore_searcher and its
         * variants. Those algorithms on the other hand have to perform certain
         * preprocessing of the input and keep extra state and due to that
         * overhead aren't generally suited for one-time searches. Consider
         * using @ref find(char) const instead for single-byte substrings, see
         * also @ref count(char) const for counting the number of occurences.
         *
         * This function is equivalent to calling @relativeref{std::string,find()}
         * on a @ref std::string or a @ref std::string_view.
         * @see @ref contains(), @ref findLast(), @ref findOr(), @ref findAny()
         */
        /* Technically it would be enough to have just one overload with a
           default value for the fail parameter. But then `find(foo, pointer)`
           would imply "find foo after pointer", because that's what the second
           parameter does in most APIs. On the other hand, naming this findOr()
           and documenting the custom failure handling would add extra
           congitive load for people looking for find() and nothing else. */
        BasicStringView<T> find(StringView substring) const {
            return findOr(substring, nullptr);
        }

        /**
         * @brief Find a character
         *
         * Faster than @ref find(StringView) const if the string has just one
         * byte.
         * @see @ref contains(char) const, @ref findOr(char, T*) const,
         *      @ref findLast(char) const, @ref count(char) const
         */
        /* Technically it would be enough to have just one overload with a
           default value for the fail parameter, see above why it's not */
        BasicStringView<T> find(char character) const {
            return findOr(character, nullptr);
        }

        /**
         * @brief Find a substring with a custom failure pointer
         *
         * Like @ref find(StringView) const, but returns an empty view pointing
         * to the @p fail value instead of @cpp nullptr @ce, which is useful to
         * avoid explicit handling of cases where the substring wasn't found.
         *
         * The @p fail value can be @cpp nullptr @ce or any other pointer, but
         * commonly it's set to either @ref begin() or @ref end(). For example
         * here when getting the basename and an extension from a file path,
         * similarly to what @ref Utility::Path::split() and
         * @relativeref{Utility::Path,splitExtension()} does:
         *
         * @snippet Containers.cpp StringView-findOr
         *
         * Consider using @ref findOr(char, T*) const for single-byte
         * substrings.
         * @see @ref findLastOr(), @ref findAnyOr()
         */
        BasicStringView<T> findOr(StringView substring, T* fail) const;

        /**
         * @brief Find a character with a custom failure pointer
         *
         * Faster than @ref findOr(StringView, T*) const if the string has just
         * one byte.
         * @see @ref find(char) const, @ref findLastOr(char, T*) const
         */
        BasicStringView<T> findOr(char character, T* fail) const;

        /**
         * @brief Find the last occurence of a substring
         *
         * Returns a view pointing to the last found substring. If not found,
         * an empty @cpp nullptr @ce view is returned. The function uses
         * @ref slice() internally, meaning it propagates the @ref flags() as
         * appropriate, except in case of a failure, where it always returns no
         * @ref StringViewFlags.
         *
         * Similarly as with @ref find(), note that the function operates with
         * a @f$ \mathcal{O}(nm) @f$ complexity and as such is meant mainly for
         * one-time searches in non-performance-critical code. See the
         * documentation of @ref find() for further information and suggested
         * alternatives. Consider using @ref findLast(char) const instead for
         * single-byte substrings.
         *
         * This function is equivalent to calling @relativeref{std::string,rfind()}
         * on a @ref std::string or a @ref std::string_view.
         * @m_keywords{rfind()}
         * @see @ref findLastOr(), @ref findLastAny()
         */
        /* Technically it would be enough to have just one overload with a
           default value for the fail parameter, see above why it's not */
        BasicStringView<T> findLast(StringView substring) const {
            return findLastOr(substring, nullptr);
        }

        /**
         * @brief Find the last occurence of a character
         *
         * Faster than @ref findLast(StringView) const if the string has just
         * one byte.
         * @see @ref find(char) const, @ref findLastOr(char, T*) const
         */
        /* Technically it would be enough to have just one overload with a
           default value for the fail parameter, see above why it's not */
        BasicStringView<T> findLast(char character) const {
            return findLastOr(character, nullptr);
        }

        /**
         * @brief Find the last occurence a substring with a custom failure pointer
         *
         * Like @ref findLast(StringView) const, but returns an empty view
         * pointing to the @p fail value instead of @cpp nullptr @ce, which is
         * useful to avoid explicit handling of cases where the substring
         * wasn't found. See @ref findOr() for an example use case.
         * @see @ref findLastAnyOr()
         */
        BasicStringView<T> findLastOr(StringView substring, T* fail) const;

        /**
         * @brief Find the last occurence of a character with a custom failure pointer
         *
         * Faster than @ref findLastOr(StringView, T*) const if the string has
         * just one byte.
         * @see @ref findLast(char) const, @ref findOr(char, T*) const
         */
        BasicStringView<T> findLastOr(char character, T* fail) const;

        /**
         * @brief Whether the view contains a substring
         *
         * A slightly lighter variant of @ref find() useful when you only want
         * to know if a substring was found or not. Consider using
         * @ref contains(char) const for single-byte substrings, see also
         * @ref count(char) const for counting the number of occurences.
         * @see @ref containsAny()
         */
        bool contains(StringView substring) const;

        /**
         * @brief Whether the view contains a character
         *
         * Faster than @ref contains(StringView) const if the string has just
         * one byte.
         * @see @ref count(char) const
         */
        bool contains(char character) const;

        /**
         * @brief Find any character from given set
         *
         * Returns a view pointing to the first found character from the set.
         * If no characters from @p characters are found, an empty
         * @cpp nullptr @ce view is returned. The function uses @ref slice()
         * internally, meaning it propagates the @ref flags() as appropriate,
         * except in case of a failure, where it always returns no
         * @ref StringViewFlags.
         *
         * This function is equivalent to calling @relativeref{std::string,find_first_of()}
         * on a @ref std::string or a @ref std::string_view.
         * @m_keywords{find_first_of()}
         * @see @ref containsAny(), @ref findLastAny(), @ref findAnyOr(),
         *      @ref find()
         */
        BasicStringView<T> findAny(StringView characters) const {
            return findAnyOr(characters, nullptr);
        }

        /**
         * @brief Find any character from given set with a custom failure pointer
         *
         * Like @ref findAny(StringView) const, but returns an empty view
         * pointing to the @p fail value instead of @cpp nullptr @ce, which is
         * useful to avoid explicit handling of cases where no character was
         * found.
         *
         * The @p fail value can be @cpp nullptr @ce or any other pointer, but
         * commonly it's set to either @ref begin() or @ref end(). For example
         * here when getting everything until the next space character:
         *
         * @snippet Containers.cpp StringView-findAnyOr
         *
         * @see @ref findLastAnyOr(), @ref findOr()
         */
        BasicStringView<T> findAnyOr(StringView characters, T* fail) const;

        /**
         * @brief Find the last occurence of any character from given set
         *
         * Returns a view pointing to the last found character from the set.
         * If no characters from @p characters are found, an empty
         * @cpp nullptr @ce view is returned. The function uses
         * @ref slice() internally, meaning it propagates the @ref flags() as
         * appropriate, except in case of a failure, where it always returns no
         * @ref StringViewFlags.
         *
         * This function is equivalent to calling @relativeref{std::string,find_last_of()}
         * on a @ref std::string or a @ref std::string_view.
         * @m_keywords{find_last_of()}
         * @see @ref findLastAnyOr(), @ref findLast()
         */
        BasicStringView<T> findLastAny(StringView characters) const {
            return findLastAnyOr(characters, nullptr);
        }

        /**
         * @brief Find the last occurence of any character from given set with a custom failure pointer
         *
         * Like @ref findLastAny(StringView) const, but returns an empty view
         * pointing to the @p fail value instead of @cpp nullptr @ce, which is
         * useful to avoid explicit handling of cases where the substring
         * wasn't found. See @ref findAnyOr() for an example use case.
         * @see @ref findLastOr()
         */
        BasicStringView<T> findLastAnyOr(StringView characters, T* fail) const;

        /**
         * @brief Whether the view contains any character from given set
         *
         * A slightly lighter variant of @ref findAny() useful when you only
         * want to know if a character was found or not.
         * @see @ref contains()
         */
        bool containsAny(StringView substring) const;

        /**
         * @brief Count of occurences of given character
         *
         * If it's only needed to know whether a character is contained in a
         * string at all, consider using @ref contains(char) const instead.
         * @see @ref find(char) const
         */
        std::size_t count(char character) const;

    private:
        /* Needed for mutable/immutable conversion */
        template<class> friend class BasicStringView;
        friend String;
        #ifndef CORRADE_SINGLES_NO_ARRAYTUPLE_COMPATIBILITY
        /* So ArrayTuple can update the data pointer */
        friend T*& Implementation::dataRef<>(BasicStringView<T>&);
        #endif

        /* MSVC demands the export macro to be here as well */
        friend CORRADE_UTILITY_EXPORT bool operator==(StringView, StringView);
        friend CORRADE_UTILITY_EXPORT bool operator!=(StringView, StringView);
        friend CORRADE_UTILITY_EXPORT bool operator<(StringView, StringView);
        friend CORRADE_UTILITY_EXPORT bool operator<=(StringView, StringView);
        friend CORRADE_UTILITY_EXPORT bool operator>=(StringView, StringView);
        friend CORRADE_UTILITY_EXPORT bool operator>(StringView, StringView);
        friend CORRADE_UTILITY_EXPORT String operator+(StringView, StringView);
        friend CORRADE_UTILITY_EXPORT String operator*(StringView, std::size_t);

        #ifndef CORRADE_SINGLES_NO_ADVANCED_STRING_APIS
        /* Called from BasicStringView(U&&, StringViewFlags), see its comment
           for details; arguments in a flipped order to avoid accidental
           ambiguity. The ArrayView type is a template to avoid having to
           include ArrayView.h. */
        template<class U, typename std::enable_if<std::is_same<T, U>::value, int>::type = 0> constexpr explicit BasicStringView(StringViewFlags flags, ArrayView<U> data) noexcept: BasicStringView{data.data(), data.size(), flags} {}
        #endif

        /* Used by the char* constructor, delinlined because it calls into
           std::strlen() */
        explicit BasicStringView(T* data, StringViewFlags flags, std::nullptr_t) noexcept;

        /* Used by slice() to skip unneeded checks in the public constexpr
           constructor */
        constexpr explicit BasicStringView(T* data, std::size_t sizePlusFlags, std::nullptr_t) noexcept: _data{data}, _sizePlusFlags{sizePlusFlags} {}

        T* _data;
        std::size_t _sizePlusFlags;
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

/**
@brief String concatenation
@m_since_latest

For joining more than one string prefer to use @ref StringView::join() to avoid
needless temporary allocations.
@todo mutable && overloads that reuse the growable string storage instead of
    allocating new, when growable strings are a thing
@see @ref operator*(StringView, std::size_t)
*/
CORRADE_UTILITY_EXPORT String operator+(StringView a, StringView b);

/**
@brief String multiplication
@m_since_latest

Equivalent to string multiplication in Python, returns @p string repeated
@p count times.
@see @ref StringView::join(), @ref operator+(StringView, StringView)
*/
CORRADE_UTILITY_EXPORT String operator*(StringView string, std::size_t count);

/**
 * @overload
 * @m_since_latest
 */
CORRADE_UTILITY_EXPORT String operator*(std::size_t count, StringView string);

/* operator<<(Debug&, StringView) implemented directly in Debug */

/* Unlike STL, where there's e.g. std::literals::string_literals with both
   being inline, here's just the second inline because making both would cause
   the literals to be implicitly available to all code in Containers. Which
   isn't great if there are eventually going to be conflicts. In case of STL
   the expected use case was that literals are available to anybody who does
   `using namespace std;`, that doesn't apply here as most APIs are in
   subnamespaces that *should not* be pulled in via `using` as a whole. */
namespace Literals {
    /** @todoc The inline causes "error: non-const getClassDef() called on
        aliased member. Please report as a bug." on Doxygen 1.8.18, plus the
        fork I have doesn't even mark them as inline in the XML output yet. And
        it also duplicates the literal reference to parent namespace, adding
        extra noise. Revisit once upgrading to a newer version. */
    #ifndef DOXYGEN_GENERATING_OUTPUT
    inline
    #endif
    namespace StringLiterals {

/* According to https://wg21.link/CWG2521, space between "" and literal name is
   deprecated because _Uppercase or __double names could be treated as reserved
   depending on whether the space was present or not, and whitespace is not
   load-bearing in any other contexts. Clang 17+ adds an off-by-default warning
   for this; GCC 4.8 however *requires* the space there, so until GCC 4.8
   support is dropped, we suppress this warning instead of removing the
   space. */
#if defined(CORRADE_TARGET_CLANG) && __clang_major__ >= 17
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-literal-operator"
#endif
/** @relatesalso Corrade::Containers::BasicStringView
@brief String view literal
@m_since_latest

The returned instance has both @ref StringViewFlag::Global and
@ref StringViewFlag::NullTerminated set. See
@ref Containers-BasicStringView-usage for more information.
@m_keywords{_s s}
*/
constexpr StringView operator""_s(const char* data, std::size_t size) {
    /* Using plain bit ops instead of EnumSet to speed up debug builds */
    return StringView{data, size, StringViewFlag(std::size_t(StringViewFlag::Global)|std::size_t(StringViewFlag::NullTerminated))};
}
#if defined(CORRADE_TARGET_CLANG) && __clang_major__ >= 17
#pragma clang diagnostic pop
#endif

}}

template<class T> constexpr T& BasicStringView<T>::operator[](const std::size_t i) const {
    /* Accessing the null terminator is fine, if it's there */
    return CORRADE_CONSTEXPR_DEBUG_ASSERT(i < size() + (flags() & StringViewFlag::NullTerminated ? 1 : 0),
        "Containers::StringView::operator[](): index" << i << "out of range for" << size() << (flags() & StringViewFlag::NullTerminated ? "null-terminated bytes" : "bytes")), _data[i];
}

template<class T> constexpr T& BasicStringView<T>::front() const {
    return CORRADE_CONSTEXPR_DEBUG_ASSERT(size(), "Containers::StringView::front(): view is empty"), _data[0];
}

template<class T> constexpr T& BasicStringView<T>::back() const {
    return CORRADE_CONSTEXPR_DEBUG_ASSERT(size(), "Containers::StringView::back(): view is empty"), _data[size() - 1];
}

template<class T> constexpr BasicStringView<T> BasicStringView<T>::slice(T* const begin, T* const end) const {
    return CORRADE_CONSTEXPR_DEBUG_ASSERT(_data <= begin && begin <= end && end <= _data + (_sizePlusFlags & ~Implementation::StringViewSizeMask),
            "Containers::StringView::slice(): slice ["
            << Utility::Debug::nospace << begin - _data
            << Utility::Debug::nospace << ":"
            << Utility::Debug::nospace << end - _data
            << Utility::Debug::nospace << "] out of range for"
            << (_sizePlusFlags & ~Implementation::StringViewSizeMask) << "elements"),
        BasicStringView<T>{begin, std::size_t(end - begin)|
            /* Propagate the global flag always */
            (_sizePlusFlags & std::size_t(StringViewFlag::Global))|
            /* The null termination flag only if the original is
               null-terminated and end points to the original end */
            ((_sizePlusFlags & std::size_t(StringViewFlag::NullTerminated))*(end == _data + (_sizePlusFlags & ~Implementation::StringViewSizeMask))),
            /* Using an internal assert-less constructor, the public
               constructor asserts would be redundant */
            nullptr};
}

template<class T> constexpr BasicStringView<T> BasicStringView<T>::slice(const std::size_t begin, const std::size_t end) const {
    return CORRADE_CONSTEXPR_DEBUG_ASSERT(begin <= end && end <= (_sizePlusFlags & ~Implementation::StringViewSizeMask),
            "Containers::StringView::slice(): slice ["
            << Utility::Debug::nospace << begin
            << Utility::Debug::nospace << ":"
            << Utility::Debug::nospace << end
            << Utility::Debug::nospace << "] out of range for"
            << (_sizePlusFlags & ~Implementation::StringViewSizeMask) << "elements"),
        BasicStringView<T>{_data + begin, (end - begin)|
            /* Propagate the global flag always */
            (_sizePlusFlags & std::size_t(StringViewFlag::Global))|
            /* The null termination flag only if the original is
               null-terminated and end points to the original end */
            ((_sizePlusFlags & std::size_t(StringViewFlag::NullTerminated))*(end == (_sizePlusFlags & ~Implementation::StringViewSizeMask))),
            /* Using an internal assert-less constructor, the public
               constructor asserts would be redundant */
            nullptr};
}

namespace Implementation {

/* Making naming unique in order to prepare for these being function pointers
   (that can't be overloaded) */
CORRADE_UTILITY_EXPORT const char* stringFindString(const char* data, std::size_t size, const char* substring, std::size_t substringSize);
CORRADE_UTILITY_EXPORT const char* stringFindLastString(const char* data, std::size_t size, const char* substring, std::size_t substringSize);
CORRADE_UTILITY_EXPORT extern const char* CORRADE_UTILITY_CPU_DISPATCHED_DECLARATION(stringFindCharacter)(const char* data, std::size_t size, char character);
CORRADE_UTILITY_CPU_DISPATCHER_DECLARATION(stringFindCharacter)
CORRADE_UTILITY_EXPORT const char* stringFindLastCharacter(const char* data, std::size_t size, char character);
CORRADE_UTILITY_EXPORT const char* stringFindAny(const char* data, std::size_t size, const char* characters, std::size_t characterCount);
CORRADE_UTILITY_EXPORT const char* stringFindLastAny(const char* data, std::size_t size, const char* characters, std::size_t characterCount);
CORRADE_UTILITY_EXPORT const char* stringFindNotAny(const char* data, std::size_t size, const char* characters, std::size_t characterCount);
CORRADE_UTILITY_EXPORT const char* stringFindLastNotAny(const char* data, std::size_t size, const char* characters, std::size_t characterCount);
CORRADE_UTILITY_EXPORT extern std::size_t CORRADE_UTILITY_CPU_DISPATCHED_DECLARATION(stringCountCharacter)(const char* data, std::size_t size, char character);
CORRADE_UTILITY_CPU_DISPATCHER_DECLARATION(stringCountCharacter)

}

template<class T> inline BasicStringView<T> BasicStringView<T>::trimmedPrefix(const StringView characters) const {
    const std::size_t size = this->size();
    T* const found = const_cast<T*>(Implementation::stringFindNotAny(_data, size, characters._data, characters.size()));
    return suffix(found ? found : _data + size);
}

template<class T> inline BasicStringView<T> BasicStringView<T>::trimmedSuffix(const StringView characters) const {
    T* const found = const_cast<T*>(Implementation::stringFindLastNotAny(_data, size(), characters._data, characters.size()));
    return prefix(found ? found + 1 : _data);
}

template<class T> inline BasicStringView<T> BasicStringView<T>::findOr(const StringView substring, T* const fail) const {
    /* Cache the getters to speed up debug builds */
    const std::size_t substringSize = substring.size();
    if(const char* const found = Implementation::stringFindString(_data, size(), substring._data, substringSize))
        return slice(const_cast<T*>(found), const_cast<T*>(found + substringSize));

    /* Using an internal assert-less constructor, the public constructor
       asserts would be redundant. Since it's a zero-sized view, it doesn't
       really make sense to try to preserve any flags. */
    return BasicStringView<T>{fail, 0 /* empty, no flags */, nullptr};
}

template<class T> inline BasicStringView<T> BasicStringView<T>::findOr(const char character, T* const fail) const {
    if(const char* const found = Implementation::stringFindCharacter(_data, size(), character))
        return slice(const_cast<T*>(found), const_cast<T*>(found + 1));

    /* Using an internal assert-less constructor, the public constructor
       asserts would be redundant. Since it's a zero-sized view, it doesn't
       really make sense to try to preserve any flags. */
    return BasicStringView<T>{fail, 0 /* empty, no flags */, nullptr};
}

template<class T> inline BasicStringView<T> BasicStringView<T>::findLastOr(const StringView substring, T* const fail) const {
    /* Cache the getters to speed up debug builds */
    const std::size_t substringSize = substring.size();
    if(const char* const found = Implementation::stringFindLastString(_data, size(), substring._data, substringSize))
        return slice(const_cast<T*>(found), const_cast<T*>(found + substringSize));

    /* Using an internal assert-less constructor, the public constructor
       asserts would be redundant. Since it's a zero-sized view, it doesn't
       really make sense to try to preserve any flags. */
    return BasicStringView<T>{fail, 0 /* empty, no flags */, nullptr};
}

template<class T> inline BasicStringView<T> BasicStringView<T>::findLastOr(const char character, T* const fail) const {
    if(const char* const found = Implementation::stringFindLastCharacter(_data, size(), character))
        return slice(const_cast<T*>(found), const_cast<T*>(found + 1));

    /* Using an internal assert-less constructor, the public constructor
       asserts would be redundant. Since it's a zero-sized view, it doesn't
       really make sense to try to preserve any flags. */
    return BasicStringView<T>{fail, 0 /* empty, no flags */, nullptr};
}

template<class T> inline bool BasicStringView<T>::contains(const StringView substring) const {
    return Implementation::stringFindString(_data, size(), substring._data, substring.size());
}

template<class T> inline bool BasicStringView<T>::contains(const char character) const {
    return Implementation::stringFindCharacter(_data, size(), character);
}

template<class T> inline BasicStringView<T> BasicStringView<T>::findAnyOr(const StringView characters, T* const fail) const {
    if(const char* const found = Implementation::stringFindAny(_data, size(), characters._data, characters.size()))
        return slice(const_cast<T*>(found), const_cast<T*>(found + 1));

    /* Using an internal assert-less constructor, the public constructor
       asserts would be redundant. Since it's a zero-sized view, it doesn't
       really make sense to try to preserve any flags. */
    return BasicStringView<T>{fail, 0 /* empty, no flags */, nullptr};
}

template<class T> inline BasicStringView<T> BasicStringView<T>::findLastAnyOr(const StringView characters, T* const fail) const {
    if(const char* const found = Implementation::stringFindLastAny(_data, size(), characters._data, characters.size()))
        return slice(const_cast<T*>(found), const_cast<T*>(found + 1));

    /* Using an internal assert-less constructor, the public constructor
       asserts would be redundant. Since it's a zero-sized view, it doesn't
       really make sense to try to preserve any flags. */
    return BasicStringView<T>{fail, 0 /* empty, no flags */, nullptr};
}

template<class T> inline bool BasicStringView<T>::containsAny(const StringView characters) const {
    return Implementation::stringFindAny(_data, size(), characters._data, characters.size());
}

template<class T> inline std::size_t BasicStringView<T>::count(const char character) const {
    return Implementation::stringCountCharacter(_data, size(), character);
}

#ifndef CORRADE_SINGLES_NO_ADVANCED_STRING_APIS
namespace Implementation {

template<class> struct ErasedArrayViewConverter;

/* Strangely enough, if the from() functions don't accept T& but just T, it
   leads to an infinite template recursion depth */
template<> struct ArrayViewConverter<char, BasicStringView<char>> {
    CORRADE_UTILITY_EXPORT static ArrayView<char> from(const BasicStringView<char>& other);
};
template<> struct ArrayViewConverter<const char, BasicStringView<char>> {
    CORRADE_UTILITY_EXPORT static ArrayView<const char> from(const BasicStringView<char>& other);
};
template<> struct ArrayViewConverter<const char, BasicStringView<const char>> {
    CORRADE_UTILITY_EXPORT static ArrayView<const char> from(const BasicStringView<const char>& other);
};
template<class T> struct ErasedArrayViewConverter<BasicStringView<T>>: ArrayViewConverter<T, BasicStringView<T>> {};
template<class T> struct ErasedArrayViewConverter<const BasicStringView<T>>: ArrayViewConverter<T, BasicStringView<T>> {};

}
#endif

}}

#endif
