#ifndef Corrade_Interconnect_Connection_h
#define Corrade_Interconnect_Connection_h
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
 * @brief Class Corrade::Interconnect::Connection
 */

#include "corradeInterconnectVisibility.h"

#include <cstddef>

namespace Corrade { namespace Interconnect {

class Emitter;

#ifndef DOXYGEN_GENERATING_OUTPUT
namespace Implementation {
    class AbstractConnectionData;

    class SignalData {
        friend class Interconnect::Emitter;
        friend class SignalDataHash;

        public:
            static const std::size_t Size = 2*sizeof(void*);

            template<class Emitter, class ...Args> inline SignalData(typename Emitter::Signal(Emitter::*signal)(Args...)): data() {
                typedef typename Emitter::Signal(Emitter::*Signal)(Args...);
                *reinterpret_cast<Signal*>(data) = signal;
            }

            inline bool operator==(const SignalData& other) const {
                for(std::size_t i = 0; i != Size; ++i)
                    if(data[i] != other.data[i]) return false;
                return true;
            }

            inline bool operator!=(const SignalData& other) const {
                return !operator==(other);
            }

        private:
            char data[Size];
    };
}
#endif

/**
@brief %Connection

Returned by Emitter::connect(), allows to remove or reestablish the connection.
Destruction of %Connection object does not remove the connection, after that
the only possibility to remove the connection is to disconnect whole emitter
or receiver or disconnect everything connected to given signal using
Emitter::disconnect() or Receiver::disconnect() or destroy either emitter or
receiver object.

@see @ref interconnect, Emitter, Receiver
*/
class CORRADE_INTERCONNECT_EXPORT Connection {
    friend class Emitter;
    friend class Receiver;

    Connection(const Connection&) = delete;
    Connection& operator=(const Connection&) = delete;

    public:
        /** @brief Move constructor */
        Connection(Connection&& other);

        /** @brief Move assignment */
        Connection& operator=(Connection&& other);

        /**
         * @brief Destructor
         *
         * Does not remove the connection.
         */
        ~Connection();

        /**
         * @brief Whether connection is possible
         * @return `False` if either emitter or receiver object doesn't exist
         *      anymore, `true` otherwise.
         *
         * @see isConnected()
         */
        inline bool isConnectionPossible() const { return data; }

        /**
         * @brief Whether the connection exists
         *
         * @see isConnectionPossible()
         */
        inline bool isConnected() const { return connected; }

        /**
         * @brief Establish the connection
         *
         * If connection is not possible, returns `false`, otherwise creates
         * the connection (if not already connected) and returns true.
         * @see isConnectionPossible(), isConnected()
         */
        bool connect();

        /**
         * @brief Remove the connection
         *
         * Disconnects if connection exists.
         * @see isConnected(), Emitter::disconnect(), Receiver::disconnect()
         */
        void disconnect();

    private:
        Connection(Implementation::SignalData signal, Implementation::AbstractConnectionData* data);

        void destroy();
        void move(Connection&& other);

        Implementation::SignalData signal;
        Implementation::AbstractConnectionData* data;
        bool connected;
};

}}

#endif
