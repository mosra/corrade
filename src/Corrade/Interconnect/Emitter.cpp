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

#include "Emitter.h"

#include "Corrade/Interconnect/Receiver.h"
#include "Corrade/Interconnect/Implementation/ReceiverConnection.h"
#include "Corrade/Utility/Assert.h"

namespace Corrade { namespace Interconnect {

namespace Implementation {

ConnectionData::ConnectionData(ConnectionData&& other) noexcept:
    storage(other.storage), /* GCC 4.8 needs () */
    call{other.call},
    lastHandledSignal{other.lastHandledSignal},
    type{other.type}
{
    if(type == ConnectionType::FunctorWithDestructor)
        other.type = ConnectionType::Functor;
}

ConnectionData& ConnectionData::operator=(ConnectionData&& other) noexcept {
    using std::swap;
    swap(storage, other.storage);
    swap(call, other.call);
    swap(lastHandledSignal, other.lastHandledSignal);
    swap(type, other.type);
    return *this;
}

ConnectionData::~ConnectionData() {
    if(type == ConnectionType::FunctorWithDestructor)
        storage.functor.destruct(storage);
}

}

Emitter::Emitter(): _lastHandledSignal{0}, _connectionsChanged{false} {}

Emitter::~Emitter() {
    for(auto& connection: _connections) disconnectFromReceiver(connection.second);
}

bool Emitter::isConnected(const Connection& connection) const {
    auto range = _connections.equal_range(connection._signal);
    for(auto it = range.first; it != range.second; ++it)
        if(&it->second == &*connection._data) return true;

    return false;
}

Implementation::ConnectionData& Emitter::connectInternal(const Implementation::SignalData& signal, Implementation::ConnectionData&& data) {
    /* Add connection to emitter */
    Implementation::ConnectionData& out =
    _connections.emplace(signal, std::move(data))->second;
    _connectionsChanged = true;

    /* Add connection to receiver, if this is member function connection */
    if(data.type == Implementation::ConnectionType::Member)
        out.storage.member.receiver->_connections.emplace_back(*this, signal, out);

    /* Return reference to the final position */
    return out;
}

void Emitter::disconnectInternal(const Implementation::SignalData& signal) {
    auto range = _connections.equal_range(signal);
    for(auto it = range.first; it != range.second; ++it)
        disconnectFromReceiver(it->second);

    _connections.erase(range.first, range.second);
    _connectionsChanged = true;
}

void Emitter::disconnectAllSignals() {
    for(auto it = _connections.begin(); it != _connections.end(); ++it)
        disconnectFromReceiver(it->second);

    _connections.clear();
    _connectionsChanged = true;
}

void Emitter::disconnectFromReceiver(const Implementation::ConnectionData& data) {
    if(data.type != Implementation::ConnectionType::Member) return;

    auto& receiverConnections = data.storage.member.receiver->_connections;
    for(auto end = receiverConnections.end(), rit = receiverConnections.begin(); rit != end; ++rit) {
        if(&*rit->data != &data) continue;

        receiverConnections.erase(rit);
        return;
    }

    /* The connection must be found */
    CORRADE_INTERNAL_ASSERT_UNREACHABLE(); /* LCOV_EXCL_LINE */
}

bool disconnect(Emitter& emitter, const Connection& connection) {
    auto range = emitter._connections.equal_range(connection._signal);
    for(auto it = range.first; it != range.second; ++it) {
        if(&it->second != &*connection._data) continue;

        emitter.disconnectFromReceiver(it->second);
        emitter._connections.erase(it);
        emitter._connectionsChanged = true;
        return true;
    }

    return false;
}

}}
