#ifndef Corrade_Containers_Pair_h
#define Corrade_Containers_Pair_h
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

#include <type_traits>

#include "Corrade/Tags.h"
#ifndef CORRADE_NO_DEBUG
#include "Corrade/Utility/Debug.h"
#endif
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

@section Containers-Pair-stl STL compatibility

Instances of @ref Pair are *explicitly* copy- and move-convertible to and
from @ref std::pair if you include @ref Corrade/Containers/PairStl.h. The
conversion is provided in a separate header to avoid overhead from an
unconditional @cpp #include <utility> @ce. Additionally, the @ref pair(T&&)
overload also allows for such a conversion. Example:

@snippet Containers-stl.cpp Pair

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
         * otherwise. This is the same as @ref Pair().
         * @see @ref ValueInit, @ref Pair(DefaultInitT)
         */
        constexpr explicit Pair(Corrade::ValueInitT) noexcept(std::is_nothrow_constructible<F>::value && std::is_nothrow_constructible<S>::value):
            /* Can't use {} here. See constructHelpers.h for details, test in
               PairTest::emplaceConstructorExplicitInCopyInitialization(). */
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
        explicit Pair(Corrade::NoInitT) noexcept;
        #else
        template<class F_ = F, class = typename std::enable_if<std::is_standard_layout<F_>::value &&  std::is_standard_layout<S>::value && std::is_trivial<F_>::value && std::is_trivial<S>::value>::type> explicit Pair(Corrade::NoInitT) noexcept {}
        template<class F_ = F, class S_ = S, class = typename std::enable_if<std::is_constructible<F_, Corrade::NoInitT>::value &&  std::is_constructible<S_, Corrade::NoInitT>::value>::type> explicit Pair(Corrade::NoInitT) noexcept: _first{Corrade::NoInit}, _second{Corrade::NoInit} {}
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
               MSVC doesn't seem to be affected by the emplaceConstructorExplicitInCopyInitialization() bug in GCC and
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

        /**
         * @brief Copy-construct a pair from external representation
         *
         * @see @ref Containers-Pair-stl, @ref pair(T&&)
         */
        template<class T, class = decltype(Implementation::PairConverter<F, S, T>::from(std::declval<const T&>()))> explicit Pair(const T& other) noexcept(std::is_nothrow_copy_constructible<F>::value && std::is_nothrow_copy_constructible<S>::value): Pair{Implementation::PairConverter<F, S, T>::from(other)} {}

        /**
         * @brief Move-construct a pair from external representation
         *
         * @see @ref Containers-Pair-stl, @ref pair(T&&)
         */
        template<class T, class = decltype(Implementation::PairConverter<F, S, T>::from(std::declval<T&&>()))> explicit Pair(T&& other) noexcept(std::is_nothrow_move_constructible<F>::value && std::is_nothrow_move_constructible<S>::value): Pair{Implementation::PairConverter<F, S, T>::from(Utility::move(other))} {}

        /**
         * @brief Copy-convert the pair to external representation
         *
         * @see @ref Containers-Pair-stl
         */
        template<class T, class = decltype(Implementation::PairConverter<F, S, T>::to(std::declval<const Pair<F, S>&>()))> explicit operator T() const & {
            return Implementation::PairConverter<F, S, T>::to(*this);
        }

        /**
         * @brief Move-convert the pair to external representation
         *
         * @see @ref Containers-Pair-stl
         */
        template<class T, class = decltype(Implementation::PairConverter<F, S, T>::to(std::declval<Pair<F, S>&&>()))> explicit operator T() && {
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
        F&& first() && { return Utility::move(_first); } /**< @overload */
        constexpr const F& first() const & { return _first; } /**< @overload */
        #if !defined(__GNUC__) || defined(__clang__) || __GNUC__ > 4
        /** @overload */
        /* This causes ambiguous overload on GCC 4.8 (and I assume 4.9 as
           well), so disabling it there. See also the corresponding test, same
           is in Optional. */
        constexpr const F&& first() const && { return Utility::move(_first); }
        #endif

        /** @brief Second element */
        S& second() & { return _second; }
        S&& second() && { return Utility::move(_second); } /**< @overload */
        constexpr const S& second() const & { return _second; } /**< @overload */
        #if !defined(__GNUC__) || defined(__clang__) || __GNUC__ > 4
        /** @overload */
        /* This causes ambiguous overload on GCC 4.8 (and I assume 4.9 as
           well), so disabling it there. See also the corresponding test, same
           is in Optional. */
        constexpr const S&& second() const && { return Utility::move(_second); }
        #endif

    private:
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

#ifndef CORRADE_NO_DEBUG
/** @debugoperator{Pair} */
template<class F, class S> Utility::Debug& operator<<(Utility::Debug& debug, const Pair<F, S>& value) {
    return debug << "{" << Utility::Debug::nospace << value.first() << Utility::Debug::nospace << "," << value.second() << Utility::Debug::nospace << "}";
}
#endif

}}

#endif
