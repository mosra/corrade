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

#include "Receiver.h"

#include <utility>

#include "Corrade/Interconnect/Connection.h"
#include "Corrade/Interconnect/Emitter.h"
#include "Corrade/Utility/Assert.h"

namespace Corrade { namespace Interconnect {

Receiver::Receiver() = default;

Receiver::~Receiver() {
    for(auto end = _connections.end(), it = _connections.begin(); it != end; ++it) {
        /* Remove connection from emitter */
        for(auto eend = (*it)->_emitter->_connections.end(), eit = (*it)->_emitter->_connections.begin(); eit != eend; ++eit) {
            if(eit->second != *it) continue;

            (*it)->_emitter->_connections.erase(eit);
            (*it)->_emitter->_connectionsChanged = true;
            break;
        }

        /* If there is connection object, remove reference to connection data
           from it and mark it as disconnected */
        if((*it)->_connection) {
            CORRADE_INTERNAL_ASSERT(*it == (*it)->_connection->_data);
            (*it)->_connection->_data = nullptr;
            (*it)->_connection->_connected = false;
        }

        /* Delete connection data (as they make no sense without receiver) */
        delete *it;
    }
}

void Receiver::disconnectAllSlots() {
    for(Implementation::AbstractConnectionData* const connection: _connections) {
        /* Remove connection from emitter */
        for(auto end = connection->_emitter->_connections.end(), eit = connection->_emitter->_connections.begin(); eit != end; ++eit) {
            if(eit->second != connection) continue;

            connection->_emitter->_connections.erase(eit);
            connection->_emitter->_connectionsChanged = true;
            break;
        }

        /* If there is no connection object, destroy also connection data (as we
           are the last remaining owner) */
        if(!connection->_connection) delete connection;

        /* Else mark the connection as disconnected */
        else connection->_connection->_connected = false;
    }

    _connections.clear();
}

}}
