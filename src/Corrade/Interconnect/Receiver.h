#ifndef Corrade_Interconnect_Receiver_h
#define Corrade_Interconnect_Receiver_h
/*
    This file is part of Corrade.

    Copyright © 2007, 2008, 2009, 2010, 2011, 2012, 2013
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

/** @file
 * @brief Class Corrade::Interconnect::Receiver
 */

#include <vector>

#include "Corrade/Interconnect/visibility.h"

namespace Corrade { namespace Interconnect {

namespace Implementation {
    class AbstractConnectionData;
}

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
        explicit Receiver();

        /**
         * @brief Whether the receiver is connected to any signal
         *
         * @see Emitter::hasSignalConnections(), slotConnectionCount()
         */
        bool hasSlotConnections() const {
            return !connections.empty();
        }

        /**
         * @brief Count of connections to this receiver slots
         *
         * @see Emitter::signalConnectionCount(), hasSlotConnections()
         */
        std::size_t slotConnectionCount() const { return connections.size(); }

        /**
         * @brief Disconnect everything from this receiver slots
         *
         * @see Emitter::disconnectAllSignals(), Connection::disconnect(),
         *      Emitter::disconnectSignal(), hasSlotConnections()
         */
        void disconnectAllSlots();

    protected:
        /* Nobody will need to have (and delete) Receiver*, thus this is faster
           than public pure virtual destructor */
        ~Receiver();

    private:
        std::vector<Implementation::AbstractConnectionData*> connections;
};

}}

#endif
