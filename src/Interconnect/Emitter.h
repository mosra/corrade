#ifndef Corrade_Interconnect_Emitter_h
#define Corrade_Interconnect_Emitter_h
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
 * @brief Class Corrade::Interconnect::Emitter
 */

#include <cstdint>
#include <unordered_map>

#include "Connection.h"

namespace Corrade { namespace Interconnect {

class Emitter;
class Receiver;

#ifndef DOXYGEN_GENERATING_OUTPUT
namespace Implementation {

class SignalDataHash {
    public:
        inline std::size_t operator()(const SignalData& data) const {
            std::size_t hash = 0;
            for(std::size_t i = 0; i != SignalData::Size/sizeof(std::size_t); ++i) {
                const std::size_t* const p = reinterpret_cast<const std::size_t*>(data.data+i*sizeof(std::size_t));
                hash ^= *p;
            }
            return hash;
        }
};

class CORRADE_INTERCONNECT_EXPORT AbstractConnectionData {
    template<class...> friend class MemberConnectionData;
    friend class Interconnect::Connection;
    friend class Interconnect::Emitter;
    friend class Interconnect::Receiver;

    AbstractConnectionData(const AbstractConnectionData&) = delete;
    AbstractConnectionData(AbstractConnectionData&&) = delete;
    AbstractConnectionData& operator=(const AbstractConnectionData&) = delete;
    AbstractConnectionData& operator=(AbstractConnectionData&&) = delete;

    public:
        explicit AbstractConnectionData() = default;
        inline virtual ~AbstractConnectionData() {}

    private:
        inline AbstractConnectionData(Emitter* emitter, Receiver* receiver): connection(nullptr), emitter(emitter), receiver(receiver), lastHandledSignal(0) {}

        Connection* connection;
        Emitter* emitter;
        Receiver* receiver;
        std::uint32_t lastHandledSignal;
};

template<class ...Args> class MemberConnectionData: public AbstractConnectionData {
    friend class Interconnect::Emitter;

    private:
        typedef void(Receiver::*Slot)(Args...);

        explicit MemberConnectionData() = default;

        template<class Emitter, class Receiver> inline MemberConnectionData(Emitter* emitter, Receiver* receiver, void(Receiver::*slot)(Args...)): AbstractConnectionData(emitter, receiver), slot(static_cast<Slot>(slot)) {}

        void handle(Args... args) {
            (receiver->*slot)(args...);
        }

        const Slot slot;
};

}
#endif

/**
@brief %Emitter object

Contains signals and manages connections between signals and slots. See
@ref interconnect for introduction.
@see Receiver, Connection
@todo Allow move
*/
class CORRADE_INTERCONNECT_EXPORT Emitter {
    friend class Connection;
    friend class Receiver;

    Emitter(const Emitter&) = delete;
    Emitter(Emitter&&) = delete;
    Emitter& operator=(const Emitter&) = delete;
    Emitter& operator=(Emitter&&) = delete;

    public:
        /**
         * @brief Signature for signals
         *
         * See emit() for more information about implementing signals.
         */
        class Signal {
            friend class Emitter;

            private:
                constexpr explicit Signal() = default;
        };

        inline explicit Emitter(): lastHandledSignal(0), connectionsChanged(false) {}
        virtual ~Emitter() = 0;

        /**
         * @brief Whether the emitter is connected to any slot
         *
         * @see Receiver::hasSlotConnections(), Connection::isConnected(),
         *      signalConnectionCount()
         */
        inline bool hasSignalConnections() const {
            return !connections.empty();
        }

        /**
         * @brief Whether given signal is connected to any slot
         *
         * @see Receiver::hasSlotConnections(), Connection::isConnected(),
         *      signalConnectionCount()
         */
        template<class Emitter, class ...Args> inline bool hasSignalConnections(Signal(Emitter::*signal)(Args...)) const {
            return connections.count(Implementation::SignalData(signal)) != 0;
        }

        /**
         * @brief Count of connections to this emitter signals
         *
         * @see Receiver::slotConnectionCount(), hasSignalConnections()
         */
        inline std::size_t signalConnectionCount() const { return connections.size(); }

        /**
         * @brief Count of slots connected to given signal
         *
         * @see Receiver::slotConnectionCount(), hasSignalConnections()
         */
        template<class Emitter, class ...Args> inline std::size_t signalConnectionCount(Signal(Emitter::*signal)(Args...)) const {
            return connections.count(Implementation::SignalData(signal));
        }

        /**
         * @brief Connect signal to slot
         * @param emitter       %Emitter
         * @param signal        %Signal
         * @param receiver      %Receiver
         * @param slot          Slot
         *
         * Connects given signal to compatible slot in receiver object.
         * @p emitter must be subclass of Emitter, @p signal must be
         * implemented signal (see emit() for more information), @p receiver
         * must be subclass of Receiver and @p slot must be non-constant
         * member function with `void` as return type. The argument count and
         * types must be exactly the same.
         *
         * The connection is automatically removed when either emitter or
         * receiver object is destroyed. You can use returned Connection
         * object to remove this particular connection, call disconnectSignal()
         * or disconnectAllSignals() to disconnect one signal or whole emitter
         * from everything or Receiver::disconnectAllSlots() to disconnect the
         * receiver from everything.
         *
         * Example usage:
         * @code
         * Postman postman;
         * Mailbox mailbox;
         * Emitter::connect(&postman, &Postman::messageDelivered, &mailbox, &Mailbox::addMessage);
         * @endcode
         *
         * Connecting with ability to remove (and possibly reestablish) this
         * particular connection using Connection::disconnect() or
         * Connection::connect():
         * @code
         * Connection c = Emitter::connect(&postman, &Postman::messageDelivered, &mailbox, &Mailbox::addMessage);
         * // ...
         * c.disconnect();
         * // ...
         * c.connect();
         * // ...
         * @endcode
         *
         * @attention It is possible to connect given signal to given slot
         *      more than once, because there is no way to check whether the
         *      connection already exists. As a result, after signal is
         *      emitted, the slot function will be then called more than once.
         * @attention In the slot you can add or remove connenctions and also
         *      delete receiver object, however you can't delete emitter
         *      object, as it would lead to undefined behavior.
         *
         * @see hasSignalConnections(), Connection::isConnected(),
         *      signalConnectionCount()
         *
         * @todo Connecting to signals
         * @todo Connecting to non-member functions and lambda functions
         */
        template<class Object, class Emitter, class Receiver, class ...Args> inline static
        #ifndef DOXYGEN_GENERATING_OUTPUT
        typename std::enable_if<std::is_base_of<Emitter, Object>::value, Connection>::type
        #else
        Connection
        #endif
        connect(Object* emitter, Signal(Emitter::*signal)(Args...), Receiver* receiver, void(Receiver::*slot)(Args...)) {
            static_assert(sizeof(Signal(Emitter::*)(Args...)) <= 2*sizeof(void*),
                "Size of member function pointer is incorrectly assumed to be smaller than 2*sizeof(void*)");

            Implementation::SignalData signalData(signal);
            auto data = new Implementation::MemberConnectionData<Args...>(emitter, receiver, slot);
            connect(signalData, data);
            return Connection(signalData, data);
        }

        /**
         * @brief Disconnect signal
         *
         * Disconnects all slots connected to given signal.
         *
         * Example usage:
         * @code
         * Postman postman;
         * postman.disconnect(&postman, &Postman::messageDelivered);
         * @endcode
         *
         * @see Connection::disconnect(), disconnectAllSignals(),
         *      Receiver::disconnectAllSlots(), hasSignalConnections()
         */
        template<class Emitter, class ...Args> inline void disconnectSignal(Signal(Emitter::*signal)(Args...)) {
            disconnect(Implementation::SignalData(signal));
        }

        /**
         * @brief Disconnect everything from this emitter signals
         *
         * @see Receiver::disconnectAllSlots(), Connection::disconnect(),
         *      disconnectSignal(), hasSignalConnections()
         */
        void disconnectAllSignals();

    protected:
        /**
         * @brief Emit signal
         * @param signal        %Signal
         * @param args          Arguments
         *
         * %Signal function implementation -- emits signal with given
         * arguments to all connected receivers. %Signal function must be
         * member function with Signal as return type, argument count and
         * types are not limited.
         *
         * Example signal implementations:
         * @code
         * class Postman: public Interconnect::Emitter {
         *     public:
         *         Signal messageDelivered(const std::string& message, int price = 0) {
         *             return emit(&Postman::messageDelivered, message, price);
         *         }
         *
         *         Signal paymentRequired(int amount) {
         *             return emit(&Postman::paymentRequired, amount);
         *         }
         * };
         * @endcode
         *
         * The implemented signal can be emitted simply by calling the
         * function:
         * @code
         * Postman postman;
         * postman.messageDelivered("hello");
         * postman.paymentRequired(245);
         * @endcode
         *
         * @todo Allow emitting slots only privately
         */
        template<class Emitter, class ...Args> Signal emit(Signal(Emitter::*signal)(Args...), typename std::common_type<Args>::type... args) {
            connectionsChanged = false;
            ++lastHandledSignal;
            auto range = connections.equal_range(Implementation::SignalData(signal));
            auto it = range.first;
            while(it != range.second) {
                /* If not already handled, proceed and mark as such */
                if(it->second->lastHandledSignal != lastHandledSignal) {
                    it->second->lastHandledSignal = lastHandledSignal;
                    static_cast<Implementation::MemberConnectionData<Args...>*>(it->second)->handle(args...);

                    /* Connections changed by the slot, go through again */
                    if(connectionsChanged) {
                        range = connections.equal_range(Implementation::SignalData(signal));
                        it = range.first;
                        connectionsChanged = false;
                        continue;
                    }
                }

                /* Nothing called or changed, next connection */
                ++it;
            }

            return {};
        }

    private:
        static void connect(const Implementation::SignalData& signal, Implementation::AbstractConnectionData* data);
        static void disconnect(const Implementation::SignalData& signal, Implementation::AbstractConnectionData* data);
        void disconnect(const Implementation::SignalData& signal);
        void disconnect(std::unordered_multimap<Implementation::SignalData, Implementation::AbstractConnectionData*, Implementation::SignalDataHash>::const_iterator it);

        std::unordered_multimap<Implementation::SignalData, Implementation::AbstractConnectionData*, Implementation::SignalDataHash> connections;
        std::uint32_t lastHandledSignal;
        bool connectionsChanged;
};

}}

#endif
