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

#include "Interconnect/Connection.h"
#include "Utility/Assert.h"

#include "corradeConfigure.h"

namespace Corrade { namespace Interconnect {

namespace Implementation {

class SignalDataHash {
    public:
        std::size_t operator()(const SignalData& data) const {
            std::size_t hash = 0;
            for(std::size_t i = 0; i != SignalData::Size; ++i)
                hash ^= data.data[i];
            return hash;
        }
};

}

/**
@brief %Emitter object

Contains signals and manages connections between signals and slots. See
@ref interconnect for introduction.

@section Emitter-signals Implementing signals

Signals are implemented as member functions with @ref Signal as return type,
argument count and types are not limited. The body consists of single
@ref emit() call, to which you pass pointer to the function and forward all
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
various @ref Interconnect::connect() "connect()" functions. The argument count
and types of slot function must be exactly the same as of the signal function.
When a connection is established, returned Connection object can be used to
remove or reestablish given connection using @ref Connection::disconnect() or
@ref Connection::connect():
@code
Connection c = Emitter::connect(...);
// ...
c.disconnect();
// ...
c.connect();
// ...
@endcode

You can also call @ref disconnectSignal() or @ref disconnectAllSignals() on
emitter to remove the connections. All emitter connections are automatically
removed when emitter object is destroyed.

@attention It is possible to connect given signal to given slot more than
    once, because there is no way to check whether the connection already
    exists. As a result, after signal is emitted, the slot function will be
    then called more than once.
@attention In the slot you can add or remove connections, however you can't
    delete the emitter object, as it would lead to undefined behavior.

You can connect any signal, as long as the emitter object is of proper type:
@code
class Base: public Interconnect::Emitter {
    public:
        Slot baseSignal();
};

class Derived: public Base {
    public:
        Slot derivedSignal();
};

Base* a = new Derived;
Derived* b = new Derived;
Interconnect::connect(*a, &Base::baseSignal, ...);       // ok
Interconnect::connect(*b, &Base::baseSignal, ...);       // ok
Interconnect::connect(*a, &Derived::derivedSignal, ...); // error, `a` is not of Derived type
Interconnect::connect(*b, &Derived::derivedSignal, ...); // ok
@endcode

There are a few slot types, each type has its particular use:

@subsection Emitter-connections-member Member function slots

When connecting to member function slot with @ref Interconnect::connect() "connect()",
@p receiver must be subclass of @ref Receiver and @p slot must be non-constant
member function with `void` as return type.

In addition to the cases mentioned above, the connection is automatically
removed also when receiver object is destroyed. You can also use
@ref Receiver::disconnectAllSlots() to disconnect the receiver from everything.

@note It is perfectly safe to delete receiver object in its own slot.

Example usage:
@code
class Mailbox: public Interconnect::Receiver {
    public:
        void addMessage(const std::string& message, int price);
};

Postman postman;
Mailbox mailbox;
Interconnect::connect(&postman, &Postman::messageDelivered, &mailbox, &Mailbox::addMessage);
@endcode

You can connect to any member function, as long as %Receiver exists somewhere
in given object type hierarchy:
@code
class Foo: public Interconnect::Emitter {
    public:
        Signal signal();
};

class Base: public Interconnect::Receiver {
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

Interconnect::connect(&foo, &Foo::signal, a, &Base::baseSlot);       // ok
Interconnect::connect(&foo, &Foo::signal, b, &Base::baseSlot);       // ok
Interconnect::connect(&foo, &Foo::signal, a, &Derived::derivedSlot); // error, `a` is not of Derived type
Interconnect::connect(&foo, &Foo::signal, b, &Derived::derivedSlot); // ok
@endcode

It is also possible to connect to member function of class which itself isn't
subclass of %Receiver, just add %Receiver using multiple inheritance.
Convoluted example:
@code
class MyString: public std::string, public Receiver {};

std::string a;
MyString b;
Interconnect::connect(&foo, &Foo::signal, &a, &std::string::clear); // error, `a` is not of Receiver type
Interconnect::connect(&foo, &Foo::signal, &b, &std::string::clear); // ok
@endcode

@see @ref Receiver, @ref Connection
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
         * See @ref emit() for more information about implementing signals.
         */
        class Signal {
            friend class Emitter;

            private:
                constexpr explicit Signal() = default;
        };

        #ifdef CORRADE_BUILD_DEPRECATED
        /**
         * @copybrief Interconnect::connect()
         * @deprecated Use @ref Corrade::Interconnect::connect() "Interconnect::connect()"
         *      instead.
         */
        template<class EmitterObject, class Emitter, class Receiver, class ReceiverObject, class ...Args> static Connection connect(EmitterObject* emitter, Signal(Emitter::*signal)(Args...), ReceiverObject* receiver, void(Receiver::*slot)(Args...)) {
            return connectInternal(*emitter, signal, *receiver, slot);
        }
        #endif

        explicit Emitter();

        /**
         * @brief Whether the emitter is connected to any slot
         *
         * @see Receiver::hasSlotConnections(), Connection::isConnected(),
         *      signalConnectionCount()
         */
        bool hasSignalConnections() const {
            return !connections.empty();
        }

        /**
         * @brief Whether given signal is connected to any slot
         *
         * @see Receiver::hasSlotConnections(), Connection::isConnected(),
         *      signalConnectionCount()
         */
        template<class Emitter, class ...Args> bool hasSignalConnections(Signal(Emitter::*signal)(Args...)) const {
            return connections.count(Implementation::SignalData(signal)) != 0;
        }

        /**
         * @brief Count of connections to this emitter signals
         *
         * @see Receiver::slotConnectionCount(), hasSignalConnections()
         */
        std::size_t signalConnectionCount() const { return connections.size(); }

        /**
         * @brief Count of slots connected to given signal
         *
         * @see Receiver::slotConnectionCount(), hasSignalConnections()
         */
        template<class Emitter, class ...Args> std::size_t signalConnectionCount(Signal(Emitter::*signal)(Args...)) const {
            return connections.count(Implementation::SignalData(signal));
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
        template<class Emitter, class ...Args> void disconnectSignal(Signal(Emitter::*signal)(Args...)) {
            disconnectInternal(Implementation::SignalData(signal));
        }

        /**
         * @brief Disconnect everything from this emitter signals
         *
         * @see Receiver::disconnectAllSlots(), Connection::disconnect(),
         *      disconnectSignal(), hasSignalConnections()
         */
        void disconnectAllSignals();

    protected:
        /* Nobody will need to have (and delete) Emitter*, thus this is faster
           than public pure virtual destructor */
        ~Emitter();

        /**
         * @brief Emit signal
         * @param signal        %Signal
         * @param args          Arguments
         *
         * See @ref Emitter-signals "class documentation" for more information
         * about implementing signals.
         */
        template<class Emitter, class ...Args> Signal emit(Signal(Emitter::*signal)(Args...), typename std::common_type<Args>::type... args);

    private:
        template<class EmitterObject, class Emitter, class Receiver, class ReceiverObject, class ...Args> friend Connection connect(EmitterObject&, Signal(Emitter::*)(Args...), ReceiverObject&, void(Receiver::*)(Args...));
        template<class EmitterObject, class Emitter, class ...Args> friend Connection connect(EmitterObject&, Signal(Emitter::*)(Args...), void(*)(Args...));

        static void connectInternal(const Implementation::SignalData& signal, Implementation::AbstractConnectionData* data);
        static void disconnectInternal(const Implementation::SignalData& signal, Implementation::AbstractConnectionData* data);

        void disconnectInternal(const Implementation::SignalData& signal);
        void disconnectInternal(std::unordered_multimap<Implementation::SignalData, Implementation::AbstractConnectionData*, Implementation::SignalDataHash>::const_iterator it);

        std::unordered_multimap<Implementation::SignalData, Implementation::AbstractConnectionData*, Implementation::SignalDataHash> connections;
        std::uint32_t lastHandledSignal;
        bool connectionsChanged;
};

namespace Implementation {

class CORRADE_INTERCONNECT_EXPORT AbstractConnectionData {
    template<class...> friend class FunctionConnectionData;
    template<class...> friend class MemberConnectionData;
    friend class Interconnect::Connection;
    friend class Interconnect::Emitter;
    friend class Interconnect::Receiver;

    AbstractConnectionData(const AbstractConnectionData&) = delete;
    AbstractConnectionData(AbstractConnectionData&&) = delete;
    AbstractConnectionData& operator=(const AbstractConnectionData&) = delete;
    AbstractConnectionData& operator=(AbstractConnectionData&&) = delete;

    public:
        enum class Type: std::uint8_t { Function, Member };

        virtual ~AbstractConnectionData() = 0;

    protected:
        explicit AbstractConnectionData(Emitter* emitter, Type type): connection(nullptr), emitter(emitter), lastHandledSignal(0), type(type) {}

    private:
        Connection* connection;
        Emitter* emitter;
        std::uint32_t lastHandledSignal;
        Type type;
};

class AbstractMemberConnectionData: public AbstractConnectionData {
    friend class Interconnect::Emitter;

    public:
        template<class Emitter, class Receiver> explicit AbstractMemberConnectionData(Emitter* emitter, Receiver* receiver): AbstractConnectionData(emitter, Type::Member), receiver(receiver) {}

    protected:
        Receiver* receiver;
};

template<class ...Args> class MemberConnectionData: public AbstractMemberConnectionData {
    friend class Interconnect::Emitter;

    public:
        typedef void(Receiver::*Slot)(Args...);

        template<class Emitter, class Receiver> explicit MemberConnectionData(Emitter* emitter, Receiver* receiver, void(Receiver::*slot)(Args...)): AbstractMemberConnectionData(emitter, receiver), slot(static_cast<Slot>(slot)) {}

    private:
        void handle(Args... args) {
            (receiver->*slot)(args...);
        }

        const Slot slot;
};

template<class ...Args> class FunctionConnectionData: public AbstractConnectionData {
    friend class Interconnect::Emitter;

    public:
        typedef void(*Slot)(Args...);

        template<class Emitter> explicit FunctionConnectionData(Emitter* emitter, Slot slot): AbstractConnectionData(emitter, Type::Function), slot(slot) {}

    private:
        void handle(Args... args) { slot(args...); }

        const Slot slot;
};

}

/**
@brief Connect signal to function slot
@param emitter       %Emitter
@param signal        %Signal
@param receiver      %Receiver
@param slot          Slot

Connects given signal to compatible slot. @p emitter must be subclass of
@ref Emitter, @p signal must be implemented signal and @p slot must be
non-member function or non-capturing lambda with `void` as return type. The
argument count and types must be exactly the same.

See @ref Emitter-connections "Emitter class documentation" for more information
about connections.

@see @ref Emitter::hasSignalConnections(), @ref Connection::isConnected(),
     @ref Emitter::signalConnectionCount()
*/
template<class EmitterObject, class Emitter, class ...Args> Connection connect(EmitterObject& emitter, Interconnect::Emitter::Signal(Emitter::*signal)(Args...), void(*slot)(Args...)) {
    static_assert(sizeof(Interconnect::Emitter::Signal(Emitter::*)(Args...)) <= 2*sizeof(void*),
        "Size of member function pointer is incorrectly assumed to be smaller than 2*sizeof(void*)");
    static_assert(std::is_base_of<Emitter, EmitterObject>::value,
        "Emitter object doesn't have given signal");

    Implementation::SignalData signalData(signal);
    auto data = new Implementation::FunctionConnectionData<Args...>(&emitter, slot);
    Interconnect::Emitter::connectInternal(signalData, data);
    return Connection(signalData, data);
}

/** @overload
@todo Why conversion of lambdas to function pointers is not done implicitly?
*/
template<class EmitterObject, class Emitter, class Lambda, class ...Args> Connection connect(EmitterObject& emitter, Interconnect::Emitter::Signal(Emitter::*signal)(Args...), Lambda slot) {
    return connect(emitter, signal, static_cast<void(*)(Args...)>(slot));
}

/**
@brief Connect signal to member function slot
@param emitter       %Emitter
@param signal        %Signal
@param receiver      %Receiver
@param slot          Slot

Connects given signal to compatible slot in receiver object. @p emitter must be
subclass of @ref Emitter, @p signal must be implemented signal, @p receiver
must be subclass of @ref Receiver and @p slot must be non-constant member
function with `void` as return type. The argument count and types must be
exactly the same.

See @ref Emitter-connections "Emitter class documentation" for more information
about connections.

@see @ref Emitter::hasSignalConnections(), @ref Connection::isConnected(),
     @ref Emitter::signalConnectionCount()
@todo Connecting to signals
*/
template<class EmitterObject, class Emitter, class Receiver, class ReceiverObject, class ...Args> Connection connect(EmitterObject& emitter, Interconnect::Emitter::Signal(Emitter::*signal)(Args...), ReceiverObject& receiver, void(Receiver::*slot)(Args...)) {
    static_assert(sizeof(Interconnect::Emitter::Signal(Emitter::*)(Args...)) <= 2*sizeof(void*),
        "Size of member function pointer is incorrectly assumed to be smaller than 2*sizeof(void*)");
    static_assert(std::is_base_of<Emitter, EmitterObject>::value,
        "Emitter object doesn't have given signal");
    static_assert(std::is_base_of<Receiver, ReceiverObject>::value,
        "Receiver object doesn't have given slot");

    Implementation::SignalData signalData(signal);
    auto data = new Implementation::MemberConnectionData<Args...>(&emitter, &receiver, static_cast<void(ReceiverObject::*)(Args...)>(slot));
    Interconnect::Emitter::connectInternal(signalData, data);
    return Connection(signalData, data);
}

#ifndef DOXYGEN_GENERATING_OUTPUT
template<class Emitter_, class ...Args> Emitter::Signal Emitter::emit(Signal(Emitter_::*signal)(Args...), typename std::common_type<Args>::type... args) {
    connectionsChanged = false;
    ++lastHandledSignal;
    auto range = connections.equal_range(Implementation::SignalData(signal));
    auto it = range.first;
    while(it != range.second) {
        /* If not already handled, proceed and mark as such */
        if(it->second->lastHandledSignal != lastHandledSignal) {
            it->second->lastHandledSignal = lastHandledSignal;
            switch(it->second->type) {
                case Implementation::AbstractConnectionData::Type::Function:
                    static_cast<Implementation::FunctionConnectionData<Args...>*>(it->second)->handle(args...);
                    break;
                case Implementation::AbstractConnectionData::Type::Member:
                    static_cast<Implementation::MemberConnectionData<Args...>*>(it->second)->handle(args...);
                    break;
                default:
                    CORRADE_ASSERT_UNREACHABLE();
            }

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
#endif

}}

#endif
