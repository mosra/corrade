#ifndef Corrade_Interconnect_Connection_h
#define Corrade_Interconnect_Connection_h
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

/** @file
 * @brief Class @ref Corrade::Interconnect::Connection
 */

#include <cstddef>

#include "Corrade/Interconnect/Interconnect.h"
#include "Corrade/Interconnect/visibility.h"

namespace Corrade { namespace Interconnect {

namespace Implementation {
    class AbstractConnectionData;
    struct SignalDataHash;

    class SignalData {
        friend Interconnect::Emitter;
        friend SignalDataHash;

        public:
            enum: std::size_t { Size = 2*sizeof(void*)/sizeof(std::size_t) };

            #ifndef CORRADE_MSVC2017_COMPATIBILITY
            template<class Emitter, class ...Args> SignalData(typename Emitter::Signal(Emitter::*signal)(Args...)): data() {
                typedef typename Emitter::Signal(Emitter::*Signal)(Args...);
                *reinterpret_cast<Signal*>(data) = signal;
            }
            #else
            /* MSVC is not able to detect template parameters, so I need to
               shovel these in explicitly using "static constructor" */
            template<class Emitter, class ...Args> static SignalData create(typename Emitter::Signal(Emitter::*signal)(Args...)) {
                typedef typename Emitter::Signal(Emitter::*Signal)(Args...);
                SignalData d;
                *reinterpret_cast<Signal*>(d.data) = signal;
                return d;
            }
            #endif

            bool operator==(const SignalData& other) const {
                for(std::size_t i = 0; i != Size; ++i)
                    if(data[i] != other.data[i]) return false;
                return true;
            }

            bool operator!=(const SignalData& other) const {
                return !operator==(other);
            }

        private:
            #ifdef CORRADE_MSVC2017_COMPATIBILITY
            SignalData(): data() {}
            #endif

            std::size_t data[Size];
    };
}

/**
@brief Connection

Returned by @ref Interconnect::connect(), allows to remove or reestablish the
connection. Destruction of Connection object does not remove the connection,
after that the only possibility to remove the connection is to disconnect whole
emitter or receiver or disconnect everything connected to given signal using
@ref Emitter::disconnectSignal(), @ref Emitter::disconnectAllSignals() or
@ref Receiver::disconnectAllSlots() or destroy either emitter or receiver
object.

@see @ref interconnect, @ref Emitter, @ref Receiver
*/
class CORRADE_INTERCONNECT_EXPORT Connection {
    friend Emitter;
    friend Receiver;

    public:
        /** @brief Copying is not allowed */
        Connection(const Connection&) = delete;

        /** @brief Move constructor */
        Connection(Connection&& other) noexcept;

        /** @brief Copying is not allowed */
        Connection& operator=(const Connection&) = delete;

        /** @brief Move assignment */
        Connection& operator=(Connection&& other) noexcept;

        /**
         * @brief Destructor
         *
         * Does not remove the connection.
         */
        ~Connection();

        /**
         * @brief Whether connection is possible
         * @return `False` if either emitter or receiver object (if applicable)
         *      doesn't exist anymore, `true` otherwise.
         *
         * @see @ref isConnected()
         */
        bool isConnectionPossible() const { return _data; }

        /**
         * @brief Whether the connection exists
         *
         * @see @ref isConnectionPossible(),
         *      @ref Emitter::hasSignalConnections(),
         *      @ref Receiver::hasSlotConnections()
         */
        bool isConnected() const { return _connected; }

        /**
         * @brief Establish the connection
         *
         * If connection is not possible, returns `false`, otherwise creates
         * the connection (if not already connected) and returns `true`.
         * @see @ref isConnectionPossible(), @ref isConnected(),
         *      @ref Interconnect::connect()
         */
        bool connect();

        /**
         * @brief Remove the connection
         *
         * Disconnects if connection exists.
         * @see @ref isConnected(), @ref Emitter::disconnectSignal(),
         *      @ref Emitter::disconnectAllSignals(),
         *      @ref Receiver::disconnectAllSlots()
         */
        void disconnect();

    #ifdef DOXYGEN_GENERATING_OUTPUT
    private:
    #endif
        explicit Connection(Implementation::SignalData signal, Implementation::AbstractConnectionData* data);

    private:
        Implementation::SignalData _signal;
        Implementation::AbstractConnectionData* _data;
        bool _connected;
};

}}

#endif
