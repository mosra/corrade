#ifndef Corrade_Containers_Tags_h
#define Corrade_Containers_Tags_h
/*
    This file is part of Corrade.

    Copyright © 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015
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
 * @brief Tag type @ref Corrade::Containers::ValueInitT, @ref Corrade::Containers::DefaultInitT, @ref Corrade::Containers::NoInitT, @ref Corrade::Containers::DirectInitT, tag @ref Corrade::Containers::ValueInit, @ref Corrade::Containers::DefaultInit, @ref Corrade::Containers::NoInit, @ref Corrade::Containers::DirectInit
 */

namespace Corrade { namespace Containers {

/**
@brief Default initialization tag type

Used to distinguish construction using default initialization (builtin types
are not initialized, others are default-constructed).
@see @ref DefaultInit
*/
struct DefaultInitT {};

/**
@brief Value initialization tag type

Used to distinguish construction using value initialization (builtin types are
zeroed out, others are default-constructed).
@see @ref ValueInit
*/
struct ValueInitT {};

/**
@brief No initialization tag type

Used to distinguish construction with no initialization at all.
@see @ref NoInit
*/
struct NoInitT {};

/**
@brief Direct initialization tag type

Used to distinguish construction with direct initialization.
@see @ref DirectInit
*/
struct DirectInitT {};

/**
@brief Default initialization tag

Use for construction using default initialization (builtin types are not
initialized, others are default-constructed).
*/
constexpr DefaultInitT DefaultInit{};

/**
@brief Value initialization tag

Use for construction using value initialization (builtin types are zeroed out,
others are default-constructed).
*/
constexpr ValueInitT ValueInit{};

/**
@brief No initialization tag

Use for construction with no initialization at all.
*/
constexpr NoInitT NoInit{};

/**
@brief Direct initialization tag

Use for construction with direct initialization.
*/
constexpr DirectInitT DirectInit{};

}}

#endif
