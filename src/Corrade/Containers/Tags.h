#ifndef Corrade_Containers_Tags_h
#define Corrade_Containers_Tags_h
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
 * @brief Tag type @ref Corrade::Containers::ValueInitT, @ref Corrade::Containers::DefaultInitT, @ref Corrade::Containers::NoInitT, @ref Corrade::Containers::DirectInitT, tag @ref Corrade::Containers::ValueInit, @ref Corrade::Containers::DefaultInit, @ref Corrade::Containers::NoInit, @ref Corrade::Containers::DirectInit
 */

#include "Corrade/configure.h"

namespace Corrade { namespace Containers {

/**
@brief Default initialization tag type

Used to distinguish construction using default initialization (builtin types
are not initialized, others are default-constructed).
@see @ref DefaultInit
*/
/* Explicit constructor to avoid ambiguous calls when using {} */
struct DefaultInitT {
    #ifndef DOXYGEN_GENERATING_OUTPUT
    struct Init{};
    constexpr explicit DefaultInitT(Init) {}
    #endif
};

/**
@brief Value initialization tag type

Used to distinguish construction using value initialization (builtin types are
zeroed out, others are default-constructed).
@see @ref ValueInit
*/
/* Explicit constructor to avoid ambiguous calls when using {} */
struct ValueInitT {
    #ifndef DOXYGEN_GENERATING_OUTPUT
    struct Init{};
    constexpr explicit ValueInitT(Init) {}
    #endif
};

/**
@brief No initialization tag type

Used to distinguish construction with no initialization at all, which leaves
the data with whatever random values the memory had before.
@see @ref NoInit, @ref NoCreateT
*/
/* Explicit constructor to avoid ambiguous calls when using {} */
struct NoInitT {
    #ifndef DOXYGEN_GENERATING_OUTPUT
    struct Init{};
    constexpr explicit NoInitT(Init) {}
    #endif
};

/**
@brief No creation tag type

Used to distinguish construction with initialization but not creation. Contrary
to @ref NoInitT this doesn't keep random values, but makes the instance empty
(usually equivalent to a moved-out state).
@see @ref NoCreate
*/
/* Explicit constructor to avoid ambiguous calls when using {} */
struct NoCreateT {
    #ifndef DOXYGEN_GENERATING_OUTPUT
    struct Init{};
    constexpr explicit NoCreateT(Init) {}
    #endif
};

/**
@brief Direct initialization tag type

Used to distinguish construction with direct initialization.
@see @ref DirectInit
*/
/* Explicit constructor to avoid ambiguous calls when using {} */
struct DirectInitT {
    #ifndef DOXYGEN_GENERATING_OUTPUT
    struct Init{};
    constexpr explicit DirectInitT(Init) {}
    #endif
};

/**
@brief In-place initialization tag type

Used to distinguish construction with in-place initialization.
@see @ref DirectInit
*/
/* Explicit constructor to avoid ambiguous calls when using {} */
struct InPlaceInitT {
    #ifndef DOXYGEN_GENERATING_OUTPUT
    struct Init{};
    constexpr explicit InPlaceInitT(Init) {}
    #endif
};

/**
@brief Default initialization tag

Use for construction using default initialization (builtin types are not
initialized, others are default-constructed).
*/
constexpr DefaultInitT DefaultInit{DefaultInitT::Init{}};

/**
@brief Value initialization tag

Use for construction using value initialization (builtin types are zeroed out,
others are default-constructed).
*/
constexpr ValueInitT ValueInit{ValueInitT::Init{}};

/**
@brief No initialization tag

Use for construction with no initialization at all.
*/
constexpr NoInitT NoInit{NoInitT::Init{}};

/**
@brief No creation tag

Use for construction with initialization, but keeping the instance empty
(usually equivalent to a moved-out state).
*/
constexpr NoCreateT NoCreate{NoCreateT::Init{}};

/**
@brief Direct initialization tag

Use for construction with direct initialization.
*/
constexpr DirectInitT DirectInit{DirectInitT::Init{}};

/**
@brief In-place initialization tag

Use for construction in-place.
*/
constexpr InPlaceInitT InPlaceInit{InPlaceInitT::Init{}};

}}

#endif
