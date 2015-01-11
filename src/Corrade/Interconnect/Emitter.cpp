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

#include "Emitter.h"

#include "Corrade/Interconnect/Receiver.h"
#include "Corrade/Utility/Assert.h"

namespace Corrade { namespace Interconnect {

namespace Implementation {

AbstractConnectionData::~AbstractConnectionData() {}

}

Emitter::Emitter(): lastHandledSignal(0), connectionsChanged(false) {}

Emitter::~Emitter() {
    for(auto connection: connections) {
        const Implementation::AbstractConnectionData* data = connection.second;

        /* Remove connection from receiver, if this is member function connection */
        if(data->type == Implementation::AbstractConnectionData::Type::Member) {
            auto& receiverConnections = static_cast<const Implementation::AbstractMemberConnectionData*>(data)->receiver->connections;
            for(auto end = receiverConnections.end(), rit = receiverConnections.begin(); rit != end; ++rit) {
                if(*rit != data) continue;

                receiverConnections.erase(rit);
                break;
            }
        }

        /* If there is connection object, remove reference to connection data
           from it and mark it as disconnected */
        if(data->connection) {
            CORRADE_INTERNAL_ASSERT(data == data->connection->data);
            data->connection->data = nullptr;
            data->connection->connected = false;
        }

        /* Delete connection data (as they make no sense without emitter) */
        delete data;
    }
}

void Emitter::connectInternal(const Implementation::SignalData& signal, Implementation::AbstractConnectionData* data) {
    /* Add connection to emitter */
    data->emitter->connections.insert(std::make_pair(signal, data));
    data->emitter->connectionsChanged = true;

    /* Add connection to receiver, if this is member function connection */
    if(data->type == Implementation::AbstractConnectionData::Type::Member)
        static_cast<Implementation::AbstractMemberConnectionData*>(data)->receiver->connections.push_back(data);

    /* If there is connection object, mark the connection as connected */
    if(data->connection) data->connection->connected = true;
}

void Emitter::disconnectInternal(const Implementation::SignalData& signal, Implementation::AbstractConnectionData* data) {
    /* Find given connection, disconnect it and erase */
    auto range = data->emitter->connections.equal_range(signal);
    for(auto it = range.first; it != range.second; ++it) {
        if(it->second != data) continue;

        data->emitter->disconnectInternal(it);
        data->emitter->connections.erase(it);
        data->emitter->connectionsChanged = true;
        return;
    }

    /* The connection must be found */
    CORRADE_ASSERT_UNREACHABLE();
}

void Emitter::disconnectInternal(const Implementation::SignalData& signal) {
    auto range = connections.equal_range(signal);
    for(auto it = range.first; it != range.second; ++it)
        disconnectInternal(it);

    connections.erase(range.first, range.second);
    connectionsChanged = true;
}

void Emitter::disconnectAllSignals() {
    for(auto it = connections.begin(); it != connections.end(); ++it)
        disconnectInternal(it);

    connections.clear();
    connectionsChanged = true;
}

void Emitter::disconnectInternal(std::unordered_multimap<Implementation::SignalData, Implementation::AbstractConnectionData*, Implementation::SignalDataHash>::const_iterator it) {
    Implementation::AbstractConnectionData* data = it->second;

    /* Remove connection from receiver, if this is member function connection */
    if(data->type == Implementation::AbstractConnectionData::Type::Member) {
        auto& receiverConnections = static_cast<Implementation::AbstractMemberConnectionData*>(data)->receiver->connections;
        for(auto end = receiverConnections.end(), rit = receiverConnections.begin(); rit != end; ++rit) {
            if(*rit != data) continue;

            receiverConnections.erase(rit);
            break;
        }
    }

    /* If there is no connection object, destroy also connection data (as we
       are the last remaining owner) */
    if(!data->connection) delete data;

    /* Else mark the connection as disconnected */
    else data->connection->connected = false;

    /* (erasing the iterator is up to the caller) */
}

}}
