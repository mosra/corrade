#ifndef Corrade_Containers_EnumSet_h
#define Corrade_Containers_EnumSet_h
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
 * @brief Class @ref Corrade::Containers::EnumSet
 * @see @ref Corrade/Containers/EnumSet.hpp
 */

#include <type_traits>

#include "Corrade/Containers/Containers.h" /* for template default args */
#include "Corrade/Containers/Tags.h"

namespace Corrade { namespace Containers {

/**
@brief Set of enum values
@tparam T           Enum type
@tparam fullValue   All enum values together. Defaults to all bits set to `1`.

Provides strongly-typed set-like functionality for strongly typed enums, such
as binary OR and AND operations. The only requirement for the enum type is that
all the values must be binary exclusive.

@anchor EnumSet-out-of-class-operators

Desired usage is via @cpp typedef @ce'ing. You should also call the
@ref CORRADE_ENUMSET_OPERATORS() macro with the resulting type as a parameter
to have out-of-class operators defined:

@snippet Containers.cpp EnumSet-usage

@anchor EnumSet-friend-operators

You can have the @ref EnumSet as a private or protected member of any class.
The only difference is that you need to call
@ref CORRADE_ENUMSET_FRIEND_OPERATORS() inside the class. *Do not* combine it
with the @ref CORRADE_ENUMSET_OPERATORS() in this case, you'd get duplicate
definitions. This macro works with templated classes as well.

@snippet Containers.cpp EnumSet-friend

@see @ref enumCastUnderlyingType(), @ref enumSetDebugOutput()
*/
#ifdef DOXYGEN_GENERATING_OUTPUT
template<class T, typename std::underlying_type<T>::type fullValue = typename std::underlying_type<T>::type(~0)>
#else
template<class T, typename std::underlying_type<T>::type fullValue>
#endif
class EnumSet {
    static_assert(std::is_enum<T>::value, "EnumSet type must be strongly typed enum");

    public:
        typedef T Type; /**< @brief Enum type */

        /** @brief Underlying type of the enum */
        typedef typename std::underlying_type<T>::type UnderlyingType;

        enum: UnderlyingType {
            FullValue = fullValue /**< All enum values together */
        };

        /** @brief Create empty set */
        constexpr /*implicit*/ EnumSet(): value() {}

        /** @brief Create set from one value */
        constexpr /*implicit*/ EnumSet(T value): value(static_cast<UnderlyingType>(value)) {}

        /**
         * @brief Create uninitialized set
         *
         * The contents are left in undefined state.
         */
        explicit EnumSet(NoInitT) {}

        /** @brief Equality operator */
        constexpr bool operator==(EnumSet<T, fullValue> other) const {
            return value == other.value;
        }

        /** @brief Non-equality operator */
        constexpr bool operator!=(EnumSet<T, fullValue> other) const {
            return !operator==(other);
        }

        /**
         * @brief Whether @p other is a subset of this (@f$ a \supseteq o @f$)
         *
         * Equivalent to @cpp (a & other) == other @ce.
         */
        constexpr bool operator>=(EnumSet<T, fullValue> other) const {
            return (*this & other) == other;
        }

        /**
         * @brief Whether @p other is a superset of this (@f$ a \subseteq o @f$)
         *
         * Equivalent to @cpp (a & other) == a @ce.
         */
        constexpr bool operator<=(EnumSet<T, fullValue> other) const {
            return (*this & other) == *this;
        }

        /** @brief Union of two sets */
        constexpr EnumSet<T, fullValue> operator|(EnumSet<T, fullValue> other) const {
            return EnumSet<T, fullValue>(value | other.value);
        }

        /** @brief Union two sets and assign */
        EnumSet<T, fullValue>& operator|=(EnumSet<T, fullValue> other) {
            value |= other.value;
            return *this;
        }

        /** @brief Intersection of two sets */
        constexpr EnumSet<T, fullValue> operator&(EnumSet<T, fullValue> other) const {
            return EnumSet<T, fullValue>(value & other.value);
        }

        /** @brief Intersect two sets and assign */
        EnumSet<T, fullValue>& operator&=(EnumSet<T, fullValue> other) {
            value &= other.value;
            return *this;
        }

        /** @brief XOR of two sets */
        constexpr EnumSet<T, fullValue> operator^(EnumSet<T, fullValue> other) const {
            return EnumSet<T, fullValue>(value ^ other.value);
        }

        /** @brief XOR two sets and assign */
        EnumSet<T, fullValue>& operator^=(EnumSet<T, fullValue> other) {
            value ^= other.value;
            return *this;
        }

        /** @brief Set complement */
        constexpr EnumSet<T, fullValue> operator~() const {
            return EnumSet<T, fullValue>(fullValue & ~value);
        }

        /** @brief Value as boolean */
        constexpr explicit operator bool() const {
            return value != 0;
        }

        /** @brief Value in underlying type */
        constexpr explicit operator UnderlyingType() const {
            return value;
        }

    private:
        constexpr explicit EnumSet(UnderlyingType type): value(type) {}

        UnderlyingType value;
};

/** @relatesalso EnumSet
@brief Cast an enum to its underlying type
@m_since{2020,06}

@see @ref std::underlying_type
*/
template<class T, class = typename std::enable_if<std::is_enum<T>::value>::type> constexpr typename std::underlying_type<T>::type enumCastUnderlyingType(T value) {
    return typename std::underlying_type<T>::type(value);
}

/** @relatesalso EnumSet
@brief Cast an enum set to its underlying type
@m_since{2020,06}

@see @ref std::underlying_type
*/
template<class T, typename std::underlying_type<T>::type fullValue> constexpr typename std::underlying_type<T>::type enumCastUnderlyingType(EnumSet<T, fullValue> value) {
    return typename std::underlying_type<T>::type(value);
}

/** @hideinitializer
@brief Define out-of-class operators for given @ref Corrade::Containers::EnumSet "EnumSet"

See @ref EnumSet-out-of-class-operators "EnumSet documentation" for example
usage.
*/
#define CORRADE_ENUMSET_OPERATORS(class)                                    \
    constexpr bool operator==(class::Type a, class b) {                     \
        return class(a) == b;                                               \
    }                                                                       \
    constexpr bool operator!=(class::Type a, class b) {                     \
        return class(a) != b;                                               \
    }                                                                       \
    constexpr bool operator>=(class::Type a, class b) {                     \
        return class(a) >= b;                                               \
    }                                                                       \
    constexpr bool operator<=(class::Type a, class b) {                     \
        return class(a) <= b;                                               \
    }                                                                       \
    constexpr class operator|(class::Type a, class b) {                     \
        return b | a;                                                       \
    }                                                                       \
    constexpr class operator&(class::Type a, class b) {                     \
        return b & a;                                                       \
    }                                                                       \
    constexpr class operator^(class::Type a, class b) {                     \
        return b ^ a;                                                       \
    }                                                                       \
    constexpr class operator~(class::Type a) {                              \
        return ~class(a);                                                   \
    }

/** @hideinitializer
@brief Define out-of-class operators for given @ref Corrade::Containers::EnumSet "EnumSet" as friends of encapsulating class

See @ref EnumSet-friend-operators "EnumSet documentation" for example usage.
*/
#define CORRADE_ENUMSET_FRIEND_OPERATORS(class)                             \
    friend constexpr bool operator==(typename class::Type a, class b) {     \
        return class(a) == b;                                               \
    }                                                                       \
    friend constexpr bool operator!=(typename class::Type a, class b) {     \
        return class(a) != b;                                               \
    }                                                                       \
    friend constexpr bool operator>=(typename class::Type a, class b) {     \
        return class(a) >= b;                                               \
    }                                                                       \
    friend constexpr bool operator<=(typename class::Type a, class b) {     \
        return class(a) <= b;                                               \
    }                                                                       \
    friend constexpr class operator|(typename class::Type a, class b) {     \
        return b | a;                                                       \
    }                                                                       \
    friend constexpr class operator&(typename class::Type a, class b) {     \
        return b & a;                                                       \
    }                                                                       \
    friend constexpr class operator^(typename class::Type a, class b) {     \
        return b ^ a;                                                       \
    }                                                                       \
    friend constexpr class operator~(typename class::Type a) {              \
        return ~class(a);                                                   \
    }

}}

#endif
