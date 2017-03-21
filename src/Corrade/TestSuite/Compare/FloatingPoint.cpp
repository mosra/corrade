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

#include "FloatingPoint.h"

#include <cmath>

#include "Corrade/Utility/Debug.h"

namespace Corrade { namespace TestSuite { namespace Implementation {

/* Adapted from http://floating-point-gui.de/errors/comparison/ */
template<class T> bool FloatComparator<T>::operator()(T actual, T expected) {
    /* Shortcut for binary equality, infinites and NaN */
    if(actual == expected || (actual != actual && expected != expected)) return true;

    const T absA = std::abs(actual);
    const T absB = std::abs(expected);
    const T difference = std::abs(actual - expected);

    /* One of the numbers is zero or both are extremely close to it, relative
       error is meaningless */
    if((actual == T{} || expected == T{} || difference < FloatComparatorEpsilon<T>::epsilon())) {
        if(difference < FloatComparatorEpsilon<T>::epsilon()) return true;
    }

    /* Relative error */
    else if(difference/(absA + absB) < FloatComparatorEpsilon<T>::epsilon())
        return true;

    _actualValue = actual;
    _expectedValue = expected;
    return false;
}

template<class T> void FloatComparator<T>::printErrorMessage(Utility::Error& e, const std::string& actual, const std::string& expected) const {
    e << "Floating-point values" << actual << "and" << expected << "are not the same, actual" << _actualValue << "but" << _expectedValue << "expected (delta" << _actualValue - _expectedValue << Utility::Debug::nospace << ").";
}

template class FloatComparator<float>;
template class FloatComparator<double>;

}}}
