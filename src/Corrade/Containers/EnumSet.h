#ifndef Corrade_Containers_EnumSet_h
#define Corrade_Containers_EnumSet_h
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

/** @file
 * @brief Class @ref Corrade::Containers::EnumSet, macro @ref CORRADE_ENUMSET_OPERATORS(), @ref CORRADE_ENUMSET_FRIEND_OPERATORS()
 * @see @ref Corrade/Containers/EnumSet.hpp
 */

#include <type_traits>

#include "Corrade/Tags.h"
#include "Corrade/Containers/Containers.h" /* for template default args */

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

<b></b>

@m_class{m-block m-success}

@par Storing more than 64 values
    The @ref EnumSet is limited by the maximum size of a builtin type, which is
    64 bits. 128-bit types are available on some platforms, but are not
    standard and thus not portable. If you need to store a larger set and
    you're fine with some limitations, check out @ref BigEnumSet.

@see @ref enumCastUnderlyingType(), @ref enumSetDebugOutput()
*/
#ifdef DOXYGEN_GENERATING_OUTPUT
template<class T, typename std::underlying_type<T>::type fullValue = typename std::underlying_type<T>::type(~0)>
#else
template<class T, typename std::underlying_type<T>::type fullValue>
#endif
class EnumSet {
    static_assert(std::is_enum<T>::value, "EnumSet type must be a strongly typed enum");

    public:
        typedef T Type; /**< @brief Enum type */

        /** @brief Underlying type of the enum */
        typedef typename std::underlying_type<T>::type UnderlyingType;

        enum: UnderlyingType {
            FullValue = fullValue /**< All enum values together */
        };

        /** @brief Create an empty set */
        constexpr /*implicit*/ EnumSet() noexcept: _value{} {}

        /** @brief Create a set from one value */
        constexpr /*implicit*/ EnumSet(T value) noexcept:
            /* Interestingly enough, on GCC 4.8, using _value{} will spam with
                warning: parameter ‘value’ set but not used [-Wunused-but-set-parameter]
               even though everything works as intended. Using () instead. */
            _value(static_cast<UnderlyingType>(value)) {}

        /**
         * @brief Create an uninitialized set
         *
         * The contents are left in an undefined state.
         */
        explicit EnumSet(Corrade::NoInitT) {}

        /** @brief Equality comparison */
        constexpr bool operator==(EnumSet<T, fullValue> other) const {
            return _value == other._value;
        }

        /** @brief Non-equality comparison */
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
            return EnumSet<T, fullValue>(_value | other._value);
        }

        /** @brief Union two sets and assign */
        EnumSet<T, fullValue>& operator|=(EnumSet<T, fullValue> other) {
            _value |= other._value;
            return *this;
        }

        /** @brief Intersection of two sets */
        constexpr EnumSet<T, fullValue> operator&(EnumSet<T, fullValue> other) const {
            return EnumSet<T, fullValue>(_value & other._value);
        }

        /** @brief Intersect two sets and assign */
        EnumSet<T, fullValue>& operator&=(EnumSet<T, fullValue> other) {
            _value &= other._value;
            return *this;
        }

        /** @brief XOR of two sets */
        constexpr EnumSet<T, fullValue> operator^(EnumSet<T, fullValue> other) const {
            return EnumSet<T, fullValue>(_value ^ other._value);
        }

        /** @brief XOR two sets and assign */
        EnumSet<T, fullValue>& operator^=(EnumSet<T, fullValue> other) {
            _value ^= other._value;
            return *this;
        }

        /** @brief Set complement */
        constexpr EnumSet<T, fullValue> operator~() const {
            return EnumSet<T, fullValue>(fullValue & ~_value);
        }

        /**
         * @brief Boolean conversion
         *
         * Returns @cpp true @ce if at least one bit is set, @cpp false @ce
         * otherwise.
         */
        constexpr explicit operator bool() const {
            return _value != 0;
        }

        /** @brief Convert to the underlying enum type */
        constexpr explicit operator UnderlyingType() const {
            return _value;
        }

    private:
        constexpr explicit EnumSet(UnderlyingType type) noexcept: _value{type} {}

        UnderlyingType _value;
};

/** @relatesalso EnumSet
@brief Cast an enum to its underlying type
@m_since{2020,06}

Works only with @ref EnumSet, not with @ref BigEnumSet.
@see @ref std::underlying_type
*/
template<class T, class = typename std::enable_if<std::is_enum<T>::value>::type> constexpr typename std::underlying_type<T>::type enumCastUnderlyingType(T value) {
    return typename std::underlying_type<T>::type(value);
}

/** @relatesalso EnumSet
@brief Cast an enum set to its underlying type
@m_since{2020,06}

Works only with @ref EnumSet, not with @ref BigEnumSet.
@see @ref std::underlying_type
*/
template<class T, typename std::underlying_type<T>::type fullValue> constexpr typename std::underlying_type<T>::type enumCastUnderlyingType(EnumSet<T, fullValue> value) {
    return typename std::underlying_type<T>::type(value);
}

/** @hideinitializer
@brief Define out-of-class operators for given @ref Corrade::Containers::EnumSet "EnumSet" or @ref Corrade::Containers::BigEnumSet "BigEnumSet"

See the @ref EnumSet-out-of-class-operators "EnumSet class documentation" for
example usage.
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
@brief Define out-of-class operators for given @ref Corrade::Containers::EnumSet "EnumSet" or @ref Corrade::Containers::BigEnumSet "BigEnumSet" as friends of encapsulating class

See the @ref EnumSet-friend-operators "EnumSet class documentation" for example
usage.
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
