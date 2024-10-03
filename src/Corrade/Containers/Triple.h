#ifndef Corrade_Containers_Triple_h
#define Corrade_Containers_Triple_h
/*
    This file is part of Corrade.

    Copyright © 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016,
                2017, 2018, 2019, 2020, 2021, 2022, 2023, 2024
              Vladimír Vondruš <mosra@centrum.cz>
    Copyright © 2022, 2023 Stanislaw Halik <sthalik@misaki.pl>

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

#include <cstddef> /* std::size_t */
/* std::declval() is said to be in <utility> but libstdc++, libc++ and MSVC STL
   all have it directly in <type_traits> because it just makes sense */
#include <type_traits>

#include "Corrade/Tags.h"
#ifndef CORRADE_SINGLES_NO_DEBUG
#include "Corrade/Utility/Debug.h"
#endif
#include "Corrade/Utility/Macros.h" /* CORRADE_CONSTEXPR14 */
#include "Corrade/Utility/Move.h"

/** @file
 * @brief Class @ref Corrade::Containers::Triple
 * @m_since_latest
 */

namespace Corrade { namespace Containers {

namespace Implementation {
    template<class, class, class, class> struct TripleConverter;
}

/**
@brief A tuple of three values
@m_since_latest

Like @ref Pair, but for three elements. Also a lightweight alternative to a
three-element @ref std::tuple that guarantees usability in @cpp constexpr @ce
contexts even in C++11. On the other hand, to simplify both the implementation
and usage semantics, the type doesn't support references --- wrap them in a
@ref Reference in order to store them in a @ref Triple. Such type composition
allows you to both rebind the reference and update the referenced value and the
intent is clear.

Similarly to other containers and equivalently to @ref std::make_tuple(),
there's also @ref triple(). The following two lines are equivalent:

@snippet Containers.cpp triple

Similarly to a @ref Pair, access to the triple elements is done using
@ref first(), @ref second() and @ref third() member functions, direct access to
the members isn't provided. This is done in order to future-proof the design
and have extra flexibility in how the internals are defined.

@section Containers-Triple-structured-bindings C++17 structured bindings

If @ref Corrade/Containers/StructuredBindings.h is included, the class can be
used in C++17 structured bindings. While the @cpp get<i>() @ce overloads are
defined inside @ref Triple itself, a separate header is used for the
@m_class{m-doc-external} [std::tuple_size](https://en.cppreference.com/w/cpp/utility/tuple_size)
and @m_class{m-doc-external} [std::tuple_element](https://en.cppreference.com/w/cpp/utility/tuple_element)
template specializations, as those may require @cpp #include <utility> @ce on
some STL implementations. Example:

@snippet Containers-cpp17.cpp Triple-structured-bindings

@section Containers-Triple-stl STL compatibility

Instances of @ref Triple are implicitly copy- and move-convertible to and from
a three-element @ref std::tuple if you include
@ref Corrade/Containers/TripleStl.h. The conversion is provided in a separate
header to avoid overhead from an unconditional @cpp #include <tuple> @ce.
Additionally, the @ref triple(T&&) overload also allows for such a conversion.
Example:

@snippet Containers-stl.cpp Triple

<b></b>

@m_class{m-block m-success}

@par Single-header version
    This class is also available as a single-header, dependency-less
    [CorradeTriple.h](https://github.com/mosra/magnum-singles/tree/master/CorradeTriple.h)
    library in the Magnum Singles repository for easier integration into your
    projects. See @ref corrade-singles for more information.
@par
    Structured bindings on C++17 are opt-in due to reliance on a potentially
    heavy STL header --- @cpp #define CORRADE_STRUCTURED_BINDINGS @ce before
    including the file. The above-mentioned STL compatibility bits are included
    as well --- opt-in with @cpp #define CORRADE_TRIPLE_STL_COMPATIBILITY @ce
    before including the file. Including it multiple times with different
    macros defined works as well.

@see @ref triple(F&&, S&&, T&&), @ref triple(T&&)
*/
template<class F, class S, class T> class Triple {
    static_assert(!std::is_lvalue_reference<F>::value && !std::is_lvalue_reference<S>::value && !std::is_lvalue_reference<T>::value, "use a Reference<T> to store a T& in a Triple");

    public:
        typedef F FirstType;    /**< @brief First type */
        typedef S SecondType;   /**< @brief Second type */
        typedef T ThirdType;   /**< @brief Second type */

        /**
         * @brief Construct a default-initialized triple
         *
         * Trivial types are not initialized, default constructor called
         * otherwise. Because of the differing behavior for trivial types it's
         * better to explicitly use either the @ref Triple(ValueInitT) or the
         * @ref Triple(NoInitT) variant instead.
         * @see @ref DefaultInit, @ref std::is_trivial
         */
        #ifndef CORRADE_MSVC2015_COMPATIBILITY
        /* Not constexpr for this joke of a compiler because I don't explicitly
           initialize _first and _second, which wouldn't be a default
           initialization if I did that. */
        constexpr
        #endif
        explicit Triple(Corrade::DefaultInitT) noexcept(std::is_nothrow_constructible<F>::value && std::is_nothrow_constructible<S>::value && std::is_nothrow_constructible<T>::value) {}

        /**
         * @brief Construct a value-initialized triple
         *
         * Trivial types are zero-initialized, default constructor called
         * otherwise. This is the same as the default constructor.
         * @see @ref ValueInit, @ref Triple(DefaultInitT)
         */
        constexpr explicit Triple(Corrade::ValueInitT) noexcept(std::is_nothrow_constructible<F>::value && std::is_nothrow_constructible<S>::value && std::is_nothrow_constructible<T>::value):
            /* Can't use {} here. See constructHelpers.h for details, test in
               TripleTest::constructorExplicitInCopyInitialization(). */
            _first(), _second(), _third() {}

        #ifdef DOXYGEN_GENERATING_OUTPUT
        /**
         * @brief Construct a triple without initializing its contents
         *
         * Enabled only for trivial types and types that implement the
         * @ref NoInit constructor. The contents are *not* initialized. Useful
         * if you will be overwriting both members later anyway or if you need
         * to initialize in a way that's not expressible via any other
         * @ref Triple constructor.
         *
         * For trivial types is equivalent to @ref Triple(DefaultInitT).
         */
        explicit Triple(Corrade::NoInitT) noexcept;
        #else
        template<class F_ = F, class = typename std::enable_if<std::is_standard_layout<F_>::value && std::is_standard_layout<S>::value && std::is_standard_layout<T>::value && std::is_trivial<F_>::value && std::is_trivial<S>::value && std::is_trivial<T>::value>::type> explicit Triple(Corrade::NoInitT) noexcept {}
        template<class F_ = F, class S_ = S, class = typename std::enable_if<std::is_constructible<F_, Corrade::NoInitT>::value && std::is_constructible<S_, Corrade::NoInitT>::value && std::is_constructible<T, Corrade::NoInitT>::value>::type> explicit Triple(Corrade::NoInitT) noexcept(std::is_nothrow_constructible<F, Corrade::NoInitT>::value && std::is_nothrow_constructible<S, Corrade::NoInitT>::value && std::is_nothrow_constructible<T, Corrade::NoInitT>::value): _first{Corrade::NoInit}, _second{Corrade::NoInit}, _third{Corrade::NoInit} {}
        #endif

        /**
         * @brief Default constructor
         *
         * Alias to @ref Triple(ValueInitT).
         */
        constexpr /*implicit*/ Triple() noexcept(std::is_nothrow_constructible<F>::value && std::is_nothrow_constructible<S>::value && std::is_nothrow_constructible<T>::value):
            #ifdef CORRADE_MSVC2015_COMPATIBILITY
            /* Otherwise it complains that _first and _second isn't initialized
               in a constexpr context. Does it not see the delegation?! OTOH
               MSVC 2015 doesn't seem to be affected by the
               constructorExplicitInCopyInitialization() bug in GCC and
               Clang, so I can use {} here I think. */
            _first{}, _second{}, _third{}
            #else
            Triple{Corrade::ValueInit}
            #endif
        {}

        /**
         * @brief Constructor
         *
         * @see @ref triple(F&&, S&&, T&&)
         */
        constexpr /*implicit*/ Triple(const F& first, const S& second, const T& third) noexcept(std::is_nothrow_copy_constructible<F>::value && std::is_nothrow_copy_constructible<S>::value && std::is_nothrow_copy_constructible<T>::value):
            /* Can't use {} on GCC 4.8, see constructHelpers.h for details and
               TripleTest::copyMoveConstructPlainStruct() for a test. */
            #if defined(CORRADE_TARGET_GCC) && !defined(CORRADE_TARGET_CLANG) && __GNUC__ < 5
            _first(first), _second(second), _third(third)
            #else
            _first{first}, _second{second}, _third{third}
            #endif
            {}
        /** @overload */
        constexpr /*implicit*/ Triple(const F& first, const S& second, T&& third) noexcept(std::is_nothrow_copy_constructible<F>::value && std::is_nothrow_copy_constructible<S>::value && std::is_nothrow_move_constructible<T>::value):
            /* Can't use {} on GCC 4.8, see constructHelpers.h for details and
               TripleTest::copyMoveConstructPlainStruct() for a test. */
            #if defined(CORRADE_TARGET_GCC) && !defined(CORRADE_TARGET_CLANG) && __GNUC__ < 5
            _first(first), _second(second), _third(Utility::move(third))
            #else
            _first{first}, _second{second}, _third{Utility::move(third)}
            #endif
            {}
        /** @overload */
        constexpr /*implicit*/ Triple(const F& first, S&& second, const T& third) noexcept(std::is_nothrow_copy_constructible<F>::value && std::is_nothrow_move_constructible<S>::value && std::is_nothrow_copy_constructible<T>::value):
            /* Can't use {} on GCC 4.8, see constructHelpers.h for details and
               TripleTest::copyMoveConstructPlainStruct() for a test. */
            #if defined(CORRADE_TARGET_GCC) && !defined(CORRADE_TARGET_CLANG) && __GNUC__ < 5
            _first(first), _second(Utility::move(second)), _third(third)
            #else
            _first{first}, _second{Utility::move(second)}, _third{third}
            #endif
            {}
        /** @overload */
        constexpr /*implicit*/ Triple(F&& first, const S& second, const T& third) noexcept(std::is_nothrow_move_constructible<F>::value && std::is_nothrow_copy_constructible<S>::value && std::is_nothrow_copy_constructible<T>::value):
            /* Can't use {} on GCC 4.8, see constructHelpers.h for details and
               TripleTest::copyMoveConstructPlainStruct() for a test. */
            #if defined(CORRADE_TARGET_GCC) && !defined(CORRADE_TARGET_CLANG) && __GNUC__ < 5
            _first(Utility::move(first)), _second(second), _third(third)
            #else
            _first{Utility::move(first)}, _second{second}, _third{third}
            #endif
            {}
        /** @overload */
        constexpr /*implicit*/ Triple(const F& first, S&& second, T&& third) noexcept(std::is_nothrow_copy_constructible<F>::value && std::is_nothrow_move_constructible<S>::value && std::is_nothrow_move_constructible<T>::value):
            /* Can't use {} on GCC 4.8, see constructHelpers.h for details and
               TripleTest::copyMoveConstructPlainStruct() for a test. */
            #if defined(CORRADE_TARGET_GCC) && !defined(CORRADE_TARGET_CLANG) && __GNUC__ < 5
            _first(first), _second(Utility::move(second)), _third(Utility::move(third))
            #else
            _first{first}, _second{Utility::move(second)}, _third{Utility::move(third)}
            #endif
            {}
        /** @overload */
        constexpr /*implicit*/ Triple(F&& first, const S& second, T&& third) noexcept(std::is_nothrow_move_constructible<F>::value && std::is_nothrow_copy_constructible<S>::value && std::is_nothrow_move_constructible<T>::value):
            /* Can't use {} on GCC 4.8, see constructHelpers.h for details and
               TripleTest::copyMoveConstructPlainStruct() for a test. */
            #if defined(CORRADE_TARGET_GCC) && !defined(CORRADE_TARGET_CLANG) && __GNUC__ < 5
            _first(Utility::move(first)), _second(second), _third(Utility::move(third))
            #else
            _first{Utility::move(first)}, _second{second}, _third{Utility::move(third)}
            #endif
            {}
        /** @overload */
        constexpr /*implicit*/ Triple(F&& first, S&& second, const T& third) noexcept(std::is_nothrow_move_constructible<F>::value && std::is_nothrow_move_constructible<S>::value && std::is_nothrow_copy_constructible<T>::value):
            /* Can't use {} on GCC 4.8, see constructHelpers.h for details and
               TripleTest::copyMoveConstructPlainStruct() for a test. */
            #if defined(CORRADE_TARGET_GCC) && !defined(CORRADE_TARGET_CLANG) && __GNUC__ < 5
            _first(Utility::move(first)), _second(Utility::move(second)), _third(third)
            #else
            _first{Utility::move(first)}, _second{Utility::move(second)}, _third{third}
            #endif
            {}
        /** @overload */
        constexpr /*implicit*/ Triple(F&& first, S&& second, T&& third) noexcept(std::is_nothrow_move_constructible<F>::value && std::is_nothrow_move_constructible<S>::value && std::is_nothrow_move_constructible<T>::value):
            /* Can't use {} on GCC 4.8, see constructHelpers.h for details and
               TripleTest::copyMoveConstructPlainStruct() for a test. */
            #if defined(CORRADE_TARGET_GCC) && !defined(CORRADE_TARGET_CLANG) && __GNUC__ < 5
            _first(Utility::move(first)), _second(Utility::move(second)), _third(Utility::move(third))
            #else
            _first{Utility::move(first)}, _second{Utility::move(second)}, _third{Utility::move(third)}
            #endif
            {}

        /** @brief Copy-construct a triple from another of different type */
        template<class OtherF, class OtherS, class OtherT, class = typename std::enable_if<std::is_constructible<F, const OtherF&>::value && std::is_constructible<S, const OtherS&>::value && std::is_constructible<T, const OtherT&>::value>::type> constexpr explicit Triple(const Triple<OtherF, OtherS, OtherT>& other) noexcept(std::is_nothrow_constructible<F, const OtherF&>::value && std::is_nothrow_constructible<S, const OtherS&>::value && std::is_nothrow_constructible<T, const OtherT&>::value):
            /* Explicit T() to avoid warnings for int-to-float conversion etc.,
               as that's a desirable use case here (and the constructor is
               explicit because of that). Using () instead of {} alone doesn't
               help as Clang still warns for float-to-double conversion.

               Can't use {} on GCC 4.8, see constructHelpers.h for details and
               TripleTest::copyMoveConstructPlainStruct() for a test. */
            #if defined(CORRADE_TARGET_GCC) && !defined(CORRADE_TARGET_CLANG) && __GNUC__ < 5
            _first(F(other._first)), _second(S(other._second)), _third(T(other._third))
            #else
            _first{F(other._first)}, _second{S(other._second)}, _third{T(other._third)}
            #endif
            {}

        /** @brief Move-construct a triple from another of different type */
        template<class OtherF, class OtherS, class OtherT, class = typename std::enable_if<std::is_constructible<F, OtherF&&>::value && std::is_constructible<S, OtherS&&>::value && std::is_constructible<T, OtherT&&>::value>::type> constexpr explicit Triple(Triple<OtherF, OtherS, OtherT>&& other) noexcept(std::is_nothrow_constructible<F, OtherF&&>::value && std::is_nothrow_constructible<S, OtherS&&>::value && std::is_nothrow_constructible<T, OtherT&&>::value):
            /* Explicit T() to avoid conversion warnings, similar to above;
               GCC 4.8 special case also similarly to above although
               copyMoveConstructPlainStruct() cannot really test it (see there
               for details). */
            #if defined(CORRADE_TARGET_GCC) && !defined(CORRADE_TARGET_CLANG) && __GNUC__ < 5
            _first(F(Utility::move(other._first))), _second(S(Utility::move(other._second))), _third(T(Utility::move(other._third)))
            #else
            _first{F(Utility::move(other._first))}, _second{S(Utility::move(other._second))}, _third{T(Utility::move(other._third))}
            #endif
            {}

        /**
         * @brief Copy-construct a triple from external representation
         *
         * @see @ref Containers-Triple-stl, @ref triple(T&&)
         */
        template<class U, class = decltype(Implementation::TripleConverter<F, S, T, U>::from(std::declval<const U&>()))> /*implicit*/ Triple(const U& other) noexcept(std::is_nothrow_copy_constructible<F>::value && std::is_nothrow_copy_constructible<S>::value && std::is_nothrow_copy_constructible<T>::value): Triple{Implementation::TripleConverter<F, S, T, U>::from(other)} {}

        /**
         * @brief Move-construct a triple from external representation
         *
         * @see @ref Containers-Triple-stl, @ref triple(T&&)
         */
        template<class U, class = decltype(Implementation::TripleConverter<F, S, T, U>::from(std::declval<U&&>()))> /*implicit*/ Triple(U&& other) noexcept(std::is_nothrow_move_constructible<F>::value && std::is_nothrow_move_constructible<S>::value && std::is_nothrow_move_constructible<T>::value): Triple{Implementation::TripleConverter<F, S, T, U>::from(Utility::move(other))} {}

        /**
         * @brief Copy-convert the triple to external representation
         *
         * @see @ref Containers-Triple-stl
         */
        template<class U, class = decltype(Implementation::TripleConverter<F, S, T, U>::to(std::declval<const Triple<F, S, T>&>()))> /*implicit*/ operator U() const & {
            return Implementation::TripleConverter<F, S, T, U>::to(*this);
        }

        /**
         * @brief Move-convert the triple to external representation
         *
         * @see @ref Containers-Triple-stl
         */
        template<class U, class = decltype(Implementation::TripleConverter<F, S, T, U>::to(std::declval<Triple<F, S, T>&&>()))> /*implicit*/ operator U() && {
            return Implementation::TripleConverter<F, S, T, U>::to(Utility::move(*this));
        }

        /** @brief Equality comparison */
        constexpr bool operator==(const Triple<F, S, T>& other) const {
            return _first == other._first && _second == other._second && _third == other._third;
        }

        /** @brief Non-equality comparison */
        constexpr bool operator!=(const Triple<F, S, T>& other) const {
            return !operator==(other);
        }

        /** @brief First element */
        CORRADE_CONSTEXPR14 F& first() & { return _first; }
        /* Not F&& because that'd cause nasty dangling reference issues in
           common code. See the accessRvalueLifetimeExtension() test for
           details. */
        CORRADE_CONSTEXPR14 F first() && { return Utility::move(_first); } /**< @overload */
        constexpr const F& first() const & { return _first; } /**< @overload */

        /** @brief Second element */
        CORRADE_CONSTEXPR14 S& second() & { return _second; }
        /* Not S&& because that'd cause nasty dangling reference issues in
           common code. See the accessRvalueLifetimeExtension() test for
           details. */
        CORRADE_CONSTEXPR14 S second() && { return Utility::move(_second); } /**< @overload */
        constexpr const S& second() const & { return _second; } /**< @overload */

        /** @brief Third element */
        CORRADE_CONSTEXPR14 T& third() & { return _third; }
        /* Not T&& because that'd cause nasty dangling reference issues in
           common code. See the accessRvalueLifetimeExtension() test for
           details. */
        CORRADE_CONSTEXPR14 T third() && { return Utility::move(_third); } /**< @overload */
        constexpr const T& third() const & { return _third; } /**< @overload */

        /* No const&& overloads right now. There's one theoretical use case,
           where an API could return a `const Triple<T>`, and then if there
           would be a `first() const&&` overload returning a `T` (and not
           `T&&`), it could get picked over the `const T&`, solving the same
           problem as the `first() &&` above. At the moment I don't see a
           practical reason to return a const value, so this isn't handled. */

    private:
        /* For the conversion constructor */
        template<class, class, class> friend class Triple;

        #if CORRADE_CXX_STANDARD > 201402
        /* For C++17 structured bindings, if StructuredBindings.h is included
           as well. There doesn't seem to be a way to call those directly, and
           I can't find any practical use of std::tuple_size, tuple_element etc
           on C++11 and C++14, so this is defined only for newer standards. */
        template<std::size_t index, typename std::enable_if<index == 0, F>::type* = nullptr> constexpr friend const F& get(const Triple<F, S, T>& value) {
            return value._first;
        }
        template<std::size_t index, typename std::enable_if<index == 0, F>::type* = nullptr> CORRADE_CONSTEXPR14 friend F& get(Triple<F, S, T>& value) {
            return value._first;
        }
        template<std::size_t index, typename std::enable_if<index == 0, F>::type* = nullptr> CORRADE_CONSTEXPR14 friend F&& get(Triple<F, S, T>&& value) {
            return Utility::move(value._first);
        }
        template<std::size_t index, typename std::enable_if<index == 1, S>::type* = nullptr> constexpr friend const S& get(const Triple<F, S, T>& value) {
            return value._second;
        }
        template<std::size_t index, typename std::enable_if<index == 1, S>::type* = nullptr> CORRADE_CONSTEXPR14 friend S& get(Triple<F, S, T>& value) {
            return value._second;
        }
        template<std::size_t index, typename std::enable_if<index == 1, S>::type* = nullptr> CORRADE_CONSTEXPR14 friend S&& get(Triple<F, S, T>&& value) {
            return Utility::move(value._second);
        }
        template<std::size_t index, typename std::enable_if<index == 2, T>::type* = nullptr> constexpr friend const T& get(const Triple<F, S, T>& value) {
            return value._third;
        }
        template<std::size_t index, typename std::enable_if<index == 2, T>::type* = nullptr> CORRADE_CONSTEXPR14 friend T& get(Triple<F, S, T>& value) {
            return value._third;
        }
        template<std::size_t index, typename std::enable_if<index == 2, T>::type* = nullptr> CORRADE_CONSTEXPR14 friend T&& get(Triple<F, S, T>&& value) {
            return Utility::move(value._third);
        }
        #endif

        F _first;
        S _second;
        T _third;
};

/** @relatesalso Triple
@brief Make a triple

Convernience alternative to @ref Triple::Triple(const F&, const S&, const T&)
and overloads. The following two lines are equivalent:

@snippet Containers.cpp triple
*/
template<class F, class S, class T> constexpr Triple<typename std::decay<F>::type, typename std::decay<S>::type, typename std::decay<T>::type> triple(F&& first, S&& second, T&& third) {
    return Triple<typename std::decay<F>::type, typename std::decay<S>::type, typename std::decay<T>::type>{Utility::forward<F>(first), Utility::forward<S>(second), Utility::forward<T>(third)};
}

namespace Implementation {
    template<class> struct DeducedTripleConverter;
}

/** @relatesalso Triple
@brief Make a triple from external representation

@see @ref Containers-Triple-stl
*/
template<class T> inline auto triple(T&& other) -> decltype(Implementation::DeducedTripleConverter<typename std::decay<T>::type>::from(Utility::forward<T>(other))) {
    return Implementation::DeducedTripleConverter<typename std::decay<T>::type>::from(Utility::forward<T>(other));
}

#ifndef CORRADE_SINGLES_NO_DEBUG
/** @debugoperator{Triple} */
template<class F, class S, class T> Utility::Debug& operator<<(Utility::Debug& debug, const Triple<F, S, T>& value) {
    /* Nested values should get printed with the same flags, so make all
       immediate flags temporarily global -- except NoSpace, unless it's also
       set globally */
    const Utility::Debug::Flags prevFlags = debug.flags();
    debug.setFlags(prevFlags | (debug.immediateFlags() & ~Utility::Debug::Flag::NoSpace));

    debug << "{" << Utility::Debug::nospace << value.first() << Utility::Debug::nospace << "," << value.second() << Utility::Debug::nospace << "," << value.third() << Utility::Debug::nospace << "}";

    /* Reset the original flags back */
    debug.setFlags(prevFlags);

    return debug;
}
#endif

}}

#endif
