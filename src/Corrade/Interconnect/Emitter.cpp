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

#include "Emitter.h"

#include "Corrade/Interconnect/Receiver.h"
#include "Corrade/Utility/Assert.h"

namespace Corrade { namespace Interconnect {

namespace Implementation {

AbstractConnectionData::~AbstractConnectionData() = default;

}

Emitter::Emitter(): _lastHandledSignal{0}, _connectionsChanged{false} {}

Emitter::~Emitter() {
    for(auto connection: _connections) {
        const Implementation::AbstractConnectionData* data = connection.second;

        /* Remove connection from receiver, if this is member function connection */
        if(data->_type == Implementation::AbstractConnectionData::Type::Member) {
            auto& receiverConnections = static_cast<const Implementation::AbstractMemberConnectionData*>(data)->_receiver->_connections;
            for(auto end = receiverConnections.end(), rit = receiverConnections.begin(); rit != end; ++rit) {
                if(*rit != data) continue;

                receiverConnections.erase(rit);
                break;
            }
        }

        /* If there is connection object, remove reference to connection data
           from it and mark it as disconnected */
        if(data->_connection) {
            CORRADE_INTERNAL_ASSERT(data == data->_connection->_data);
            data->_connection->_data = nullptr;
            data->_connection->_connected = false;
        }

        /* Delete connection data (as they make no sense without emitter) */
        delete data;
    }
}

void Emitter::connectInternal(const Implementation::SignalData& signal, Implementation::AbstractConnectionData* data) {
    /* Add connection to emitter */
    data->_emitter->_connections.insert(std::make_pair(signal, data));
    data->_emitter->_connectionsChanged = true;

    /* Add connection to receiver, if this is member function connection */
    if(data->_type == Implementation::AbstractConnectionData::Type::Member)
        static_cast<Implementation::AbstractMemberConnectionData*>(data)->_receiver->_connections.push_back(data);

    /* If there is connection object, mark the connection as connected */
    if(data->_connection) data->_connection->_connected = true;
}

void Emitter::disconnectInternal(const Implementation::SignalData& signal, Implementation::AbstractConnectionData* data) {
    /* Find given connection, disconnect it and erase */
    auto range = data->_emitter->_connections.equal_range(signal);
    for(auto it = range.first; it != range.second; ++it) {
        if(it->second != data) continue;

        data->_emitter->disconnectInternal(it);
        data->_emitter->_connections.erase(it);
        data->_emitter->_connectionsChanged = true;
        return;
    }

    /* The connection must be found */
    CORRADE_ASSERT_UNREACHABLE(); /* LCOV_EXCL_LINE */
}

void Emitter::disconnectInternal(const Implementation::SignalData& signal) {
    auto range = _connections.equal_range(signal);
    for(auto it = range.first; it != range.second; ++it)
        disconnectInternal(it);

    _connections.erase(range.first, range.second);
    _connectionsChanged = true;
}

void Emitter::disconnectAllSignals() {
    for(auto it = _connections.begin(); it != _connections.end(); ++it)
        disconnectInternal(it);

    _connections.clear();
    _connectionsChanged = true;
}

void Emitter::disconnectInternal(std::unordered_multimap<Implementation::SignalData, Implementation::AbstractConnectionData*, Implementation::SignalDataHash>::const_iterator it) {
    Implementation::AbstractConnectionData* data = it->second;

    /* Remove connection from receiver, if this is member function connection */
    if(data->_type == Implementation::AbstractConnectionData::Type::Member) {
        auto& receiverConnections = static_cast<Implementation::AbstractMemberConnectionData*>(data)->_receiver->_connections;
        for(auto end = receiverConnections.end(), rit = receiverConnections.begin(); rit != end; ++rit) {
            if(*rit != data) continue;

            receiverConnections.erase(rit);
            break;
        }
    }

    /* If there is no connection object, destroy also connection data (as we
       are the last remaining owner) */
    if(!data->_connection) delete data;

    /* Else mark the connection as disconnected */
    else data->_connection->_connected = false;

    /* (erasing the iterator is up to the caller) */
}

}}
