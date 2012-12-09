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

#include "Receiver.h"

#include "Utility/Assert.h"
#include "Emitter.h"

namespace Corrade { namespace Interconnect {

Receiver::~Receiver() {
    for(auto end = connections.end(), it = connections.begin(); it != end; ++it) {
        /* Remove connection from emitter */
        for(auto end = (*it)->emitter->connections.end(), eit = (*it)->emitter->connections.begin(); eit != end; ++eit) {
            if(eit->second != *it) continue;

            (*it)->emitter->connections.erase(eit);
            (*it)->emitter->connectionsChanged = true;
            break;
        }

        /* If there is connection object, remove reference to connection data
           from it and mark it as disconnected */
        if((*it)->connection) {
            CORRADE_INTERNAL_ASSERT(*it == (*it)->connection->data);
            (*it)->connection->data = nullptr;
            (*it)->connection->connected = false;
        }

        /* Delete connection data (as they make no sense without receiver) */
        delete *it;
    }
}

void Receiver::disconnect() {
    for(auto it = connections.begin(); it != connections.end(); ++it) {
        /* Remove connection from emitter */
        for(auto end = (*it)->emitter->connections.end(), eit = (*it)->emitter->connections.begin(); eit != end; ++eit) {
            if(eit->second != *it) continue;

            (*it)->emitter->connections.erase(eit);
            (*it)->emitter->connectionsChanged = true;
            break;
        }

        /* If there is no connection object, destroy also connection data (as we
           are the last remaining owner) */
        if(!(*it)->connection) delete *it;

        /* Else mark the connection as disconnected */
        else (*it)->connection->connected = false;
    }

    connections.clear();
}

}}
