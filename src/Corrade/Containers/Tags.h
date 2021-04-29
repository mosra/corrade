#ifndef Corrade_Containers_Tags_h
#define Corrade_Containers_Tags_h
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

#ifdef CORRADE_BUILD_DEPRECATED
/** @file
 * @deprecated Tag types and tags
 * @m_deprecated_since_latest Use @ref Corrade/Tags.h instead.
 */
#endif

#include "Corrade/configure.h"

#ifdef CORRADE_BUILD_DEPRECATED
#include "Corrade/Tags.h"

CORRADE_DEPRECATED_FILE("use Corrade/Tags.h and tags in the Corrade namespace instead")

/* Deprecated aliases defined in Corrade/Tags.h and not here since most code
   relies on the header being included transitively and so the aliases wouldn't
   be found if we'd switch to the new header everywhere */
#else
#error use Corrade/Tags.h and tags in the Corrade namespace instead
#endif

#endif
