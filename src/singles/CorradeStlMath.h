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

    v2019.01-186-gdd93f1f1 (2019-06-06)
    -   Initial release

    Generated from Corrade {{revision}}, {{stats:loc}} / {{stats:preprocessed-cpp17}} LoC
*/

#include "base.h"

#pragma ACME stats preprocessed-cpp17 g++ -std=c++17 -P -E -x c++ - | wc -l

#include <ciso646>

/* A semi-verbatim copy of Utility/StlMath.h because otherwise the includes
   don't stay in the correct place. */
#ifndef Corrade_Utility_StlMath_h
#define Corrade_Utility_StlMath_h
/*
    Copyright © 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016,
                2017, 2018, 2019, 2020 Vladimír Vondruš <mosra@centrum.cz>
*/

#ifdef _GLIBCXX_USE_STD_SPEC_FUNCS
#undef _GLIBCXX_USE_STD_SPEC_FUNCS
#define _GLIBCXX_USE_STD_SPEC_FUNCS 0
#endif
#include <cmath>

#endif
