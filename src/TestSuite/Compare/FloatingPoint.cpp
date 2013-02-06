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

#include "FloatingPoint.h"

#include <cmath>

#ifndef DOXYGEN_GENERATING_OUTPUT
namespace Corrade { namespace TestSuite { namespace Implementation {

template<class T> bool FloatComparator<T>::operator()(T actual, T expected) {
    if(actual == expected || (actual != actual && expected != expected) ||
        std::abs(actual - expected) < FloatComparatorEpsilon<T>::epsilon()) return true;

    actualValue = actual;
    expectedValue = expected;
    return false;
}

template<class T> void FloatComparator<T>::printErrorMessage(Utility::Error& e, const std::string& actual, const std::string& expected) const {
    e << "Floating-point values" << actual << "and" << expected << "are not the same, actual" << actualValue << "but" << expectedValue << "expected";
    e.setFlag(Utility::Debug::SpaceAfterEachValue, false);
    e << " (delta " << actualValue-expectedValue << ").";
    e.setFlag(Utility::Debug::SpaceAfterEachValue, true);
}

template class FloatComparator<float>;
template class FloatComparator<double>;

}}}
#endif
