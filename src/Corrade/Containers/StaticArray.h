#ifndef Corrade_Containers_StaticArray_h
#define Corrade_Containers_StaticArray_h
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
 * @brief Class @ref Corrade::Containers::StaticArray, alias @ref Corrade::Containers::Array2, @ref Corrade::Containers::Array3, @ref Corrade::Containers::Array4
 */

#include <new>
/* std::declval() is said to be in <utility> but libstdc++, libc++ and MSVC STL
   all have it directly in <type_traits> because it just makes sense */
#include <type_traits>

#include "Corrade/Tags.h"
#include "Corrade/Containers/ArrayView.h"
#include "Corrade/Containers/constructHelpers.h"
#include "Corrade/Containers/sequenceHelpers.h"
#include "Corrade/Utility/Macros.h" /* CORRADE_CONSTEXPR14 */

namespace Corrade { namespace Containers {

namespace Implementation {

template<std::size_t size_, class T, bool trivial> struct StaticArrayData;
template<std::size_t size_, class T> struct StaticArrayData<size_, T, true> {
    /* Here we additionally deal with types that have a NoInit constructor */
    template<class U = T, typename std::enable_if<!std::is_constructible<U, Corrade::NoInitT>::value, int>::type = 0> explicit StaticArrayData(Corrade::NoInitT) {}
    template<class U = T, typename std::enable_if<std::is_constructible<U, Corrade::NoInitT>::value, int>::type = 0> explicit StaticArrayData(Corrade::NoInitT): StaticArrayData{Corrade::NoInit, typename GenerateSequence<size_>::Type{}} {}
    template<std::size_t... sequence, class U = T, typename std::enable_if<std::is_constructible<U, Corrade::NoInitT>::value, int>::type = 0> explicit StaticArrayData(Corrade::NoInitT noInit, Sequence<sequence...>): _data{T{(&noInit)[0*sequence]}...} {}

    /* Compared to StaticArrayData<size_, T, false> it does the right thing by
       default. MSVC 2015, 2019 and 2022 (but not 2017, _MSC_VER=191x)
       complains that the constexpr constructor doesn't initialize all members
       when used with a non-trivially-constructible type, so there has to be a
       non-constexpr variant that doesn't initialize trivially constructible
       types (and MSVC 2015 would complain if it would be constexpr), and a
       constexpr variant that initializes types with a constructor. Not fixable
       with /permissive- either. */
    #if !defined(CORRADE_TARGET_MSVC) || defined(CORRADE_TARGET_CLANG) || (_MSC_VER >= 1910 && _MSC_VER < 1920)
    constexpr explicit StaticArrayData(Corrade::DefaultInitT) {}
    #else
    template<class U = T, typename std::enable_if<std::is_trivially_constructible<U>::value, int>::type = 0> explicit StaticArrayData(Corrade::DefaultInitT) {}
    template<class U = T, typename std::enable_if<!std::is_trivially_constructible<U>::value, int>::type = 0> constexpr explicit StaticArrayData(Corrade::DefaultInitT): _data{} {}
    #endif

    /* Compared to StaticArrayData<size_, T, false>, there's no way to trigger
       the featurebug in C++ where new T{} doesn't work for an explicit
       defaulted constructor in trivially constructible or NoInit-constructible
       types, so this uses a {}. For details see constructHelpers.h and
       StaticArrayTest::constructorExplicitInCopyInitialization(). */
    constexpr explicit StaticArrayData(Corrade::ValueInitT): _data{} {}

    /* Same as in StaticArrayData<size_, T, false> */
    template<class ...Args> constexpr explicit StaticArrayData(Corrade::InPlaceInitT, Args&&... args): _data{Utility::forward<Args>(args)...} {}
    template<std::size_t ...sequence> constexpr explicit StaticArrayData(Corrade::InPlaceInitT, Sequence<sequence...>, const T(&data)[sizeof...(sequence)]): _data{data[sequence]...} {}
    /* See StaticArrayTest::constructArrayMove() for details why it has to be
       disabled */
    #ifndef CORRADE_MSVC2017_COMPATIBILITY
    template<std::size_t ...sequence> constexpr explicit StaticArrayData(Corrade::InPlaceInitT, Sequence<sequence...>, T(&&data)[sizeof...(sequence)]): _data{Utility::move(data[sequence])...} {}
    #endif

    T _data[size_];
};

template<std::size_t size_, class T> struct StaticArrayData<size_, T, false> {
    /* Compared to StaticArrayData<size_, T, true> it does the right thing by
       default */
    explicit StaticArrayData(Corrade::NoInitT) {}

    /* Compared to StaticArrayData<size_, T, true> a default constructor has to
       be called on the union members. If the default constructor is trivial,
       the StaticArrayData<size_, T, true> base was picked instead.

       The () instead of {} works around a featurebug in C++ where new T{}
       doesn't work for an explicit defaulted constructor. Doesn't apply to
       StaticArrayData<size_, T, true>. For details see constructHelpers.h and
       StaticArrayTest::constructorExplicitInCopyInitialization(). */
    explicit StaticArrayData(Corrade::DefaultInitT): _data() {}
    explicit StaticArrayData(Corrade::ValueInitT): _data() {}

    /* Same as in StaticArrayData<size_, T, true> */
    template<class ...Args> explicit StaticArrayData(Corrade::InPlaceInitT, Args&&... args): _data{Utility::forward<Args>(args)...} {}
    template<std::size_t ...sequence> explicit StaticArrayData(Corrade::InPlaceInitT, Implementation::Sequence<sequence...>, const T(&data)[sizeof...(sequence)]): _data{data[sequence]...} {}
    /* See StaticArrayTest::constructArrayMove() for details why it has to be
       disabled */
    #ifndef CORRADE_MSVC2017_COMPATIBILITY
    template<std::size_t ...sequence> explicit StaticArrayData(Corrade::InPlaceInitT, Implementation::Sequence<sequence...>, T(&&data)[sizeof...(sequence)]): _data{Utility::move(data[sequence])...} {}
    #endif

    /* Compared to StaticArrayData<size_, T, true> we need to explicitly
       copy/move the union members */
    StaticArrayData(const StaticArrayData<size_, T, false>& other) noexcept(std::is_nothrow_copy_constructible<T>::value);
    StaticArrayData(StaticArrayData<size_, T, false>&& other) noexcept(std::is_nothrow_move_constructible<T>::value);
    ~StaticArrayData();
    StaticArrayData<size_, T, false>& operator=(const StaticArrayData<size_, T, false>&) noexcept(std::is_nothrow_copy_constructible<T>::value);
    StaticArrayData<size_, T, false>& operator=(StaticArrayData<size_, T, false>&&) noexcept(std::is_nothrow_move_constructible<T>::value);

    union {
        T _data[size_];
    };
};

template<std::size_t size_, class T> using StaticArrayDataFor = StaticArrayData<size_, T,
    #ifdef CORRADE_NO_STD_IS_TRIVIALLY_TRAITS
    __has_trivial_constructor(T)
    #else
    std::is_trivially_constructible<T>::value
    #endif
    || std::is_constructible<T, Corrade::NoInitT>::value>;

}

/**
@brief Compile-time-sized array
@tparam size_   Array size
@tparam T       Element type

Like @ref Array, but with compile-time size information. Useful as a more
featureful alternative to plain C arrays or @ref std::array, especially when it
comes to initialization. A non-owning version of this container is a
@ref StaticArrayView.

@section Containers-StaticArray-usage Usage

The @ref StaticArray class provides an access and slicing API similar to
@ref Array, which in turn shares the basic workflow patterns with
@ref ArrayView, see @ref Containers-ArrayView-usage "its usage docs" for
details. The main difference is that @ref StaticArray doesn't do any heap
allocation and thus has no concept of a deleter, and it has additional
compile-time-sized overloads of @ref slice(), @ref prefix(), @ref suffix(),
@ref exceptPrefix() and @ref exceptSuffix(), mirroring the APIs of
@ref StaticArrayView.

@snippet Containers.cpp StaticArray-usage

@subsection Containers-StaticArray-usage-initialization Array initialization

The array is by default *value-initialized*, which means that trivial types
are zero-initialized and the default constructor is called on other types. It
is possible to initialize the array in a different way using so-called *tags*:

-   @ref StaticArray(DefaultInitT) leaves trivial types uninitialized
    and calls the default constructor elsewhere. In other words,
    @cpp T array[size] @ce. Because of the differing behavior for trivial types
    it's better to explicitly use either the @ref ValueInit or @ref NoInit
    variants instead.
-   @ref StaticArray(ValueInitT) is equivalent to the implicit parameterless
    constructor, zero-initializing trivial types and calling the default
    constructor elsewhere. Useful when you want to make the choice appear
    explicit. In other words, @cpp T array[size]{} @ce.
-   @ref StaticArray(DirectInitT, Args&&... args) constructs every element of
    the array using provided arguments. In other words,
    @cpp T array[size]{T{args...}, T{args...}, …} @ce.
-   @ref StaticArray(InPlaceInitT, Args&&... args) is equivalent to
    @ref StaticArray(Args&&... args) shown in the example snippet above. Again
    useful when you want to make the choice appear explicit). In other words,
    @cpp T array[size]{args...} @ce. Note that the variadic template means you
    can't use @cpp {} @ce for nested type initializers but have to specify the
    types explicitly. An alternative is directly passing an array, i.e. with
    the items wrapped in an additional @cpp {} @ce, with
    @ref StaticArray(InPlaceInitT, const T(&)[size]) or
    @ref StaticArray(InPlaceInitT, T(&&)[size]), or using the implicit
    @ref StaticArray(const T(&)[size]) and @ref StaticArray(T(&&)[size])
    variants.
-   @ref StaticArray(NoInitT) does not initialize anything. Useful for trivial
    types when you'll be overwriting the contents anyway, for non-trivial types
    this is the dangerous option and you need to call the constructor on all
    elements manually using placement new, @ref std::uninitialized_copy() or
    similar --- see the constructor docs for an example.

@snippet Containers.cpp StaticArray-usage-initialization

@subsection Containers-StaticArray-constexpr Usage in constexpr contexts

In order to implement the @ref StaticArray(NoInitT) constructor for arbitrary
types, internally the data has to be a @cpp union @ce array. That would however
lose trivial copyability and possibility to use the type in @cpp constexpr @ce
contexts, so to solve that, types that have a trivial default constructor or a
constructor taking @ref Corrade::NoInitT use a specialized internal
representation without a @cpp union @ce. @ref StaticArray of such types is then
a @cpp constexpr @ce type and is trivially copyable if the underlying type is
trivially copyable.

@section Containers-StaticArray-views Conversion to array views

Arrays are implicitly convertible to @ref ArrayView / @ref StaticArrayView as
described in the following table. The conversion is only allowed if @cpp T* @ce
is implicitly convertible to @cpp U* @ce (or both are the same type) and both
have the same size. This also extends to other container types constructibe
from @ref ArrayView / @ref StaticArrayView, which means for example that a
@ref StridedArrayView1D is implicitly convertible from @ref StaticArray as
well.

Owning array type               | ↭ | Non-owning view type
------------------------------- | - | ---------------------
@ref StaticArray "Array<size, T>" | → | @ref StaticArrayView "ArrayView<size, U>"
@ref StaticArray "Array<size, T>" | → | @ref StaticArrayView "ArrayView<size, const U>"
@ref StaticArray "const Array<size, T>" | → | @ref StaticArrayView "ArrayView<size, const U>"
@ref StaticArray "Array<size, T>" | → | @ref ArrayView "ArrayView&lt;U&gt;"
@ref StaticArray "Array<size, T>" | → | @ref ArrayView "ArrayView<const U>"
@ref StaticArray "const Array<size, T>" | → | @ref ArrayView "ArrayView<const U>"

@section Containers-StaticArray-structured-bindings C++17 structured bindings

If @ref Corrade/Containers/StructuredBindings.h is included, the class can be
used in C++17 structured bindings. While the @cpp get<i>() @ce overloads are
defined inside @ref StaticArray itself, a separate header is used for the
@m_class{m-doc-external} [std::tuple_size](https://en.cppreference.com/w/cpp/utility/tuple_size)
and @m_class{m-doc-external} [std::tuple_element](https://en.cppreference.com/w/cpp/utility/tuple_element)
template specializations, as those may require @cpp #include <utility> @ce on
some STL implementations. Example:

@snippet Containers-cpp17.cpp StaticArray-structured-bindings

@section Containers-StaticArray-stl STL compatibility

On compilers that support C++2a and @ref std::span, implicit conversion of an
@ref StaticArray to it is provided in @ref Corrade/Containers/ArrayViewStlSpan.h.
The conversion is provided in a separate header to avoid unconditional
@cpp #include <span> @ce, which significantly affects compile times. The
following table lists allowed conversions:

Corrade type                    | ↭ | STL type
------------------------------- | - | ---------------------
@ref StaticArray "StaticArray<size, T>" | → | @ref std::span "std::span<T, size>"
@ref StaticArray "StaticArray<size, T>" | → | @ref std::span "std::span<const T, size>"
@ref StaticArray "const StaticArray<size, T>" | → | @ref std::span "std::span<const T, size>"

@m_class{m-block m-success}

@par Single-header version
    This class, together with @ref Array, is also available as a single-header
    [CorradeArray.h](https://github.com/mosra/magnum-singles/tree/master/CorradeArray.h)
    library in the Magnum Singles repository for easier integration into your
    projects. See @ref corrade-singles for more information.

@see @ref Array2, @ref Array3, @ref Array4
*/
/* Underscore at the end to avoid conflict with member size(). It's ugly, but
   having count instead of size_ would make the naming horribly inconsistent. */
template<std::size_t size_, class T> class StaticArray: Implementation::StaticArrayDataFor<size_, T> {
    /* Ideally this could be derived from StaticArrayView<size_, T>, avoiding a
       lot of redundant code, however I'm unable to find a way to add
       const/non-const overloads of all slicing functions and also prevent
       const StaticArray<size_, T>& from being sliced to a (mutable)
       StaticArrayView<size_, T>. */

    public:
        enum: std::size_t {
            Size = size_    /**< Array size */
        };
        typedef T Type;     /**< @brief Element type */

        /**
         * @brief Construct a default-initialized array
         *
         * Creates array of given size, the contents are default-initialized
         * (i.e., trivial types are not initialized). Because of the differing
         * behavior for trivial types it's better to explicitly use either the
         * @ref StaticArray(ValueInitT) or the @ref StaticArray(NoInitT)
         * variant instead.
         * @see @relativeref{Corrade,DefaultInit},
         *      @ref StaticArray(DirectInitT, Args&&... args),
         *      @ref StaticArray(InPlaceInitT, Args&&... args),
         *      @ref std::is_trivial
         */
        constexpr explicit StaticArray(Corrade::DefaultInitT): Implementation::StaticArrayDataFor<size_, T>{Corrade::DefaultInit} {}

        /**
         * @brief Construct a value-initialized array
         *
         * Creates array of given size, the contents are value-initialized
         * (i.e., trivial types are zero-initialized, default constructor
         * called otherwise). This is the same as @ref StaticArray().
         * @see @relativeref{Corrade,ValueInit},
         *      @ref StaticArray(DefaultInitT),
         *      @ref StaticArray(NoInitT),
         *      @ref StaticArray(DirectInitT, Args&&... args),
         *      @ref StaticArray(InPlaceInitT, Args&&... args),
         *      @ref StaticArray(InPlaceInitT, const T(&)[size]),
         *      @ref StaticArray(InPlaceInitT, T(&&)[size]),
         *      @ref std::is_trivial
         */
        constexpr explicit StaticArray(Corrade::ValueInitT): Implementation::StaticArrayDataFor<size_, T>{Corrade::ValueInit} {}

        /**
         * @brief Construct an array without initializing its contents
         *
         * Creates array of given size, the contents are *not* initialized.
         * Useful if you will be overwriting all elements later anyway or if
         * you need to call custom constructors in a way that's not expressible
         * via any other @ref StaticArray constructor.
         *
         * For trivial types is equivalent to @ref StaticArray(DefaultInitT).
         * For non-trivial types, the class will explicitly call the destructor
         * on *all elements* --- which means that for non-trivial types you're
         * expected to construct all elements using placement new (or for
         * example @ref std::uninitialized_copy()) in order to avoid calling
         * destructors on uninitialized memory:
         *
         * @snippet Containers.cpp StaticArray-NoInit
         *
         * @see @relativeref{Corrade,NoInit},
         *      @ref StaticArray(ValueInitT),
         *      @ref StaticArray(DirectInitT, Args&&... args),
         *      @ref StaticArray(InPlaceInitT, Args&&... args),
         *      @ref StaticArray(InPlaceInitT, const T(&)[size]),
         *      @ref StaticArray(InPlaceInitT, T(&&)[size]),
         *      @ref std::is_trivial
         */
        explicit StaticArray(Corrade::NoInitT): Implementation::StaticArrayDataFor<size_, T>{Corrade::NoInit} {}

        /**
         * @brief Construct a direct-initialized array
         *
         * Constructs the array using the @ref StaticArray(NoInitT) constructor
         * and then initializes each element with placement new using forwarded
         * @p args.
         * @see @ref StaticArray(InPlaceInitT, Args&&... args),
         *      @ref StaticArray(InPlaceInitT, const T(&)[size]),
         *      @ref StaticArray(InPlaceInitT, T(&&)[size])
         */
        /* Not constexpr as it delegates to a NoInit constructor */
        template<class ...Args> explicit StaticArray(Corrade::DirectInitT, Args&&... args);

        /**
         * @brief Construct an in-place-initialized array
         *
         * The arguments are forwarded to the array constructor. Note that the
         * variadic template means you can't use @cpp {} @ce for nested type
         * initializers --- see @ref StaticArray(InPlaceInitT, const T(&)[size])
         * or @ref StaticArray(InPlaceInitT, T(&&)[size]) for an
         * alternative. Same as @ref StaticArray(Args&&... args).
         *
         * To prevent accidents, compared to regular C array initialization the
         * constructor expects the number of arguments to match the size
         * *exactly*. I.e., it's not possible to omit a suffix of the array to
         * implicitly value-initialize it.
         * @see @ref StaticArray(DirectInitT, Args&&... args)
         */
        template<class ...Args> constexpr explicit StaticArray(Corrade::InPlaceInitT, Args&&... args): Implementation::StaticArrayDataFor<size_, T>{Corrade::InPlaceInit, Utility::forward<Args>(args)...} {
            static_assert(sizeof...(args) == size_, "Containers::StaticArray: wrong number of initializers");
        }

        /**
         * @brief In-place construct an array by copying the elements from a fixed-size array
         * @m_since_latest
         *
         * Compared to @ref StaticArray(InPlaceInitT, Args&&... args) doesn't
         * require the elements to have explicitly specified type. The array
         * elements are copied to the array constructor, if you have a
         * non-copyable type or want to move the elements, use
         * @ref StaticArray(InPlaceInitT, T(&&)[size]) instead. Same as
         * @ref StaticArray(const T(&)[size]).
         *
         * To prevent accidents, compared to regular C array initialization the
         * constructor expects the number of arguments to match the size
         * *exactly*. I.e., it's not possible to omit a suffix of the array to
         * implicitly value-initialize it.
         */
        #if !defined(CORRADE_TARGET_GCC) || defined(CORRADE_TARGET_CLANG) || __GNUC__ >= 5
        template<std::size_t size> constexpr /*implicit*/ StaticArray(Corrade::InPlaceInitT, const T(&data)[size]): Implementation::StaticArrayDataFor<size_, T>{Corrade::InPlaceInit, typename Implementation::GenerateSequence<size>::Type{}, data} {
            static_assert(size == size_, "Containers::StaticArray: wrong number of initializers");
        }
        #else
        /* GCC 4.8 isn't able to figure out the size on its own. Which means
           there we use the type-provided size and lose the check for element
           count, but at least it compiles. */
        constexpr /*implicit*/ StaticArray(Corrade::InPlaceInitT, const T(&data)[size_]): Implementation::StaticArrayDataFor<size_, T>{Corrade::InPlaceInit, typename Implementation::GenerateSequence<size_>::Type{}, data} {}
        #endif

        /* See StaticArrayTest::constructArrayMove() for details why it has to
           be disabled */
        #ifndef CORRADE_MSVC2017_COMPATIBILITY
        /**
         * @brief In-place construct an array by moving the elements from a fixed-size array
         * @m_since_latest
         *
         * Compared to @ref StaticArray(InPlaceInitT, Args&&... args) doesn't
         * require the elements to have an explicitly specified type. Same as
         * @ref StaticArray(T(&&)[size]).
         * @partialsupport Not available on
         *      @ref CORRADE_MSVC2015_COMPATIBILITY "MSVC 2015" and
         *      @ref CORRADE_MSVC2017_COMPATIBILITY "MSVC 2017" as these
         *      compilers don't support moving arrays.
         * @see @ref StaticArray(InPlaceInitT, const T(&)[size])
         */
        #if !defined(CORRADE_TARGET_GCC) || defined(CORRADE_TARGET_CLANG) || __GNUC__ >= 5
        template<std::size_t size> constexpr /*implicit*/ StaticArray(Corrade::InPlaceInitT, T(&&data)[size]): Implementation::StaticArrayDataFor<size_, T>{Corrade::InPlaceInit, typename Implementation::GenerateSequence<size>::Type{}, Utility::move(data)} {
            static_assert(size == size_, "Containers::StaticArray: wrong number of initializers");
        }
        #else
        /* GCC 4.8 isn't able to figure out the size on its own. Which means
           there we use the type-provided size and lose the check for element
           count, but at least it compiles. */
        constexpr /*implicit*/ StaticArray(Corrade::InPlaceInitT, T(&&data)[size_]): Implementation::StaticArrayDataFor<size_, T>{Corrade::InPlaceInit, typename Implementation::GenerateSequence<size_>::Type{}, Utility::move(data)} {}
        #endif
        #endif

        /**
         * @brief Construct a value-initialized array
         *
         * Alias to @ref StaticArray(ValueInitT).
         * @see @ref StaticArray(DefaultInitT)
         */
        constexpr /*implicit*/ StaticArray(): Implementation::StaticArrayDataFor<size_, T>{Corrade::ValueInit} {}

        /**
         * @brief Construct an in-place-initialized array
         *
         * Alias to @ref StaticArray(InPlaceInitT, Args&&... args).
         * @see @ref StaticArray(DirectInitT, Args&&... args)
         */
        #ifdef DOXYGEN_GENERATING_OUTPUT
        template<class ...Args> constexpr /*implicit*/ StaticArray(Args&&... args);
        #else
        template<class First, class ...Next, typename std::enable_if<std::is_convertible<First&&, T>::value, int>::type = 0> constexpr /*implicit*/ StaticArray(First&& first, Next&&... next): Implementation::StaticArrayDataFor<size_, T>{Corrade::InPlaceInit, Utility::forward<First>(first), Utility::forward<Next>(next)...} {
            static_assert(sizeof...(next) + 1 == size_, "Containers::StaticArray: wrong number of initializers");
        }
        #endif

        /**
         * @brief In-place construct an array by copying the elements from a fixed-size array
         * @m_since_latest
         *
         * Alias to @ref StaticArray(InPlaceInitT, const T(&)[size]).
         * @see @ref StaticArray(T(&&)[size]),
         *      @ref StaticArray(InPlaceInitT, Args&&... args)
         */
        #if !defined(CORRADE_TARGET_GCC) || defined(CORRADE_TARGET_CLANG) || __GNUC__ >= 5
        template<std::size_t size> constexpr /*implicit*/ StaticArray(const T(&data)[size]): Implementation::StaticArrayDataFor<size_, T>{Corrade::InPlaceInit, typename Implementation::GenerateSequence<size>::Type{}, data} {
            static_assert(size == size_, "Containers::StaticArray: wrong number of initializers");
        }
        #else
        /* GCC 4.8 isn't able to figure out the size on its own. Which means
           there we use the type-provided size and lose the check for element
           count, but at least it compiles. */
        constexpr /*implicit*/ StaticArray(const T(&data)[size_]): Implementation::StaticArrayDataFor<size_, T>{Corrade::InPlaceInit, typename Implementation::GenerateSequence<size_>::Type{}, data} {}
        #endif

        /* See StaticArrayTest::constructArrayMove() for details why it has to
           be disabled */
        #ifndef CORRADE_MSVC2017_COMPATIBILITY
        /**
         * @brief In-place construct an array by moving the elements from a fixed-size array
         * @m_since_latest
         *
         * Alias to @ref StaticArray(InPlaceInitT, T(&&)[size]).
         * @partialsupport Not available on
         *      @ref CORRADE_MSVC2015_COMPATIBILITY "MSVC 2015" and
         *      @ref CORRADE_MSVC2017_COMPATIBILITY "MSVC 2017" as these
         *      compilers don't support moving arrays.
         * @see @ref StaticArray(const T(&)[size]),
         *      @ref StaticArray(InPlaceInitT, Args&&... args)
         */
        #if !defined(CORRADE_TARGET_GCC) || defined(CORRADE_TARGET_CLANG) || __GNUC__ >= 5
        template<std::size_t size> constexpr /*implicit*/ StaticArray(T(&&data)[size]): Implementation::StaticArrayDataFor<size_, T>{Corrade::InPlaceInit, typename Implementation::GenerateSequence<size>::Type{}, Utility::move(data)} {
            static_assert(size == size_, "Containers::StaticArray: wrong number of initializers");
        }
        #else
        /* GCC 4.8 isn't able to figure out the size on its own. Which means
           there we use the type-provided size and lose the check for element
           count, but at least it compiles. */
        constexpr /*implicit*/ StaticArray(T(&&data)[size_]): Implementation::StaticArrayDataFor<size_, T>{Corrade::InPlaceInit, typename Implementation::GenerateSequence<size_>::Type{}, Utility::move(data)} {}
        #endif
        #endif

        /* The following view conversion is *not* restricted to this& because
           that would break uses like `consume(foo());`, where `consume()`
           expects a view but `foo()` returns an owning array. */

        /**
         * @brief Convert to external view representation
         *
         * @see @ref Containers-Array-stl
         */
        /* To simplify the implementation, there's no ArrayViewConverter
           overload. Instead, the implementer is supposed to extend
           StaticArrayViewConverter specializations for the non-static arrays
           as well. */
        template<class U, class = decltype(Implementation::StaticArrayViewConverter<size_, T, U>::to(std::declval<StaticArrayView<size_, T>>()))> /*implicit*/ operator U() {
            return Implementation::StaticArrayViewConverter<size_, T, U>::to(*this);
        }

        /** @overload */
        template<class U, class = decltype(Implementation::StaticArrayViewConverter<size_, const T, U>::to(std::declval<StaticArrayView<size_, const T>>()))> constexpr /*implicit*/ operator U() const {
            return Implementation::StaticArrayViewConverter<size_, const T, U>::to(*this);
        }

        #ifndef CORRADE_MSVC_COMPATIBILITY
        /** @brief Whether the array is non-empty */
        /* Disabled on MSVC w/o /permissive- to avoid ambiguous operator+()
           when doing pointer arithmetic. */
        constexpr explicit operator bool() const { return true; }
        #endif

        /* `char* a = Containers::StaticArray<char>(5); a[3] = 5;` would result
           in instant segfault, disallowing it in the following conversion
           operators */

        /** @brief Conversion to array type */
        /*implicit*/ operator T*() & { return this->_data; }

        /** @overload */
        constexpr /*implicit*/ operator const T*() const & { return this->_data; }

        /** @brief Array data */
        T* data() { return this->_data; }
        constexpr const T* data() const { return this->_data; }      /**< @overload */

        /**
         * @brief Array size
         *
         * Equivalent to @ref Size.
         * @see @ref isEmpty()
         */
        constexpr std::size_t size() const { return size_; }

        /**
         * @brief Whether the array is empty
         * @m_since_latest
         *
         * Always @cpp true @ce (it's not possible to create a zero-sized C
         * array).
         * @see @ref size(), @ref Size
         */
        constexpr bool isEmpty() const { return !size_; }

        #ifdef CORRADE_BUILD_DEPRECATED
        /** @copybrief isEmpty()
         * @m_deprecated_since_latest Use @ref isEmpty() instead.
         */
        CORRADE_DEPRECATED("use isEmpty() instead") constexpr bool empty() const { return !size_; }
        #endif

        /**
         * @brief Pointer to the first element
         *
         * @see @ref front(), @ref operator[]()
         */
        T* begin() { return this->_data; }
        /** @overload */
        constexpr const T* begin() const { return this->_data; }
        /** @overload */
        constexpr const T* cbegin() const { return this->_data; }

        /**
         * @brief Pointer to (one item after) the last element
         *
         * @see @ref back(), @ref operator[]()
         */
        T* end() { return this->_data + size_; }
        /** @overload */
        constexpr const T* end() const { return this->_data + size_; }
        /** @overload */
        constexpr const T* cend() const { return this->_data + size_; }

        /**
         * @brief First element
         *
         * @see @ref isEmpty(), @ref begin(), @ref operator[]()
         */
        T& front() { return this->_data[0]; }
        /** @overload */
        constexpr const T& front() const { return this->_data[0]; }

        /**
         * @brief Last element
         *
         * @see @ref isEmpty(), @ref end(), @ref operator[]()
         */
        T& back() { return this->_data[size_ - 1]; }
        /** @overload */
        constexpr const T& back() const { return this->_data[size_ - 1]; }

        /**
         * @brief Element access
         * @m_since_latest
         *
         * Expects that @p i is less than @ref size().
         * @see @ref front(), @ref back()
         */
        #ifdef DOXYGEN_GENERATING_OUTPUT
        T& operator[](std::size_t i);
        /**
         * @overload
         * @m_since_latest
         */
        constexpr const T& operator[](std::size_t i) const;
        #else
        /* Has to be done this way because otherwise it causes ambiguity with a
           builtin operator[] for pointers if an int or ssize_t is used due to
           the implicit pointer conversion. Sigh. */
        /** @todo clean up once implicit pointer conversion is removed */
        template<class U, typename std::enable_if<std::is_convertible<U, std::size_t>::value, int>::type = 0> T& operator[](U i);
        template<class U, typename std::enable_if<std::is_convertible<U, std::size_t>::value, int>::type = 0> constexpr const T& operator[](U i) const;
        #endif

        /**
         * @brief View on a slice
         *
         * Equivalent to @ref StaticArrayView::slice(T*, T*) const and
         * overloads.
         */
        /* Unlike sliceSize(T*, size_t), slice<size>(T*), prefix(T*) and
           suffix(T*) this doesn't have ambiguity prevention for slice(0, 0)
           as such use case is rather rare I think. */
        ArrayView<T> slice(T* begin, T* end) {
            return ArrayView<T>(*this).slice(begin, end);
        }
        /** @overload */
        constexpr ArrayView<const T> slice(const T* begin, const T* end) const {
            return ArrayView<const T>(*this).slice(begin, end);
        }
        /** @overload */
        ArrayView<T> slice(std::size_t begin, std::size_t end) {
            return ArrayView<T>(*this).slice(begin, end);
        }
        /** @overload */
        constexpr ArrayView<const T> slice(std::size_t begin, std::size_t end) const {
            return ArrayView<const T>(*this).slice(begin, end);
        }

        /**
         * @brief View on a slice of given size
         * @m_since_latest
         *
         * Equivalent to @ref StaticArrayView::sliceSize(T*, std::size_t) const
         * and overloads.
         */
        #ifdef DOXYGEN_GENERATING_OUTPUT
        ArrayView<T> sliceSize(T* begin, std::size_t size);
        #else
        /* To avoid ambiguity when calling sliceSize(0, ...). FFS, zero as null
           pointer was deprecated in C++11 already, why is this still a
           problem?! */
        template<class U, typename std::enable_if<std::is_convertible<U, T*>::value && !std::is_convertible<U, std::size_t>::value, int>::type = 0> ArrayView<T> sliceSize(U begin, std::size_t size) {
            return ArrayView<T>{*this}.sliceSize(begin, size);
        }
        #endif
        /**
         * @overload
         * @m_since_latest
         */
        #ifdef DOXYGEN_GENERATING_OUTPUT
        constexpr ArrayView<const T> sliceSize(const T* begin, std::size_t size) const;
        #else
        /* To avoid ambiguity when calling sliceSize(0, ...). FFS, zero as null
           pointer was deprecated in C++11 already, why is this still a
           problem?! */
        template<class U, typename std::enable_if<std::is_convertible<U, const T*>::value && !std::is_convertible<U, std::size_t>::value, int>::type = 0> constexpr ArrayView<const T> sliceSize(const U begin, std::size_t size) const {
            return ArrayView<const T>{*this}.sliceSize(begin, size);
        }
        #endif
        /**
         * @overload
         * @m_since_latest
         */
        ArrayView<T> sliceSize(std::size_t begin, std::size_t size) {
            return ArrayView<T>{*this}.sliceSize(begin, size);
        }
        /**
         * @overload
         * @m_since_latest
         */
        constexpr ArrayView<const T> sliceSize(std::size_t begin, std::size_t size) const {
            return ArrayView<const T>{*this}.sliceSize(begin, size);
        }

        /**
         * @brief Fixed-size view on a slice
         *
         * Equivalent to @ref StaticArrayView::slice(T*) const and overloads.
         */
        #ifdef DOXYGEN_GENERATING_OUTPUT
        template<std::size_t size__> StaticArrayView<size__, T> slice(T* begin);
        #else
        /* To avoid ambiguity when calling slice<size>(0). FFS, zero as null
           pointer was deprecated in C++11 already, why is this still a
           problem?! */
        template<std::size_t size__, class U, typename std::enable_if<std::is_convertible<U, T*>::value && !std::is_convertible<U, std::size_t>::value, int>::type = 0> StaticArrayView<size__, T> slice(U begin) {
            return ArrayView<T>(*this).template slice<size__>(begin);
        }
        #endif
        /** @overload */
        #ifdef DOXYGEN_GENERATING_OUTPUT
        template<std::size_t size__> constexpr StaticArrayView<size__, const T> slice(const T* begin) const;
        #else
        /* To avoid ambiguity when calling slice<size>(0). FFS, zero as null
           pointer was deprecated in C++11 already, why is this still a
           problem?! */
        template<std::size_t size__, class U, typename std::enable_if<std::is_convertible<U, const T*>::value && !std::is_convertible<U, std::size_t>::value, int>::type = 0> constexpr StaticArrayView<size__, const T> slice(U begin) const {
            return ArrayView<const T>(*this).template slice<size__>(begin);
        }
        #endif
        /** @overload */
        template<std::size_t size__> StaticArrayView<size__, T> slice(std::size_t begin) {
            return ArrayView<T>(*this).template slice<size__>(begin);
        }
        /** @overload */
        template<std::size_t size__> constexpr StaticArrayView<size__, const T> slice(std::size_t begin) const {
            return ArrayView<const T>(*this).template slice<size__>(begin);
        }

        /**
         * @brief Fixed-size view on a slice
         * @m_since{2019,10}
         *
         * Equivalent to @ref StaticArrayView::slice() const.
         */
        template<std::size_t begin_, std::size_t end_> StaticArrayView<end_ - begin_, T> slice() {
            return StaticArrayView<size_, T>(*this).template slice<begin_, end_>();
        }

        /**
         * @overload
         * @m_since{2019,10}
         */
        template<std::size_t begin_, std::size_t end_> constexpr StaticArrayView<end_ - begin_, const T> slice() const {
            return StaticArrayView<size_, const T>(*this).template slice<begin_, end_>();
        }

        /**
         * @brief Fixed-size view on a slice of given size
         * @m_since_latest
         *
         * Equivalent to @ref StaticArrayView::sliceSize() const.
         */
        template<std::size_t begin_, std::size_t size__> StaticArrayView<size__, T> sliceSize() {
            return StaticArrayView<size_, T>(*this).template sliceSize<begin_, size__>();
        }

        /**
         * @overload
         * @m_since_latest
         */
        template<std::size_t begin_, std::size_t size__> constexpr StaticArrayView<size__, const T> sliceSize() const {
            return StaticArrayView<size_, const T>(*this).template sliceSize<begin_, size__>();
        }

        /**
         * @brief View on a prefix until a pointer
         *
         * Equivalent to @ref StaticArrayView::prefix(T*) const.
         */
        #ifdef DOXYGEN_GENERATING_OUTPUT
        ArrayView<T> prefix(T* end);
        #else
        /* To avoid ambiguity when calling prefix(0). FFS, zero as null pointer
           was deprecated in C++11 already, why is this still a problem?! */
        template<class U, typename std::enable_if<std::is_convertible<U, T*>::value && !std::is_convertible<U, std::size_t>::value, int>::type = 0> ArrayView<T> prefix(U end) {
            return ArrayView<T>(*this).prefix(end);
        }
        #endif
        /** @overload */
        #ifdef DOXYGEN_GENERATING_OUTPUT
        constexpr ArrayView<const T> prefix(const T* end) const;
        #else
        /* To avoid ambiguity when calling prefix(0). FFS, zero as null pointer
           was deprecated in C++11 already, why is this still a problem?! */
        template<class U, typename std::enable_if<std::is_convertible<U, const T*>::value && !std::is_convertible<U, std::size_t>::value, int>::type = 0> constexpr ArrayView<const T> prefix(U end) const {
            return ArrayView<const T>(*this).prefix(end);
        }
        #endif

        /**
         * @brief View on a suffix after a pointer
         *
         * Equivalent to @ref StaticArrayView::suffix(T*) const.
         * @todo once non-deprecated suffix(std::size_t size) is a thing, add
         *      the ambiguity-preventing template here as well
         */
        ArrayView<T> suffix(T* begin) {
            return ArrayView<T>(*this).suffix(begin);
        }
        /**
         * @overload
         * @todo once non-deprecated suffix(std::size_t size) is a thing, add
         *      the ambiguity-preventing template here as well
         */
        constexpr ArrayView<const T> suffix(const T* begin) const {
            return ArrayView<const T>(*this).suffix(begin);
        }

        /**
         * @brief View on the first @p size items
         *
         * Equivalent to @ref StaticArrayView::prefix(std::size_t) const.
         */
        ArrayView<T> prefix(std::size_t size) {
            return ArrayView<T>(*this).prefix(size);
        }
        /** @overload */
        constexpr ArrayView<const T> prefix(std::size_t size) const {
            return ArrayView<const T>(*this).prefix(size);
        }

        /* Here will be suffix(std::size_t size), view on the last size
           items, once the deprecated suffix(std::size_t begin) is gone and
           enough time passes to not cause silent breakages in existing code. */

        /**
         * @brief Fixed-size view on the first @p size__ items
         *
         * Equivalent to @ref StaticArrayView::prefix() const and overloads.
         */
        template<std::size_t size__> StaticArrayView<size__, T> prefix();
        template<std::size_t size__> constexpr StaticArrayView<size__, const T> prefix() const; /**< @overload */

        /* Here will be suffix<size__>(), view on the last size__ items, once
           the deprecated suffix<begin_>() is gone and enough time passes to
           not cause silent breakages in existing code. */

        /**
         * @brief View except the first @p size items
         * @m_since_latest
         *
         * Equivalent to @ref StaticArrayView::exceptPrefix(std::size_t) const.
         */
        ArrayView<T> exceptPrefix(std::size_t size) {
            return ArrayView<T>(*this).exceptPrefix(size);
        }
        /**
         * @overload
         * @m_since_latest
         */
        constexpr ArrayView<const T> exceptPrefix(std::size_t size) const {
            return ArrayView<const T>(*this).exceptPrefix(size);
        }

        #ifdef CORRADE_BUILD_DEPRECATED
        /** @copybrief exceptPrefix()
         * @m_deprecated_since_latest Use @ref exceptPrefix() instead.
         */
        CORRADE_DEPRECATED("use exceptPrefix() instead") ArrayView<T> suffix(std::size_t begin) {
            return ArrayView<T>(*this).suffix(begin);
        }
        /** @copybrief exceptPrefix()
         * @m_deprecated_since_latest Use @ref exceptPrefix() instead.
         */
        CORRADE_DEPRECATED("use exceptPrefix() instead") ArrayView<const T> suffix(std::size_t begin) const {
            return ArrayView<const T>(*this).suffix(begin);
        }
        #endif

        /**
         * @brief Fixed-size view except the first @p size__ items
         * @m_since_latest
         *
         * Equivalent to @ref StaticArrayView::exceptPrefix() const and
         * overloads.
         */
        template<std::size_t size__> StaticArrayView<size_ - size__, T> exceptPrefix() {
            return StaticArrayView<size_, T>(*this).template exceptPrefix<size__>();
        }
         /**
          * @overload
          * @m_since_latest
          */
        template<std::size_t size__> constexpr StaticArrayView<size_ - size__, const T> exceptPrefix() const {
            return StaticArrayView<size_, const T>(*this).template exceptPrefix<size__>();
        }

        #ifdef CORRADE_BUILD_DEPRECATED
        /**
         * @copybrief exceptPrefix()
         * @m_deprecated_since_latest Use @ref exceptPrefix() instead.
         */
        template<std::size_t begin_> CORRADE_DEPRECATED("use exceptPrefix() instead") StaticArrayView<size_ - begin_, T> suffix() {
            return StaticArrayView<size_, T>(*this).template exceptPrefix<begin_>();
        }
        /**
         * @copybrief exceptPrefix()
         * @m_deprecated_since_latest Use @ref exceptPrefix() instead.
         */
        template<std::size_t begin_> CORRADE_DEPRECATED("use exceptPrefix() instead") StaticArrayView<size_ - begin_, const T> suffix() const {
            return StaticArrayView<size_, const T>(*this).template exceptPrefix<begin_>();
        }
        #endif

        /**
         * @brief View except the last @p size items
         * @m_since_latest
         *
         * Equivalent to @ref StaticArrayView::exceptSuffix(std::size_t) const.
         */
        ArrayView<T> exceptSuffix(std::size_t size) {
            return ArrayView<T>(*this).exceptSuffix(size);
        }
        /**
         * @overload
         * @m_since_latest
         */
        constexpr ArrayView<const T> exceptSuffix(std::size_t size) const {
            return ArrayView<const T>(*this).exceptSuffix(size);
        }

        #ifdef CORRADE_BUILD_DEPRECATED
        /**
         * @copybrief exceptSuffix()
         * @m_deprecated_since_latest Use @ref exceptSuffix() instead.
         */
        CORRADE_DEPRECATED("use exceptSuffix() instead") ArrayView<T> except(std::size_t count) {
            return ArrayView<T>(*this).exceptSuffix(count);
        }
        /**
         * @overload
         * @m_deprecated_since_latest Use @ref exceptSuffix() instead.
         */
        CORRADE_DEPRECATED("use exceptSuffix() instead") ArrayView<const T> except(std::size_t count) const {
            return ArrayView<const T>(*this).exceptSuffix(count);
        }
        #endif

        /**
         * @brief Fixed-size view except the last @p size__ items
         * @m_since_latest
         *
         * Equivalent to @ref StaticArrayView::exceptSuffix() const.
         */
        template<std::size_t size__> StaticArrayView<size_ - size__, T> exceptSuffix() {
            return StaticArrayView<size_, T>(*this).template exceptSuffix<size__>();
        }
        /**
         * @overload
         * @m_since_latest
         */
        template<std::size_t size__> constexpr StaticArrayView<size_ - size__, const T> exceptSuffix() const {
            return StaticArrayView<size_, const T>(*this).template exceptSuffix<size__>();
        }

        #ifdef CORRADE_BUILD_DEPRECATED
        /**
         * @copybrief exceptSuffix()
         * @m_deprecated_since_latest Use @ref exceptSuffix() instead.
         */
        template<std::size_t count> CORRADE_DEPRECATED("use exceptSuffix() instead") StaticArrayView<size_ - count, T> except() {
            return StaticArrayView<size_, T>(*this).template exceptSuffix<count>();
        }
        /**
         * @overload
         * @m_deprecated_since_latest Use @ref exceptSuffix() instead.
         */
        template<std::size_t count> CORRADE_DEPRECATED("use exceptSuffix() instead") StaticArrayView<size_ - count, const T> except() const {
            return StaticArrayView<size_, const T>(*this).template exceptSuffix<count>();
        }
        #endif

    private:
        #if CORRADE_CXX_STANDARD > 201402
        /* For C++17 structured bindings, if StructuredBindings.h is included
           as well. There doesn't seem to be a way to call those directly, and
           I can't find any practical use of std::tuple_size, tuple_element etc
           on C++11 and C++14, so this is defined only for newer standards. */
        template<std::size_t index> constexpr friend const T& get(const StaticArray<size_, T>& value) {
            return value._data[index];
        }
        template<std::size_t index> CORRADE_CONSTEXPR14 friend T& get(StaticArray<size_, T>& value) {
            return value._data[index];
        }
        template<std::size_t index> CORRADE_CONSTEXPR14 friend T&& get(StaticArray<size_, T>&& value) {
            return Utility::move(value._data[index]);
        }
        #endif
};

#ifndef CORRADE_MSVC2015_COMPATIBILITY /* Multiple definitions still broken */
/**
@brief One-component array
@m_since_latest

Convenience alternative to @cpp StaticArray<1, T> @ce. See @ref StaticArray for
more information. Useful in case you want to take advantage of the @ref NoInit
tag on an arbitrary type and @ref Optional doesn't suit the use case.
@see @ref Array2, @ref Array3, @ref Array4, @ref ArrayView2, @ref ArrayView3,
    @ref ArrayView4
*/
template<class T> using Array1 = StaticArray<1, T>;

/**
@brief Two-component array
@m_since_latest

Convenience alternative to @cpp StaticArray<2, T> @ce. See @ref StaticArray for
more information.
@see @ref Array1, @ref Array3, @ref Array4, @ref ArrayView2, @ref ArrayView3,
    @ref ArrayView4
*/
template<class T> using Array2 = StaticArray<2, T>;

/**
@brief Three-component array
@m_since_latest

Convenience alternative to @cpp StaticArray<3, T> @ce. See @ref StaticArray for
more information.
@see @ref Array1, @ref Array2, @ref Array4, @ref ArrayView2, @ref ArrayView3,
    @ref ArrayView4
*/
template<class T> using Array3 = StaticArray<3, T>;

/**
@brief Four-component array
@m_since_latest

Convenience alternative to @cpp StaticArray<4, T> @ce. See @ref StaticArray for
more information.
@see @ref Array1, @ref Array2, @ref Array3, @ref ArrayView2, @ref ArrayView3,
    @ref ArrayView4
*/
template<class T> using Array4 = StaticArray<4, T>;
#endif

/** @relatesalso StaticArray
@brief Make a view on a @ref StaticArray

Convenience alternative to converting to an @ref ArrayView explicitly. The
following two lines are equivalent:

@snippet Containers.cpp StaticArray-arrayView
*/
template<std::size_t size, class T> constexpr ArrayView<T> arrayView(StaticArray<size, T>& array) {
    return ArrayView<T>{array};
}

/** @relatesalso StaticArray
@brief Make a view on a const @ref StaticArray

Convenience alternative to converting to an @ref ArrayView explicitly. The
following two lines are equivalent:

@snippet Containers.cpp StaticArray-arrayView-const
*/
template<std::size_t size, class T> constexpr ArrayView<const T> arrayView(const StaticArray<size, T>& array) {
    return ArrayView<const T>{array};
}

/** @relatesalso StaticArray
@brief Make a static view on a @ref StaticArray

Convenience alternative to converting to an @ref StaticArrayView explicitly.
The following two lines are equivalent:

@snippet Containers.cpp StaticArray-staticArrayView

@todoc Make it a real reference once Doxygen is sane
*/
template<std::size_t size, class T> constexpr StaticArrayView<size, T> staticArrayView(StaticArray<size, T>& array) {
    return StaticArrayView<size, T>{array};
}

/** @relatesalso StaticArray
@brief Make a static view on a const @ref StaticArray

Convenience alternative to converting to an @ref StaticArrayView explicitly.
The following two lines are equivalent:

@snippet Containers.cpp StaticArray-staticArrayView-const

@todoc Make it a real reference once Doxygen is sane
*/
template<std::size_t size, class T> constexpr StaticArrayView<size, const T> staticArrayView(const StaticArray<size, T>& array) {
    return StaticArrayView<size, const T>{array};
}

/** @relatesalso StaticArray
@brief Reinterpret-cast a static array

See @ref arrayCast(StaticArrayView<size, T>) for more information.
*/
template<class U, std::size_t size, class T> StaticArrayView<size*sizeof(T)/sizeof(U), U> arrayCast(StaticArray<size, T>& array) {
    return arrayCast<U>(staticArrayView(array));
}

/** @overload */
template<class U, std::size_t size, class T> StaticArrayView<size*sizeof(T)/sizeof(U), const U> arrayCast(const StaticArray<size, T>& array) {
    return arrayCast<const U>(staticArrayView(array));
}

/** @relatesalso StaticArray
@brief Static array size

See @ref arraySize(ArrayView<T>) for more information.
*/
template<std::size_t size_, class T> constexpr std::size_t arraySize(const StaticArray<size_, T>&) {
    return size_;
}

template<std::size_t size_, class T> template<class ...Args> StaticArray<size_, T>::StaticArray(Corrade::DirectInitT, Args&&... args): StaticArray{Corrade::NoInit} {
    for(T& i: this->_data)
        /* This works around a featurebug in C++ where new T{} doesn't work for
           an explicit defaulted constructor. Additionally it works around GCC
           4.8 bugs where copy/move construction can't be done with {} for
           plain structs. */
        Implementation::construct(i, Utility::forward<Args>(args)...);
}

namespace Implementation {

template<std::size_t size_, class T> StaticArrayData<size_, T, false>::StaticArrayData(const StaticArrayData<size_, T, false>& other) noexcept(std::is_nothrow_copy_constructible<T>::value): StaticArrayData{Corrade::NoInit} {
    for(std::size_t i = 0; i != size_; ++i)
        /* Can't use {}, see the GCC 4.8-specific overload for details */
        #if defined(CORRADE_TARGET_GCC) && !defined(CORRADE_TARGET_CLANG) && __GNUC__ < 5
        Implementation::construct(_data[i], other._data[i]);
        #else
        new(_data + i) T{other._data[i]};
        #endif
}

template<std::size_t size_, class T> StaticArrayData<size_, T, false>::StaticArrayData(StaticArrayData<size_, T, false>&& other) noexcept(std::is_nothrow_move_constructible<T>::value): StaticArrayData{Corrade::NoInit} {
    for(std::size_t i = 0; i != size_; ++i)
        /* Can't use {}, see the GCC 4.8-specific overload for details */
        #if defined(CORRADE_TARGET_GCC) && !defined(CORRADE_TARGET_CLANG) && __GNUC__ < 5
        Implementation::construct(_data[i], Utility::move(other._data[i]));
        #else
        new(&_data[i]) T{Utility::move(other._data[i])};
        #endif
}

template<std::size_t size_, class T> StaticArrayData<size_, T, false>::~StaticArrayData() {
    for(T& i: _data) {
        i.~T();
        #ifdef CORRADE_MSVC2015_COMPATIBILITY
        /* Complains i is set but not used for trivially destructible types */
        static_cast<void>(i);
        #endif
    }
}

template<std::size_t size_, class T> StaticArrayData<size_, T, false>& StaticArrayData<size_, T, false>::operator=(const StaticArrayData<size_, T, false>& other) noexcept(std::is_nothrow_copy_constructible<T>::value) {
    for(std::size_t i = 0; i != size_; ++i)
        _data[i] = other._data[i];
    return *this;
}

template<std::size_t size_, class T> StaticArrayData<size_, T, false>& StaticArrayData<size_, T, false>::operator=(StaticArrayData<size_, T, false>&& other) noexcept(std::is_nothrow_move_constructible<T>::value) {
    using Utility::swap;
    for(std::size_t i = 0; i != size_; ++i)
        swap(_data[i], other._data[i]);
    return *this;
}

}

#ifndef DOXYGEN_GENERATING_OUTPUT
template<std::size_t size_, class T> template<class U, typename std::enable_if<std::is_convertible<U, std::size_t>::value, int>::type> constexpr const T& StaticArray<size_, T>::operator[](const U i) const {
    return CORRADE_CONSTEXPR_DEBUG_ASSERT(std::size_t(i) < size_,
        "Containers::StaticArray::operator[](): index" << i << "out of range for" << size_ << "elements"), this->_data[i];
}

template<std::size_t size_, class T> template<class U, typename std::enable_if<std::is_convertible<U, std::size_t>::value, int>::type> T& StaticArray<size_, T>::operator[](const U i) {
    return const_cast<T&>(static_cast<const StaticArray<size_, T>&>(*this)[i]);
}
#endif

template<std::size_t size_, class T> template<std::size_t size__> StaticArrayView<size__, T> StaticArray<size_, T>::prefix() {
    static_assert(size__ <= size_, "prefix size too large");
    return StaticArrayView<size__, T>{this->_data};
}

template<std::size_t size_, class T> template<std::size_t size__> constexpr StaticArrayView<size__, const T> StaticArray<size_, T>::prefix() const {
    static_assert(size__ <= size_, "prefix size too large");
    return StaticArrayView<size__, const T>{this->_data};
}

namespace Implementation {

/* StaticArray to ArrayView in order to have implicit conversion for
   StridedArrayView without needing to introduce a header dependency. The
   SFINAE needs to be here in order to ensure proper behavior with function
   overloads taking more than one type of (Strided)ArrayView. */
template<class U, std::size_t size, class T> struct ArrayViewConverter<U, StaticArray<size, T>> {
    template<class V = U, typename std::enable_if<std::is_convertible<T*, V*>::value, int>::type = 0> constexpr static ArrayView<U> from(StaticArray<size, T>& other) {
        static_assert(sizeof(T) == sizeof(U), "types are not compatible");
        return {other.data(), other.size()};
    }
};
template<class U, std::size_t size, class T> struct ArrayViewConverter<const U, StaticArray<size, T>> {
    template<class V = U, typename std::enable_if<std::is_convertible<T*, V*>::value, int>::type = 0> constexpr static ArrayView<const U> from(const StaticArray<size, T>& other) {
        static_assert(sizeof(T) == sizeof(U), "types are not compatible");
        return {other.data(), other.size()};
    }
};
template<class U, std::size_t size, class T> struct ArrayViewConverter<const U, StaticArray<size, const T>> {
    template<class V = U, typename std::enable_if<std::is_convertible<T*, V*>::value, int>::type = 0> constexpr static ArrayView<const U> from(const StaticArray<size, const T>& other) {
        static_assert(sizeof(T) == sizeof(U), "types are not compatible");
        return {other.data(), other.size()};
    }
};
template<std::size_t size, class T> struct ErasedArrayViewConverter<StaticArray<size, T>>: ArrayViewConverter<T, StaticArray<size, T>> {};
template<std::size_t size, class T> struct ErasedArrayViewConverter<const StaticArray<size, T>>: ArrayViewConverter<const T, StaticArray<size, T>> {};

/* StaticArray to StaticArrayView in order to have implicit conversion for
   StridedArrayView without needing to introduce a header dependency. The
   SFINAE needs to be here in order to ensure proper behavior with function
   overloads taking more than one type of StaticArrayView. */
template<class U, std::size_t size, class T> struct StaticArrayViewConverter<size, U, StaticArray<size, T>> {
    template<class V = U, typename std::enable_if<std::is_convertible<T*, V*>::value, int>::type = 0> constexpr static StaticArrayView<size, U> from(StaticArray<size, T>& other) {
        static_assert(sizeof(T) == sizeof(U), "types are not compatible");
        return StaticArrayView<size, T>{other.data()};
    }
};
template<class U, std::size_t size, class T> struct StaticArrayViewConverter<size, const U, StaticArray<size, T>> {
    template<class V = U, typename std::enable_if<std::is_convertible<T*, V*>::value, int>::type = 0> constexpr static StaticArrayView<size, const U> from(const StaticArray<size, T>& other) {
        static_assert(sizeof(T) == sizeof(U), "types are not compatible");
        return StaticArrayView<size, const T>(other.data());
    }
};
template<class U, std::size_t size, class T> struct StaticArrayViewConverter<size, const U, StaticArray<size, const T>> {
    template<class V = U, typename std::enable_if<std::is_convertible<T*, V*>::value, int>::type = 0> constexpr static StaticArrayView<size, const U> from(const StaticArray<size, const T>& other) {
        static_assert(sizeof(T) == sizeof(U), "types are not compatible");
        return StaticArrayView<size, const T>(other.data());
    }
};
template<std::size_t size, class T> struct ErasedStaticArrayViewConverter<StaticArray<size, T>>: StaticArrayViewConverter<size, T, StaticArray<size, T>> {};
template<std::size_t size, class T> struct ErasedStaticArrayViewConverter<const StaticArray<size, T>>: StaticArrayViewConverter<size, const T, StaticArray<size, T>> {};

}

}}

#endif
