#ifndef Corrade_Interconnect_Receiver_h
#define Corrade_Interconnect_Receiver_h
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

/** @file
 * @brief Class Corrade::Interconnect::Receiver
 */

#include <vector>

#include "corradeInterconnectVisibility.h"

namespace Corrade { namespace Interconnect {

#ifndef DOXYGEN_GENERATING_OUTPUT
namespace Implementation {
    class AbstractConnectionData;
}
#endif

/**
@brief %Receiver object

Contains member function slots. See @ref interconnect for introduction.
@see Emitter, Connection
@todo Allow move
*/
class CORRADE_INTERCONNECT_EXPORT Receiver {
    friend class Implementation::AbstractConnectionData;
    friend class Emitter;

    Receiver(const Receiver&) = delete;
    Receiver(Receiver&&) = delete;
    Receiver& operator=(const Receiver&) = delete;
    Receiver& operator=(Receiver&&) = delete;

    public:
        explicit Receiver() = default;
        virtual ~Receiver() = 0;

        /**
         * @brief Whether the receiver is connected to any signal
         *
         * @see Emitter::hasSignalConnections(), slotConnectionCount()
         */
        inline bool hasSlotConnections() const {
            return !connections.empty();
        }

        /**
         * @brief Count of connections to this receiver slots
         *
         * @see Emitter::signalConnectionCount(), hasSlotConnections()
         */
        inline std::size_t slotConnectionCount() const { return connections.size(); }

        /**
         * @brief Disconnect everything from this receiver slots
         *
         * @see Emitter::disconnectAllSignals(), Connection::disconnect(),
         *      Emitter::disconnectSignal(), hasSlotConnections()
         */
        void disconnectAllSlots();

    private:
        std::vector<Implementation::AbstractConnectionData*> connections;
};

}}

#endif
