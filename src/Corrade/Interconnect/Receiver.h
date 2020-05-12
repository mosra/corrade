#ifndef Corrade_Interconnect_Receiver_h
#define Corrade_Interconnect_Receiver_h
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

/** @file
 * @brief Class @ref Corrade::Interconnect::Receiver
 */

#include <cstddef>
#include <vector>

#include "Corrade/Interconnect/Interconnect.h"
#include "Corrade/Interconnect/visibility.h"

namespace Corrade { namespace Interconnect {

namespace Implementation { struct ReceiverConnection; }

/**
@brief Receiver object

Contains member function slots. See @ref interconnect for introduction.
@see @ref Emitter, @ref Connection
@todo Allow move
*/
class CORRADE_INTERCONNECT_EXPORT Receiver {
    public:
        explicit Receiver();

        /** @brief Copying is not allowed */
        Receiver(const Receiver&) = delete;

        /** @brief Moving is not allowed */
        Receiver(Receiver&&) = delete;

        /** @brief Copying is not allowed */
        Receiver& operator=(const Receiver&) = delete;

        /** @brief Moving is not allowed */
        Receiver& operator=(Receiver&&) = delete;

        /**
         * @brief Whether the receiver is connected to any signal
         *
         * @see @ref Emitter::hasSignalConnections(),
         *      @ref slotConnectionCount()
         */
        bool hasSlotConnections() const;

        /**
         * @brief Count of connections to this receiver slots
         *
         * @see @ref Emitter::signalConnectionCount(),
         *      @ref hasSlotConnections()
         */
        std::size_t slotConnectionCount() const;

        /**
         * @brief Disconnect everything from this receiver slots
         *
         * @see @ref Emitter::disconnectAllSignals(),
         *      @ref Connection::disconnect(),
         *      @ref Emitter::disconnectSignal(), @ref hasSlotConnections()
         */
        void disconnectAllSlots();

    protected:
        /* Nobody will need to have (and delete) Receiver*, thus this is faster
           than public pure virtual destructor */
        ~Receiver();

    private:
        /* https://bugzilla.gnome.org/show_bug.cgi?id=776986 */
        #ifndef DOXYGEN_GENERATING_OUTPUT
        friend Implementation::ConnectionData;
        friend Emitter;
        #endif

        std::vector<Implementation::ReceiverConnection> _connections;
};

}}

#endif
