#ifndef Corrade_Tags_h
#define Corrade_Tags_h
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
 * @brief Tag type @ref Corrade::ValueInitT, @ref Corrade::DefaultInitT, @ref Corrade::NoInitT, @ref Corrade::DirectInitT, tag @ref Corrade::ValueInit, @ref Corrade::DefaultInit, @ref Corrade::NoInit, @ref Corrade::DirectInit
 * @m_since_latest
 */

#include "Corrade/configure.h"

#ifdef CORRADE_BUILD_DEPRECATED
#include "Corrade/Utility/Macros.h"
#endif

namespace Corrade {

/**
@brief Default initialization tag type
@m_since_latest

Used to distinguish construction with default initialization. The actual
meaning of "default" may vary, see documentation of a particular API using this
tag for a detailed behavior description.
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
@m_since_latest

Used to distinguish construction with value initialization (builtin types are
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
@m_since_latest

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
@m_since_latest

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
@m_since_latest

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
@m_since_latest

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
@m_since_latest

Use for construction with default initialization. The actual meaning of
"default" may vary, see documentation of a particular API using this tag for
a detailed behavior description.
*/
constexpr DefaultInitT DefaultInit{DefaultInitT::Init{}};

/**
@brief Value initialization tag
@m_since_latest

Use for construction using value initialization (builtin types are zeroed out,
others are default-constructed).
*/
constexpr ValueInitT ValueInit{ValueInitT::Init{}};

/**
@brief No initialization tag
@m_since_latest

Use for construction with no initialization at all.
*/
constexpr NoInitT NoInit{NoInitT::Init{}};

/**
@brief No creation tag
@m_since_latest

Use for construction with initialization, but keeping the instance empty
(usually equivalent to a moved-out state).
*/
constexpr NoCreateT NoCreate{NoCreateT::Init{}};

/**
@brief Direct initialization tag
@m_since_latest

Use for construction with direct initialization.
*/
constexpr DirectInitT DirectInit{DirectInitT::Init{}};

/**
@brief In-place initialization tag
@m_since_latest

Use for construction in-place.
*/
constexpr InPlaceInitT InPlaceInit{InPlaceInitT::Init{}};

#ifdef CORRADE_BUILD_DEPRECATED
/* Deprecated aliases defined here and not in in Corrade/Containers/Tags.h
   since most code relies on the header being included transitively and so the
   aliases wouldn't be found if we'd switch to the new header everywhere */

namespace Containers {

/** @brief @copybrief Corrade::DefaultInitT
 * @m_deprecated_since_latest Use @ref Corrade::DefaultInitT instead.
 */
typedef CORRADE_DEPRECATED("use Corrade::DefaultInitT instead") Corrade::DefaultInitT DefaultInitT;

/** @brief @copybrief Corrade::ValueInitT
 * @m_deprecated_since_latest Use @ref Corrade::ValueInitT instead.
 */
typedef CORRADE_DEPRECATED("use Corrade::ValueInitT instead") Corrade::ValueInitT ValueInitT;

/** @brief @copybrief Corrade::NoInitT
 * @m_deprecated_since_latest Use @ref Corrade::NoInitT instead.
 */
typedef CORRADE_DEPRECATED("use Corrade::NoInitT instead") Corrade::NoInitT NoInitT;

/** @brief @copybrief Corrade::NoCreateT
 * @m_deprecated_since_latest Use @ref Corrade::NoCreateT instead.
 */
typedef CORRADE_DEPRECATED("use Corrade::NoCreateT instead") Corrade::NoCreateT NoCreateT;

/** @brief @copybrief Corrade::DirectInitT
 * @m_deprecated_since_latest Use @ref Corrade::DirectInitT instead.
 */
typedef CORRADE_DEPRECATED("use Corrade::DirectInitT instead") Corrade::DirectInitT DirectInitT;

/** @brief @copybrief Corrade::InPlaceInitT
 * @m_deprecated_since_latest Use @ref Corrade::InPlaceInitT instead.
 */
typedef CORRADE_DEPRECATED("use Corrade::InPlaceInitT instead") Corrade::InPlaceInitT InPlaceInitT;

/** @brief @copybrief Corrade::DefaultInit
 * @m_deprecated_since_latest Use @ref Corrade::DefaultInit instead.
 * @todo when removing, clean up all Corrade::DefaultInit in Containers to be
 *      DefaultInit again
 */
constexpr CORRADE_DEPRECATED("use Corrade::DefaultInitT instead") Corrade::DefaultInitT DefaultInit{Corrade::DefaultInitT::Init{}};

/** @brief @copybrief Corrade::ValueInit
 * @m_deprecated_since_latest Use @ref Corrade::ValueInit instead.
 * @todo when removing, clean up all Corrade::ValueInit in Containers to be
 *      ValueInit again
 */
constexpr CORRADE_DEPRECATED("use Corrade::ValueInit instead") Corrade::ValueInitT ValueInit{Corrade::ValueInitT::Init{}};

/** @brief @copybrief Corrade::NoInit
 * @m_deprecated_since_latest Use @ref Corrade::NoInit instead.
 * @todo when removing, clean up all Corrade::NoInit in Containers to be
 *      NoInit again
 */
constexpr CORRADE_DEPRECATED("use Corrade::NoInit instead") Corrade::NoInitT NoInit{Corrade::NoInitT::Init{}};

/** @brief @copybrief Corrade::NoCreate
 * @m_deprecated_since_latest Use @ref Corrade::NoCreate instead.
 * @todo when removing, clean up all Corrade::NoCreate in Containers to be
 *      NoCreate again
 */
constexpr CORRADE_DEPRECATED("use Corrade::NoCreateT instead") Corrade::NoCreateT NoCreate{Corrade::NoCreateT::Init{}};

/** @brief @copybrief Corrade::DirectInit
 * @m_deprecated_since_latest Use @ref Corrade::DirectInit instead.
 * @todo when removing, clean up all Corrade::DirectInit in Containers to be
 *      DirectInit again
 */
constexpr CORRADE_DEPRECATED("use Corrade::DirectInitT instead") Corrade::DirectInitT DirectInit{Corrade::DirectInitT::Init{}};

/** @brief @copybrief Corrade::InPlaceInit
 * @m_deprecated_since_latest Use @ref Corrade::InPlaceInit instead.
 * @todo when removing, clean up all Corrade::InPlaceInit in Containers to be
 *      InPlaceInit again
 */
constexpr CORRADE_DEPRECATED("use Corrade::InPlaceInit instead") Corrade::InPlaceInitT InPlaceInit{Corrade::InPlaceInitT::Init{}};

}
#endif

}

#endif
