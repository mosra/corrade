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

#include "Connection.h"

#include <utility>

#include "Corrade/Interconnect/Emitter.h"
#include "Corrade/Utility/Assert.h"

namespace Corrade { namespace Interconnect {

Connection::Connection(Connection&& other) noexcept: _signal{other._signal}, _data{other._data}, _connected{other._connected} {
    if(_data) _data->_connection = this;
    other._data = nullptr;
    other._connected = false;
}

Connection::~Connection() {
    /* If disconnected, delete connection data (as we are the last remaining
       owner) */
    if(!_connected) delete _data;

    /* Else remove reference to itself from connection data */
    else if(_data) {
        CORRADE_INTERNAL_ASSERT(_data->_connection == this);
        _data->_connection = nullptr;
    }
}

Connection& Connection::operator=(Connection&& other) noexcept {
    using std::swap;
    swap(other._signal, _signal);
    swap(other._data, _data);
    swap(other._connected, _connected);
    if(_data) _data->_connection = this;
    if(other._data) other._data->_connection = &other;
    return *this;
}

bool Connection::connect() {
    /* The connection is not possible anymore */
    if(!_data) return false;

    /* Already connected */
    if(_connected) return true;

    /* Create the connection */
    Emitter::connectInternal(_signal, _data);
    return true;
}

void Connection::disconnect() {
    /* Already disconnected or the connection doesn't exist anymore */
    if(!_connected || !_data) return;

    Emitter::disconnectInternal(_signal, _data);
}

Connection::Connection(Implementation::SignalData signal, Implementation::AbstractConnectionData* data): _signal{signal}, _data{data}, _connected{true} {
    _data->_connection = this;
}

}}
