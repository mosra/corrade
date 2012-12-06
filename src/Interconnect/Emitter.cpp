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

#include "Emitter.h"

#include "Utility/Debug.h"
#include "Receiver.h"

namespace Corrade { namespace Interconnect {

Emitter::~Emitter() {
    for(auto it = connections.begin(); it != connections.end(); ++it) {
        /* Remove connection from receiver */
        for(auto rit = it->second->receiver->connections.begin(); rit != it->second->receiver->connections.end(); ++rit) {
            if(*rit != it->second) continue;

            it->second->receiver->connections.erase(rit);
            break;
        }

        /* If there is connection object, remove reference to connection data
           from it and mark it as disconnected */
        if(it->second->connection) {
            CORRADE_INTERNAL_ASSERT(it->second == it->second->connection->data);
            it->second->connection->data = nullptr;
            it->second->connection->connected = false;
        }

        /* Delete connection data (as they make no sense without emitter) */
        delete it->second;
    }
}

void Emitter::connect(const Implementation::SignalData& signal, Implementation::AbstractConnectionData* data) {
    /* Add connection to emitter */
    data->emitter->connections.insert(std::make_pair(signal, data));

    /* Add connection to receiver */
    data->receiver->connections.push_back(data);

    /* If there is connection object, mark the connection as connected */
    if(data->connection) data->connection->connected = true;
}

void Emitter::disconnect(const Implementation::SignalData& signal, Implementation::AbstractConnectionData* data) {
    /* Find given connection, disconnect it and erase */
    auto range = data->emitter->connections.equal_range(signal);
    for(auto it = range.first; it != range.second; ++it) {
        if(it->second != data) continue;

        data->emitter->disconnect(it);
        data->emitter->connections.erase(it);
        return;
    }

    /* The connection must be found */
    CORRADE_INTERNAL_ASSERT(false);
}

void Emitter::disconnect(const Implementation::SignalData& signal) {
    auto range = connections.equal_range(signal);
    for(auto it = range.first; it != range.second; ++it)
        disconnect(it);

    connections.erase(range.first, range.second);
}

void Emitter::disconnect() {
    for(auto it = connections.begin(); it != connections.end(); ++it)
        disconnect(it);

    connections.clear();
}

void Emitter::disconnect(std::unordered_multimap<Implementation::SignalData, Implementation::AbstractConnectionData*, Implementation::SignalDataHash>::const_iterator it) {
    Implementation::AbstractConnectionData* data = it->second;

    /* Remove connection from receiver */
    for(auto end = data->receiver->connections.end(), rit = data->receiver->connections.begin(); rit != end; ++rit) {
        if(*rit != data) continue;

        data->receiver->connections.erase(rit);
        break;
    }

    /* If there is no connection object, destroy also connection data (as we
       are the last remaining owner) */
    if(!data->connection) delete data;

    /* Else mark the connection as disconnected */
    else data->connection->connected = false;

    /* (erasing the iterator is up to the caller) */
}

}}
