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

#include "TypeTraits.h"

#include <vector>
#include <string>

using namespace std;

namespace Corrade { namespace Utility {

#ifndef DOXYGEN_GENERATING_OUTPUT
static_assert(IsIterable<vector<int>>::Value, "std::vector should be iterable");
static_assert(IsIterable<string>::Value, "std::string should be iterable");

struct AnyType {};
static_assert(!HasInsertionOperator<AnyType>::Value, "Generic type shouldn't be outputable to std::ostream");
static_assert(HasInsertionOperator<string>::Value, "std::string should be outputtable to std::ostream");
#endif

}}
