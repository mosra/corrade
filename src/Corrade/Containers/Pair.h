#ifndef Corrade_Containers_Pair_h
#define Corrade_Containers_Pair_h
/*
    This file is part of Corrade.

    Copyright © 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016,
                2017, 2018, 2019, 2020, 2021, 2022, 2023
              Vladimír Vondruš <mosra@centrum.cz>
    Copyright © 2022 Stanislaw Halik <sthalik@misaki.pl>

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
 * @brief Class @ref Corrade::Containers::Pair
 * @m_since_latest
 */

namespace Corrade { namespace Containers {

namespace Implementation {
    template<class, class, class> struct PairConverter;
}

/**
@brief Pair of values
@m_since_latest

An alternative to @ref std::pair that is trivially copyable for trivial types,
provides move semantics consistent across standard library implementations and
guarantees usability in @cpp constexpr @ce contexts even in C++11. On the other
hand, to simplify both the implementation and usage semantics, the type doesn't
support references --- wrap them in a @ref Reference in order to store them in
a @ref Pair. Such type composition allows you to both rebind the reference and
update the referenced value and the intent is clear.

Similarly to other containers and equivalently to @ref std::make_pair(),
there's also @ref pair(). The following two lines are equivalent:

@snippet Containers.cpp pair

Unlike with @ref std::pair, access to the pair elements is done using
@ref first() and @ref second() member *functions*, without providing access to
the members directly. This is done in order to future-proof the design and have
extra flexibility in how the internals are defined.

There's also a three-element variant, called a @ref Triple.

@section Containers-Pair-structured-bindings C++17 structured bindings

If @ref Corrade/Containers/StructuredBindings.h is included, the class can be
used in C++17 structured bindings. While the @cpp get<i>() @ce overloads are
defined inside @ref Pair itself, a separate header is used for the
@m_class{m-doc-external} [std::tuple_size](https://en.cppreference.com/w/cpp/utility/tuple_size)
and @m_class{m-doc-external} [std::tuple_element](https://en.cppreference.com/w/cpp/utility/tuple_element)
template specializations, as those may require @cpp #include <utility> @ce on
some STL implementations. Example:

@snippet Containers-cpp17.cpp Pair-structured-bindings

@section Containers-Pair-stl STL compatibility

Instances of @ref Pair are implicitly copy- and move-convertible to and from
@ref std::pair if you include @ref Corrade/Containers/PairStl.h. The
conversion is provided in a separate header to avoid overhead from an
unconditional @cpp #include <utility> @ce. Additionally, the @ref pair(T&&)
overload also allows for such a conversion. Example:

@snippet Containers-stl.cpp Pair

<b></b>

@m_class{m-block m-success}

@par Single-header version
    This class is also available as a single-header, dependency-less
    [CorradePair.h](https://github.com/mosra/magnum-singles/tree/master/CorradePair.h)
    library in the Magnum Singles repository for easier integration into your
    projects. See @ref corrade-singles for more information. The above
    mentioned STL compatibility is included as well, but disabled by default.
    Enable it by specifying @cpp #define CORRADE_PAIR_STL_COMPATIBILITY @ce
    before including the file. Including it multiple times with different
    macros defined works as well.

@see @ref pair(F&&, S&&), @ref pair(T&&)
*/
template<class F, class S> class Pair {
    static_assert(!std::is_lvalue_reference<F>::value && !std::is_lvalue_reference<S>::value, "use a Reference<T> to store a T& in a Pair");

    public:
        typedef F FirstType;    /**< @brief First type */
        typedef S SecondType;   /**< @brief Second type */

        /**
         * @brief Construct a default-initialized pair
         *
         * Trivial types are not initialized, default constructor called
         * otherwise. Because of the differing behavior for trivial types it's
         * better to explicitly use either the @ref Pair(ValueInitT) or the
         * @ref Pair(NoInitT) variant instead.
         * @see @ref DefaultInit, @ref std::is_trivial
         */
        #ifndef CORRADE_MSVC2015_COMPATIBILITY
        /* Not constexpr for this joke of a compiler because I don't explicitly
           initialize _first and _second, which wouldn't be a default
           initialization if I did that. */
        constexpr
        #endif
        explicit Pair(Corrade::DefaultInitT) noexcept(std::is_nothrow_constructible<F>::value && std::is_nothrow_constructible<S>::value) {}

        /**
         * @brief Construct a value-initialized pair
         *
         * Trivial types are zero-initialized, default constructor called
         * otherwise. This is the same as the default constructor.
         * @see @ref ValueInit, @ref Pair(DefaultInitT)
         */
        constexpr explicit Pair(Corrade::ValueInitT) noexcept(std::is_nothrow_constructible<F>::value && std::is_nothrow_constructible<S>::value):
            /* Can't use {} here. See constructHelpers.h for details, test in
               PairTest::constructorExplicitInCopyInitialization(). */
            _first(), _second() {}

        #ifdef DOXYGEN_GENERATING_OUTPUT
        /**
         * @brief Construct a pair without initializing its contents
         *
         * Enabled only for trivial types and types that implement the
         * @ref NoInit constructor. The contents are *not* initialized. Useful
         * if you will be overwriting both members later anyway or if you need
         * to initialize in a way that's not expressible via any other
         * @ref Pair constructor.
         *
         * For trivial types is equivalent to @ref Pair(DefaultInitT).
         */
        explicit Pair(Corrade::NoInitT) noexcept(std::is_nothrow_constructible<F, Corrade::NoInitT>::value && std::is_nothrow_constructible<S, Corrade::NoInitT>::value);
        #else
        template<class F_ = F, class = typename std::enable_if<std::is_standard_layout<F_>::value && std::is_standard_layout<S>::value && std::is_trivial<F_>::value && std::is_trivial<S>::value>::type> explicit Pair(Corrade::NoInitT) noexcept {}
        /** @todo support combined trivial & NoInit variants once we figure out
            how to express the overloads to not conflict with each other */
        template<class F_ = F, class S_ = S, class = typename std::enable_if<std::is_constructible<F_, Corrade::NoInitT>::value && std::is_constructible<S_, Corrade::NoInitT>::value>::type> explicit Pair(Corrade::NoInitT) noexcept(std::is_nothrow_constructible<F, Corrade::NoInitT>::value && std::is_nothrow_constructible<S, Corrade::NoInitT>::value): _first{Corrade::NoInit}, _second{Corrade::NoInit} {}
        #endif

        /**
         * @brief Default constructor
         *
         * Alias to @ref Pair(ValueInitT).
         */
        constexpr /*implicit*/ Pair() noexcept(std::is_nothrow_constructible<F>::value && std::is_nothrow_constructible<S>::value):
            #ifdef CORRADE_MSVC2015_COMPATIBILITY
            /* Otherwise it complains that _first and _second isn't initialized
               in a constexpr context. Does it not see the delegation?! OTOH
               MSVC 2015 doesn't seem to be affected by the
               constructorExplicitInCopyInitialization() bug in GCC and
               Clang, so I can use {} here I think. */
            _first{}, _second{}
            #else
            Pair{Corrade::ValueInit}
            #endif
        {}

        /**
         * @brief Constructor
         *
         * @see @ref pair(F&&, S&&)
         */
        constexpr /*implicit*/ Pair(const F& first, const S& second) noexcept(std::is_nothrow_copy_constructible<F>::value && std::is_nothrow_copy_constructible<S>::value):
            /* Can't use {} on GCC 4.8, see constructHelpers.h for details and
               PairTest::copyMoveConstructPlainStruct() for a test. */
            #if defined(CORRADE_TARGET_GCC) && !defined(CORRADE_TARGET_CLANG) && __GNUC__ < 5
            _first(first), _second(second)
            #else
            _first{first}, _second{second}
            #endif
            {}
        /** @overload */
        constexpr /*implicit*/ Pair(const F& first, S&& second) noexcept(std::is_nothrow_copy_constructible<F>::value && std::is_nothrow_move_constructible<S>::value):
            /* Can't use {} on GCC 4.8, see constructHelpers.h for details and
               PairTest::copyMoveConstructPlainStruct() for a test. */
            #if defined(CORRADE_TARGET_GCC) && !defined(CORRADE_TARGET_CLANG) && __GNUC__ < 5
            _first(first), _second(Utility::move(second))
            #else
            _first{first}, _second{Utility::move(second)}
            #endif
            {}
        /** @overload */
        constexpr /*implicit*/ Pair(F&& first, const S& second) noexcept(std::is_nothrow_move_constructible<F>::value && std::is_nothrow_copy_constructible<S>::value):
            /* Can't use {} on GCC 4.8, see constructHelpers.h for details and
               PairTest::copyMoveConstructPlainStruct() for a test. */
            #if defined(CORRADE_TARGET_GCC) && !defined(CORRADE_TARGET_CLANG) && __GNUC__ < 5
            _first(Utility::move(first)), _second(second)
            #else
            _first{Utility::move(first)}, _second{second}
            #endif
            {}
        /** @overload */
        constexpr /*implicit*/ Pair(F&& first, S&& second) noexcept(std::is_nothrow_move_constructible<F>::value && std::is_nothrow_move_constructible<S>::value):
            /* Can't use {} on GCC 4.8, see constructHelpers.h for details and
               PairTest::copyMoveConstructPlainStruct() for a test. */
            #if defined(CORRADE_TARGET_GCC) && !defined(CORRADE_TARGET_CLANG) && __GNUC__ < 5
            _first(Utility::move(first)), _second(Utility::move(second))
            #else
            _first{Utility::move(first)}, _second{Utility::move(second)}
            #endif
            {}

        /** @brief Copy-construct a pair from another of different type */
        template<class OtherF, class OtherS, class = typename std::enable_if<std::is_constructible<F, const OtherF&>::value && std::is_constructible<S, const OtherS&>::value>::type> constexpr explicit Pair(const Pair<OtherF, OtherS>& other) noexcept(std::is_nothrow_constructible<F, const OtherF&>::value && std::is_nothrow_constructible<S, const OtherS&>::value):
            /* Explicit T() to avoid warnings for int-to-float conversion etc.,
               as that's a desirable use case here (and the constructor is
               explicit because of that). Using () instead of {} alone doesn't
               help as Clang still warns for float-to-double conversion.

               Can't use {} on GCC 4.8, see constructHelpers.h for details and
               PairTest::copyMoveConstructPlainStruct() for a test. */
            #if defined(CORRADE_TARGET_GCC) && !defined(CORRADE_TARGET_CLANG) && __GNUC__ < 5
            _first(F(other._first)), _second(S(other._second))
            #else
            _first{F(other._first)}, _second{S(other._second)}
            #endif
            {}

        /** @brief Move-construct a pair from another of different type */
        template<class OtherF, class OtherS, class = typename std::enable_if<std::is_constructible<F, OtherF&&>::value && std::is_constructible<S, OtherS&&>::value>::type> constexpr explicit Pair(Pair<OtherF, OtherS>&& other) noexcept(std::is_nothrow_constructible<F, OtherF&&>::value && std::is_nothrow_constructible<S, OtherS&&>::value):
            /* Explicit T() to avoid conversion warnings, similar to above;
               GCC 4.8 special case also similarly to above although
               copyMoveConstructPlainStruct() cannot really test it (see there
               for details). */
            #if defined(CORRADE_TARGET_GCC) && !defined(CORRADE_TARGET_CLANG) && __GNUC__ < 5
            _first(F(Utility::move(other._first))), _second(S(Utility::move(other._second)))
            #else
            _first{F(Utility::move(other._first))}, _second{S(Utility::move(other._second))}
            #endif
            {}

        /**
         * @brief Copy-construct a pair from external representation
         *
         * @see @ref Containers-Pair-stl, @ref pair(T&&)
         */
        template<class T, class = decltype(Implementation::PairConverter<F, S, T>::from(std::declval<const T&>()))> /*implicit*/ Pair(const T& other) noexcept(std::is_nothrow_copy_constructible<F>::value && std::is_nothrow_copy_constructible<S>::value): Pair{Implementation::PairConverter<F, S, T>::from(other)} {}

        /**
         * @brief Move-construct a pair from external representation
         *
         * @see @ref Containers-Pair-stl, @ref pair(T&&)
         */
        template<class T, class = decltype(Implementation::PairConverter<F, S, T>::from(std::declval<T&&>()))> /*implicit*/ Pair(T&& other) noexcept(std::is_nothrow_move_constructible<F>::value && std::is_nothrow_move_constructible<S>::value): Pair{Implementation::PairConverter<F, S, T>::from(Utility::move(other))} {}

        /**
         * @brief Copy-convert the pair to external representation
         *
         * @see @ref Containers-Pair-stl
         */
        template<class T, class = decltype(Implementation::PairConverter<F, S, T>::to(std::declval<const Pair<F, S>&>()))> /*implicit*/ operator T() const & {
            return Implementation::PairConverter<F, S, T>::to(*this);
        }

        /**
         * @brief Move-convert the pair to external representation
         *
         * @see @ref Containers-Pair-stl
         */
        template<class T, class = decltype(Implementation::PairConverter<F, S, T>::to(std::declval<Pair<F, S>&&>()))> /*implicit*/ operator T() && {
            return Implementation::PairConverter<F, S, T>::to(Utility::move(*this));
        }

        /** @brief Equality comparison */
        constexpr bool operator==(const Pair<F, S>& other) const {
            return _first == other._first && _second == other._second;
        }

        /** @brief Non-equality comparison */
        constexpr bool operator!=(const Pair<F, S>& other) const {
            return !operator==(other);
        }

        /** @brief First element */
        F& first() & { return _first; }
        /* Not F&& because that'd cause nasty dangling reference issues in
           common code. See the accessRvalueLifetimeExtension() test for
           details. */
        F first() && { return Utility::move(_first); } /**< @overload */
        constexpr const F& first() const & { return _first; } /**< @overload */

        /** @brief Second element */
        S& second() & { return _second; }
        /* Not S&& because that'd cause nasty dangling reference issues in
           common code. See the accessRvalueLifetimeExtension() test for
           details. */
        S second() && { return Utility::move(_second); } /**< @overload */
        constexpr const S& second() const & { return _second; } /**< @overload */

        /* No const&& overloads right now. There's one theoretical use case,
           where an API could return a `const Pair<T>`, and then if there
           would be a `first() const&&` overload returning a `T` (and not
           `T&&`), it could get picked over the `const T&`, solving the same
           problem as the `first() &&` above. I don't see a practical reason to
           return a const value, so this isn't handled at the moment. */

    private:
        /* For the conversion constructor */
        template<class, class> friend class Pair;

        #if CORRADE_CXX_STANDARD > 201402
        /* For C++17 structured bindings, if StructuredBindings.h is included
           as well. There doesn't seem to be a way to call those directly, and
           I can't find any practical use of std::tuple_size, tuple_element etc
           on C++11 and C++14, so this is defined only for newer standards. */
        template<std::size_t index, typename std::enable_if<index == 0, F>::type* = nullptr> constexpr friend const F& get(const Pair<F, S>& value) {
            return value._first;
        }
        template<std::size_t index, typename std::enable_if<index == 0, F>::type* = nullptr> CORRADE_CONSTEXPR14 friend F& get(Pair<F, S>& value) {
            return value._first;
        }
        template<std::size_t index, typename std::enable_if<index == 0, F>::type* = nullptr> CORRADE_CONSTEXPR14 friend F&& get(Pair<F, S>&& value) {
            return Utility::move(value._first);
        }
        template<std::size_t index, typename std::enable_if<index == 1, S>::type* = nullptr> constexpr friend const S& get(const Pair<F, S>& value) {
            return value._second;
        }
        template<std::size_t index, typename std::enable_if<index == 1, S>::type* = nullptr> CORRADE_CONSTEXPR14 friend S& get(Pair<F, S>& value) {
            return value._second;
        }
        template<std::size_t index, typename std::enable_if<index == 1, S>::type* = nullptr> CORRADE_CONSTEXPR14 friend S&& get(Pair<F, S>&& value) {
            return Utility::move(value._second);
        }
        #endif

        F _first;
        S _second;
};

/** @relatesalso Pair
@brief Make a pair

Convernience alternative to @ref Pair::Pair(const F&, const S&) and overloads.
The following two lines are equivalent:

@snippet Containers.cpp pair
*/
template<class F, class S> constexpr Pair<typename std::decay<F>::type, typename std::decay<S>::type> pair(F&& first, S&& second) {
    return Pair<typename std::decay<F>::type, typename std::decay<S>::type>{Utility::forward<F>(first), Utility::forward<S>(second)};
}

namespace Implementation {
    template<class> struct DeducedPairConverter;
}

/** @relatesalso Pair
@brief Make a pair from external representation

@see @ref Containers-Pair-stl
*/
template<class T> inline auto pair(T&& other) -> decltype(Implementation::DeducedPairConverter<typename std::decay<T>::type>::from(Utility::forward<T>(other))) {
    return Implementation::DeducedPairConverter<typename std::decay<T>::type>::from(Utility::forward<T>(other));
}

#ifndef CORRADE_SINGLES_NO_DEBUG
/** @debugoperator{Pair} */
template<class F, class S> Utility::Debug& operator<<(Utility::Debug& debug, const Pair<F, S>& value) {
    /* Nested values should get printed with the same flags, so make all
       immediate flags temporarily global -- except NoSpace, unless it's also
       set globally */
    const Utility::Debug::Flags prevFlags = debug.flags();
    debug.setFlags(prevFlags | (debug.immediateFlags() & ~Utility::Debug::Flag::NoSpace));

    debug << "{" << Utility::Debug::nospace << value.first() << Utility::Debug::nospace << "," << value.second() << Utility::Debug::nospace << "}";

    /* Reset the original flags back */
    debug.setFlags(prevFlags);

    return debug;
}
#endif

}}

#endif
