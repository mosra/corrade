#ifndef Corrade_Interconnect_Emitter_h
#define Corrade_Interconnect_Emitter_h
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
        inline virtual ~AbstractConnectionData() {}

    private:
        inline explicit AbstractConnectionData(Emitter* emitter, Receiver* receiver): connection(nullptr), emitter(emitter), receiver(receiver), lastHandledSignal(0) {}

        Connection* connection;
        Emitter* emitter;
        Receiver* receiver;
        std::uint32_t lastHandledSignal;
};

template<class ...Args> class MemberConnectionData: public AbstractConnectionData {
    friend class Interconnect::Emitter;

    private:
        typedef void(Receiver::*Slot)(Args...);

        template<class Emitter, class Receiver> inline explicit MemberConnectionData(Emitter* emitter, Receiver* receiver, void(Receiver::*slot)(Args...)): AbstractConnectionData(emitter, receiver), slot(static_cast<Slot>(slot)) {}

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

@section Emitter-signals Implementing signals

Signals are implemented as member functions with Signal as return type,
argument count and types are not limited. The body consists of single
emit() call, to which you pass pointer to the function and forward all
arguments. Example signal implementations:
@code
class Postman: public Interconnect::Emitter {
    public:
        Signal messageDelivered(const std::string& message, int price = 0) {
            return emit(&Postman::messageDelivered, message, price);
        }

        Signal paymentRequired(int amount) {
            return emit(&Postman::paymentRequired, amount);
        }
};
@endcode

The implemented signal can be emitted simply by calling the function:
@code
Postman postman;
postman.messageDelivered("hello");
postman.paymentRequired(245);
@endcode

If the signal is not declared as public function, it cannot be connected or
called from outside the class.

@section Emitter-connections Connecting signals to slots

Signals implemented on %Emitter subclasses can be connected to slots using
various connect() functions. The argument count and types of slot function
must be exactly the same as of the signal function. When a connection is
established, returned Connection object can be used to remove or reestablish
given connection using Connection::disconnect() or Connection::connect():
@code
Connection c = Emitter::connect(...);
// ...
c.disconnect();
// ...
c.connect();
// ...
@endcode

You can also call disconnectSignal() or disconnectAllSignals() on emitter to
remove the connections. All emitter connections are automatically removed
when emitter object is destroyed.

@attention It is possible to connect given signal to given slot more than
    once, because there is no way to check whether the connection already
    exists. As a result, after signal is emitted, the slot function will be
    then called more than once.
@attention In the slot you can add or remove connections, however you can't
    delete the emitter object, as it would lead to undefined behavior.

You can connect any signal, as long as the emitter object is of proper type:
@code
class Base: public Emitter {
    public:
        Slot baseSignal();
};

class Derived: public Base {
    public:
        Slot derivedSignal();
};

Base* a = new Derived;
Derived* b = new Derived;
Emitter::connect(a, &Base::baseSignal, ...);       // ok
Emitter::connect(b, &Base::baseSignal, ...);       // ok
Emitter::connect(a, &Derived::derivedSignal, ...); // error, `a` is not of Derived type
Emitter::connect(b, &Derived::derivedSignal, ...); // ok
@endcode

There are a few slot types, each type has its particular use:

@subsection Emitter-connections-member Member function slots

When connecting to member function slot with connect(), @p receiver must be
subclass of Receiver and @p slot must be non-constant member function with
`void` as return type.

In addition to the cases mentioned above, the connection is automatically
removed also when receiver object is destroyed. You can also use
Receiver::disconnectAllSlots() to disconnect the receiver from everything.

@note It is perfectly safe to delete receiver object in its own slot.

Example usage:
@code
class Mailbox: public Receiver {
    public:
        void addMessage(const std::string& message, int price);
};

Postman postman;
Mailbox mailbox;
Emitter::connect(&postman, &Postman::messageDelivered, &mailbox, &Mailbox::addMessage);
@endcode

You can connect to any member function, as long as %Receiver exists somewhere
in given object type hierarchy:
@code
class Foo {
    public:
        Signal signal();
};

class Base: public Receiver {
    public:
        void baseSlot();
};

class Derived: public Base {
    public:
        void derivedSlot();
};

Foo foo;
Base* a = new Derived;
Derived* b = new Derived;

Emitter::connect(&foo, &Foo::signal, a, &Base::baseSlot);       // ok
Emitter::connect(&foo, &Foo::signal, b, &Base::baseSlot);       // ok
Emitter::connect(&foo, &Foo::signal, a, &Derived::derivedSlot); // error, `a` is not of Derived type
Emitter::connect(&foo, &Foo::signal, b, &Derived::derivedSlot); // ok
@endcode

It is also possible to connect to member function of class which itself isn't
subclass of %Receiver, just add %Receiver with multiple inheritance.
Convoluted example:
@code
class MyString: public std::string, public Receiver {};

std::string a;
MyString b;
Emitter::connect(&foo, &Foo::signal, &a, &std::string::clear); // error, `a` is not of Receiver type
Emitter::connect(&foo, &Foo::signal, &b, &std::string::clear); // ok
@endcode

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
         * @brief Connect signal to member function slot
         * @param emitter       %Emitter
         * @param signal        %Signal
         * @param receiver      %Receiver
         * @param slot          Slot
         *
         * Connects given signal to compatible slot in receiver object.
         * @p emitter must be subclass of Emitter, @p signal must be
         * implemented signal, @p receiver must be subclass of Receiver and
         * @p slot must be non-constant member function with `void` as return
         * type. The argument count and types must be exactly the same.
         *
         * See @ref Emitter-connections "class documentation" for more
         * information about connections.
         *
         * @see hasSignalConnections(), Connection::isConnected(),
         *      signalConnectionCount()
         * @todo Connecting to signals
         * @todo Connecting to non-member functions and lambda functions
         */
        template<class EmitterObject, class Emitter, class Receiver, class ReceiverObject, class ...Args> inline static
        #ifdef DOXYGEN_GENERATING_OUTPUT
        Connection
        #else
        typename std::enable_if<std::is_base_of<Emitter, EmitterObject>::value && std::is_base_of<Receiver, ReceiverObject>::value, Connection>::type
        #endif
        connect(EmitterObject* emitter, Signal(Emitter::*signal)(Args...), ReceiverObject* receiver, void(Receiver::*slot)(Args...)) {
            static_assert(sizeof(Signal(Emitter::*)(Args...)) <= 2*sizeof(void*),
                "Size of member function pointer is incorrectly assumed to be smaller than 2*sizeof(void*)");

            Implementation::SignalData signalData(signal);
            auto data = new Implementation::MemberConnectionData<Args...>(emitter, receiver, static_cast<void(ReceiverObject::*)(Args...)>(slot));
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
         * See @ref Emitter-signals "class documentation" for more information
         * about implementing signals.
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

            return Signal();
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
