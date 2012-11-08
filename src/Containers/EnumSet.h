#ifndef Corrade_Containers_EnumSet_h
#define Corrade_Containers_EnumSet_h
/*
    Copyright © 2007, 2008, 2009, 2010, 2011, 2012
              Vladimír Vondruš <mosra@centrum.cz>

    This file is part of Corrade.

    Corrade is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License version 3
    only, as published by the Free Software Foundation.

    Corrade is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU Lesser General Public License version 3 for more details.
*/

/** @file
 * @brief Class Corrade::Containers::EnumSet
 */

#include <type_traits>

#include "corradeCompatibility.h"

namespace Corrade { namespace Containers {

/**
@brief Set of enum values
@tparam T           Enum type
@tparam U           Underlying type of the enum
@tparam fullValue   All enum values together. Defaults to all bits set to `1`.

Provides strongly-typed set-like functionality for strongly typed enums, such
as binary OR and AND operations. The only requirement for enum type is that
all the values must be binary exclusive.

@anchor EnumSet-out-of-class-operators
Desired usage is via typedef'ing. You should also call
CORRADE_ENUMSET_OPERATORS() macro with the resulting type as parameter to have
also out-of-class operators defined:
@code
enum class Feature: unsigned int {
    Fast = 1 << 0,
    Cheap = 1 << 1,
    Tested = 1 << 2,
    Popular = 1 << 3
};

typedef EnumSet<Feature, unsigned int> Features;
CORRADE_ENUMSET_OPERATORS(Features)
@endcode

@anchor EnumSet-friend-operators
If you have the EnumSet as private or protected member of any class, you have
to declare the out-of-class operators as friends. It can be done with
CORRADE_ENUMSET_FRIEND_OPERATORS() macro:
@code
class Application {
    private:
        enum class Flag: unsigned int {
            Redraw = 1 << 0,
            Exit = 1 << 1
        };

        typedef EnumSet<Flag, unsigned int> Flags;
        CORRADE_ENUMSET_FRIEND_OPERATORS(Flags)
};

CORRADE_ENUMSET_OPERATORS(Application::Flags)
@endcode

One thing these macros cannot do is to provide operators for enum sets inside
templated classes. If the enum values are not depending on the template, you
can work around the issue by declaring the enum in some hidden namespace
outside the class and then typedef'ing it back into the class:
@code
namespace Implementation {
    enum class ObjectFlag: unsigned int {
        Dirty = 1 << 0,
        Marked = 1 << 1
    };

    typedef EnumSet<ObjectFlag, unsigned int> ObjectFlags;
    CORRADE_ENUMSET_OPERATORS(ObjectFlags)
}

template<class T> class Object {
    public:
        typedef Implementation::ObjectFlag Flag;
        typedef Implementation::ObjectFlags Flags;
};
@endcode
*/
template<class T, class U, U fullValue = ~U(0)> class EnumSet {
    static_assert(std::is_enum<T>::value && !std::is_convertible<T, U>::value, "EnumSet type must be strongly typed enum");

    public:
        typedef T Type;             /**< @brief Enum type */
        typedef U UnderlyingType;   /**< @brief Underlying type of the enum */

        /** @brief Create empty set */
        inline constexpr EnumSet(): value() {}

        /** @brief Create set from one value */
        inline constexpr EnumSet(T value): value(static_cast<UnderlyingType>(value)) {}

        /** @brief Equality operator */
        inline constexpr bool operator==(EnumSet<T, U, fullValue> other) const {
            return value == other.value;
        }

        /** @brief Non-equality operator */
        inline constexpr bool operator!=(EnumSet<T, U, fullValue> other) const {
            return !operator==(other);
        }

        /**
         * @brief Whether @p other is subset of this
         *
         * Equivalent to `a & other == other`
         */
        inline constexpr bool operator>=(EnumSet<T, U, fullValue> other) const {
            return (*this & other) == other;
        }

        /**
         * @brief Whether @p other is superset of this
         *
         * Equivalent to `a & other == a`
         */
        inline constexpr bool operator<=(EnumSet<T, U, fullValue> other) const {
            return (*this & other) == *this;
        }

        /** @brief Union of two sets */
        inline constexpr EnumSet<T, U, fullValue> operator|(EnumSet<T, U, fullValue> other) const {
            return EnumSet<T, U, fullValue>(value | other.value);
        }

        /** @brief Union two sets and assign */
        inline EnumSet<T, U, fullValue>& operator|=(EnumSet<T, U, fullValue> other) {
            value |= other.value;
            return *this;
        }

        /** @brief Intersection of two sets */
        inline constexpr EnumSet<T, U, fullValue> operator&(EnumSet<T, U, fullValue> other) const {
            return EnumSet<T, U, fullValue>(value & other.value);
        }

        /** @brief Intersect two sets and assign */
        inline EnumSet<T, U, fullValue>& operator&=(EnumSet<T, U, fullValue> other) {
            value &= other.value;
            return *this;
        }

        /** @brief Set complement */
        inline constexpr EnumSet<T, U, fullValue> operator~() const {
            return EnumSet<T, U, fullValue>(fullValue & ~value);
        }

        /** @brief Value as boolean */
        #ifndef CORRADE_GCC44_COMPATIBILITY
        inline constexpr explicit operator bool() const {
        #else /* GCC 4.4 doesn't support explicit conversion operators */
        inline constexpr operator bool() const {
        #endif
            return value != 0;
        }

        /** @brief Value in underlying type */
        #ifndef CORRADE_GCC44_COMPATIBILITY
        inline constexpr explicit operator UnderlyingType() const {
        #else /* GCC 4.4 doesn't support explicit conversion operators */
        inline constexpr operator UnderlyingType() const {
        #endif
            return value;
        }

    private:
        inline constexpr explicit EnumSet(UnderlyingType type): value(type) {}

        UnderlyingType value;
};

/** @hideinitializer
@brief Define out-of-class operators for given EnumSet

See @ref EnumSet-out-of-class-operators "EnumSet documentation" for example
usage.
*/
#define CORRADE_ENUMSET_OPERATORS(class)                                    \
    inline constexpr bool operator==(class::Type a, class b) {              \
        return class(a) == b;                                               \
    }                                                                       \
    inline constexpr bool operator!=(class::Type a, class b) {              \
        return class(a) != b;                                               \
    }                                                                       \
    inline constexpr bool operator>=(class::Type a, class b) {              \
        return class(a) >= b;                                               \
    }                                                                       \
    inline constexpr bool operator<=(class::Type a, class b) {              \
        return class(a) <= b;                                               \
    }                                                                       \
    inline constexpr class operator|(class::Type a, class b) {              \
        return b | a;                                                       \
    }                                                                       \
    inline constexpr class operator&(class::Type a, class b) {              \
        return b & a;                                                       \
    }                                                                       \
    inline constexpr class operator~(class::Type a) {                       \
        return ~class(a);                                                   \
    }

/** @hideinitializer
@brief Define out-of-class operators for given EnumSet as friends of encapsulating class

See @ref EnumSet-friend-operators "EnumSet documentation" for example usage.
*/
#define CORRADE_ENUMSET_FRIEND_OPERATORS(class)                             \
    friend constexpr bool operator==(class::Type, class);                   \
    friend constexpr bool operator!=(class::Type, class);                   \
    friend constexpr bool operator>=(class::Type, class);                   \
    friend constexpr bool operator<=(class::Type, class);                   \
    friend constexpr class operator&(class::Type, class);                   \
    friend constexpr class operator|(class::Type, class);                   \
    friend constexpr class operator~(class::Type);

}}

#endif
