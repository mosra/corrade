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

#include "Receiver.h"

#include "Corrade/Interconnect/Connection.h"
#include "Corrade/Interconnect/Emitter.h"
#include "Corrade/Interconnect/Implementation/ReceiverConnection.h"
#include "Corrade/Utility/Assert.h"

namespace Corrade { namespace Interconnect {

Receiver::Receiver() = default;

Receiver::~Receiver() { disconnectAllSlots(); }

bool Receiver::hasSlotConnections() const { return !_connections.empty(); }

std::size_t Receiver::slotConnectionCount() const { return _connections.size(); }

void Receiver::disconnectAllSlots() {
    for(Implementation::ReceiverConnection& connection: _connections) {
        auto range = connection.emitter->_connections.equal_range(connection.signal);
        for(auto it = range.first; it != range.second; ++it) {
            if(&it->second != &*connection.data) continue;

            connection.emitter->_connections.erase(it);
            connection.emitter->_connectionsChanged = true;
            break;
        }
    }

    _connections.clear();
}

}}
