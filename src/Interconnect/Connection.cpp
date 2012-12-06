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

#include "Connection.h"

#include "Utility/Assert.h"
#include "Emitter.h"
#include "Receiver.h"

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
    Emitter::connect(signal, data);
    return true;
}

void Connection::disconnect() {
    /* Already disconnected or the connection doesn't exist anymore */
    if(!connected || !data) return;

    Emitter::disconnect(signal, data);
}

Connection::Connection(Implementation::SignalData signal, Implementation::AbstractConnectionData* data): signal(signal), data(data), connected(true) {
    data->connection = this;
}

void Connection::destroy() {
    /* If disconnected, delete connection data (as we are the last remaining
       owner) */
    if(!connected) delete data;

    /* Else remove reference to itself from connection data */
    if(data) {
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
