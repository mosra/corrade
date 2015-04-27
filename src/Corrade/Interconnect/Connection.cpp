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

#include "Connection.h"

#include <utility>

#include "Corrade/Interconnect/Emitter.h"
#include "Corrade/Utility/Assert.h"

namespace Corrade { namespace Interconnect {

Connection::Connection(Connection&& other): signal(other.signal), data(other.data), connected(other.connected) {
    move(std::move(other));
}

Connection::~Connection() {
    destroy();
}

Connection& Connection::operator=(Connection&& other) {
    destroy();

    signal = other.signal;
    data = other.data;
    connected = other.connected;

    move(std::move(other));
    return *this;
}

bool Connection::connect() {
    /* The connection is not possible anymore */
    if(!data) return false;

    /* Already connected */
    if(connected) return true;

    /* Create the connection */
    Emitter::connectInternal(signal, data);
    return true;
}

void Connection::disconnect() {
    /* Already disconnected or the connection doesn't exist anymore */
    if(!connected || !data) return;

    Emitter::disconnectInternal(signal, data);
}

Connection::Connection(Implementation::SignalData signal, Implementation::AbstractConnectionData* data): signal(signal), data(data), connected(true) {
    data->connection = this;
}

void Connection::destroy() {
    /* If disconnected, delete connection data (as we are the last remaining
       owner) */
    if(!connected) delete data;

    /* Else remove reference to itself from connection data */
    else if(data) {
        CORRADE_INTERNAL_ASSERT(data->connection == this);
        data->connection = nullptr;
    }
}

void Connection::move(Connection&& other) {
    if(data) data->connection = this;
    other.data = nullptr;
    other.connected = false;
}

}}
