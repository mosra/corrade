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

#include "Connection.h"

#include <utility>

#include "Corrade/Interconnect/Emitter.h"
#include "Corrade/Utility/Assert.h"

namespace Corrade { namespace Interconnect {

Connection::Connection(
    #ifdef CORRADE_BUILD_DEPRECATED
    Emitter& emitter,
    #endif
    Implementation::SignalData signal, Implementation::ConnectionData& data):
    #ifdef CORRADE_BUILD_DEPRECATED
    _emitter{emitter},
    #endif
    _signal{signal}, _data{&data} {}

#ifdef CORRADE_BUILD_DEPRECATED
/* LCOV_EXCL_START */
bool Connection::isConnected() const {
    Utility::Warning{} << "Interconnect::Emitter::isConnected(): this function is dangerous, use Emitter::isConnected() instead";
    return _emitter->isConnected(*this);
}

void Connection::disconnect() {
    Utility::Warning{} << "Interconnect::Connection::disconnect(): this function is dangerous, use Interconnect::disconnect() instead";
    Interconnect::disconnect(_emitter, *this);
}
/* LCOV_EXCL_STOP */
#endif

}}
