/*
    Corrade's optimized <cmath>, without the heavy C++17 additions

    https://doc.magnum.graphics/corrade/StlMath_8h.html

    This is a single-header library generated from the Corrade project. With
    the goal being easy integration, it's deliberately free of all comments
    to keep the file size small. More info, detailed changelogs and docs here:

    -   Project homepage — https://magnum.graphics/corrade/
    -   Documentation — https://doc.magnum.graphics/corrade/
    -   GitHub project page — https://github.com/mosra/corrade
    -   GitHub Singles repository — https://github.com/mosra/magnum-singles

    v2020.06-1454-gfc3b7 (2023-08-27)
    -   Compatibility with C++20 which removes the <ciso646> header
    v2019.01-186-gdd93f1f1 (2019-06-06)
    -   Initial release

    Generated from Corrade {{revision}}, {{stats:loc}} / {{stats:preprocessed-cpp17}} LoC
*/

#include "base.h"
/* No external includes, so avoid extraneous newline here */
/* Just make sure bits/c++config.h with _GLIBCXX_USE_STD_SPEC_FUNCS is brought
   in by <ciso646> or <version>. */
#ifdef _MSC_VER
#ifdef _MSVC_LANG
#define CORRADE_CXX_STANDARD _MSVC_LANG
#else
#define CORRADE_CXX_STANDARD 201103L
#endif
#else
#define CORRADE_CXX_STANDARD __cplusplus
#endif
#if CORRADE_CXX_STANDARD >= 202002
#include <version>
#else
#include <ciso646>
#endif

#pragma ACME stats preprocessed-cpp17 g++ -std=c++17 -P -E -x c++ - | wc -l

/* A semi-verbatim copy of Utility/StlMath.h because otherwise the includes
   don't stay in the correct place. */
#ifndef Corrade_Utility_StlMath_h
#define Corrade_Utility_StlMath_h
/*
    Copyright © 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016,
                2017, 2018, 2019, 2020, 2021, 2022, 2023, 2024, 2025
              Vladimír Vondruš <mosra@centrum.cz>
*/

#ifdef _GLIBCXX_USE_STD_SPEC_FUNCS
#undef _GLIBCXX_USE_STD_SPEC_FUNCS
#define _GLIBCXX_USE_STD_SPEC_FUNCS 0
#endif
#include <cmath>

#endif
