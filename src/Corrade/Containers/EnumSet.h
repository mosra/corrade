#ifndef Corrade_Containers_EnumSet_h
#define Corrade_Containers_EnumSet_h
/*
    This file is part of Corrade.

    Copyright © 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016,
                2017 Vladimír Vondruš <mosra@centrum.cz>

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
 */

#include <type_traits>

#include "Corrade/Containers/Containers.h"
#include "Corrade/Containers/Tags.h"

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
@ref CORRADE_ENUMSET_OPERATORS() macro with the resulting type as parameter to
have out-of-class operators defined:
@code
enum class Feature: unsigned int {
    Fast = 1 << 0,
    Cheap = 1 << 1,
    Tested = 1 << 2,
    Popular = 1 << 3
};

typedef EnumSet<Feature> Features;
CORRADE_ENUMSET_OPERATORS(Features)
@endcode

@anchor EnumSet-friend-operators
If you have the EnumSet as private or protected member of any class, you have
to declare the out-of-class operators as friends. It can be done with
@ref CORRADE_ENUMSET_FRIEND_OPERATORS() macro:
@code
class Application {
    private:
        enum class Flag: unsigned int {
            Redraw = 1 << 0,
            Exit = 1 << 1
        };

        typedef EnumSet<Flag> Flags;
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

    typedef EnumSet<ObjectFlag> ObjectFlags;
    CORRADE_ENUMSET_OPERATORS(ObjectFlags)
}

template<class T> class Object {
    public:
        typedef Implementation::ObjectFlag Flag;
        typedef Implementation::ObjectFlags Flags;
};
@endcode

@see @ref enumSetDebugOutput()
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
         * @brief Whether @p other is subset of this
         *
         * Equivalent to `a & other == other`
         */
        constexpr bool operator>=(EnumSet<T, fullValue> other) const {
            return (*this & other) == other;
        }

        /**
         * @brief Whether @p other is superset of this
         *
         * Equivalent to `a & other == a`
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
    friend constexpr bool operator==(class::Type, class);                   \
    friend constexpr bool operator!=(class::Type, class);                   \
    friend constexpr bool operator>=(class::Type, class);                   \
    friend constexpr bool operator<=(class::Type, class);                   \
    friend constexpr class operator&(class::Type, class);                   \
    friend constexpr class operator|(class::Type, class);                   \
    friend constexpr class operator^(class::Type, class);                   \
    friend constexpr class operator~(class::Type);

}}

#endif
