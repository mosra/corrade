#ifndef Corrade_Containers_Triple_h
#define Corrade_Containers_Triple_h
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

#include <type_traits>

#include "Corrade/Tags.h"
#ifndef CORRADE_NO_DEBUG
#include "Corrade/Utility/Debug.h"
#endif
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

@section Containers-Triple-stl STL compatibility

Instances of @ref Triple are implicitly copy- and move-convertible to and from
a three-element @ref std::tuple if you include
@ref Corrade/Containers/TripleStl.h. The conversion is provided in a separate
header to avoid overhead from an unconditional @cpp #include <tuple> @ce.
Additionally, the @ref triple(T&&) overload also allows for such a conversion.
Example:

@snippet Containers-stl.cpp Triple

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
         * otherwise. This is the same as @ref Triple().
         * @see @ref ValueInit, @ref Triple(DefaultInitT)
         */
        constexpr explicit Triple(Corrade::ValueInitT) noexcept(std::is_nothrow_constructible<F>::value && std::is_nothrow_constructible<S>::value && std::is_nothrow_constructible<T>::value):
            /* Can't use {} here. See constructHelpers.h for details, test in
               TripleTest::emplaceConstructorExplicitInCopyInitialization(). */
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
               MSVC doesn't seem to be affected by the emplaceConstructorExplicitInCopyInitialization() bug in GCC and
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

        /** @brief Third element */
        T& third() & { return _third; }
        /* Not T&& because that'd cause nasty dangling reference issues in
           common code. See the accessRvalueLifetimeExtension() test for
           details. */
        T third() && { return Utility::move(_third); } /**< @overload */
        constexpr const T& third() const & { return _third; } /**< @overload */

        /* No const&& overloads right now. There's one theoretical use case,
           where an API could return a `const Triple<T>`, and then if there
           would be a `first() const&&` overload returning a `T` (and not
           `T&&`), it could get picked over the `const T&`, solving the same
           problem as the `first() &&` above. At the moment I don't see a
           practical reason to return a const value, so this isn't handled. */

    private:
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

#ifndef CORRADE_NO_DEBUG
/** @debugoperator{Triple} */
template<class F, class S, class T> Utility::Debug& operator<<(Utility::Debug& debug, const Triple<F, S, T>& value) {
    return debug << "{" << Utility::Debug::nospace << value.first() << Utility::Debug::nospace << "," << value.second() << Utility::Debug::nospace << "," << value.third() << Utility::Debug::nospace << "}";
}
#endif

}}

#endif
