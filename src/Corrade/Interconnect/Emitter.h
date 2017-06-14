#ifndef Corrade_Interconnect_Emitter_h
#define Corrade_Interconnect_Emitter_h
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
 * @brief Class @ref Corrade::Interconnect::Emitter
 */

#include <cstddef>
#include <cstdint>
#include <type_traits>
#include <unordered_map>
#include <utility>

#include "Corrade/Interconnect/Connection.h"
#include "Corrade/Utility/Assert.h"

namespace Corrade { namespace Interconnect {

namespace Implementation {

struct SignalDataHash {
    std::size_t operator()(const SignalData& data) const {
        std::size_t hash = 0;
        for(std::size_t i = 0; i != SignalData::Size; ++i)
            hash ^= data.data[i];
        return hash;
    }
};

}

/**
@brief Emitter object

Contains signals and manages connections between signals and slots. See
@ref interconnect for introduction.

@anchor Interconnect-Emitter-signals
## Implementing signals

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

@anchor Interconnect-Emitter-connections
## Connecting signals to slots

Signals implemented on Emitter subclasses can be connected to slots using
various @ref Interconnect::connect() "connect()" functions. The argument count
and types of slot function must be exactly the same as of the signal function.
When a connection is established, returned @ref Connection object can be used
to remove or reestablish given connection using @ref Connection::disconnect()
or @ref Connection::connect():
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

### Member function slots

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

You can connect to any member function, as long as Receiver exists somewhere
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
subclass of Receiver, just add Receiver using multiple inheritance. Convoluted
example:
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
    friend Connection;
    friend Receiver;

    public:
        /**
         * @brief Signature for signals
         *
         * See @ref emit() for more information about implementing signals.
         */
        class Signal {
            friend Emitter;

            private:
                constexpr explicit Signal() = default;
        };

        explicit Emitter();

        /** @brief Copying is not allowed */
        Emitter(const Emitter&) = delete;

        /** @brief Moving is not allowed */
        Emitter(Emitter&&) = delete;

        /** @brief Copying is not allowed */
        Emitter& operator=(const Emitter&) = delete;

        /** @brief Moving is not allowed */
        Emitter& operator=(Emitter&&) = delete;

        /**
         * @brief Whether the emitter is connected to any slot
         *
         * @see @ref Receiver::hasSlotConnections(),
         *      @ref Connection::isConnected(), @ref signalConnectionCount()
         */
        bool hasSignalConnections() const {
            return !_connections.empty();
        }

        /**
         * @brief Whether given signal is connected to any slot
         *
         * @see @ref Receiver::hasSlotConnections(),
         *      @ref Connection::isConnected(), @ref signalConnectionCount()
         */
        template<class Emitter, class ...Args> bool hasSignalConnections(Signal(Emitter::*signal)(Args...)) const {
            return _connections.count(
                #ifndef CORRADE_MSVC2017_COMPATIBILITY
                Implementation::SignalData(signal)
                #else
                Implementation::SignalData::create<Emitter, Args...>(signal)
                #endif
                ) != 0;
        }

        /**
         * @brief Count of connections to this emitter signals
         *
         * @see @ref Receiver::slotConnectionCount(),
         *      @ref hasSignalConnections()
         */
        std::size_t signalConnectionCount() const { return _connections.size(); }

        /**
         * @brief Count of slots connected to given signal
         *
         * @see @ref Receiver::slotConnectionCount(),
         *      @ref hasSignalConnections()
         */
        template<class Emitter, class ...Args> std::size_t signalConnectionCount(Signal(Emitter::*signal)(Args...)) const {
            return _connections.count(
                #ifndef CORRADE_MSVC2017_COMPATIBILITY
                Implementation::SignalData(signal)
                #else
                Implementation::SignalData::create<Emitter, Args...>(signal)
                #endif
                );
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
         * @see @ref Connection::disconnect(), @ref disconnectAllSignals(),
         *      @ref Receiver::disconnectAllSlots(),
         *      @ref hasSignalConnections()
         */
        template<class Emitter, class ...Args> void disconnectSignal(Signal(Emitter::*signal)(Args...)) {
            disconnectInternal(
                #ifndef CORRADE_MSVC2017_COMPATIBILITY
                Implementation::SignalData(signal)
                #else
                Implementation::SignalData::create<Emitter, Args...>(signal)
                #endif
                );
        }

        /**
         * @brief Disconnect everything from this emitter signals
         *
         * @see @ref Receiver::disconnectAllSlots(),
         *      @ref Connection::disconnect(), @ref disconnectSignal(),
         *      @ref hasSignalConnections()
         */
        void disconnectAllSignals();

    protected:
        /* Nobody will need to have (and delete) Emitter*, thus this is faster
           than public pure virtual destructor */
        ~Emitter();

        /**
         * @brief Emit signal
         * @param signal        Signal
         * @param args          Arguments
         *
         * See @ref Interconnect-Emitter-signals "class documentation" for more
         * information about implementing signals.
         */
        template<class Emitter, class ...Args> Signal emit(Signal(Emitter::*signal)(Args...), typename std::common_type<Args>::type... args);

    private:
        template<class EmitterObject, class Emitter, class Receiver, class ReceiverObject, class ...Args> friend Connection connect(EmitterObject&, Signal(Emitter::*)(Args...), ReceiverObject&, void(Receiver::*)(Args...));
        template<class EmitterObject, class Emitter, class ...Args> friend Connection connect(EmitterObject&, Signal(Emitter::*)(Args...), void(*)(Args...));

        static void connectInternal(const Implementation::SignalData& signal, Implementation::AbstractConnectionData* data);
        static void disconnectInternal(const Implementation::SignalData& signal, Implementation::AbstractConnectionData* data);

        void disconnectInternal(const Implementation::SignalData& signal);
        void disconnectInternal(std::unordered_multimap<Implementation::SignalData, Implementation::AbstractConnectionData*, Implementation::SignalDataHash>::const_iterator it);

        std::unordered_multimap<Implementation::SignalData, Implementation::AbstractConnectionData*, Implementation::SignalDataHash> _connections;
        std::uint32_t _lastHandledSignal;
        bool _connectionsChanged;
};

namespace Implementation {

class CORRADE_INTERCONNECT_EXPORT AbstractConnectionData {
    template<class...> friend class FunctionConnectionData;
    template<class, class...> friend class MemberConnectionData;
    friend Interconnect::Connection;
    friend Interconnect::Emitter;
    friend Interconnect::Receiver;

    public:
        enum class Type: std::uint8_t { Function, Member };

        AbstractConnectionData(const AbstractConnectionData&) = delete;
        AbstractConnectionData(AbstractConnectionData&&) = delete;

        virtual ~AbstractConnectionData() = 0;

        AbstractConnectionData& operator=(const AbstractConnectionData&) = delete;
        AbstractConnectionData& operator=(AbstractConnectionData&&) = delete;

    protected:
        explicit AbstractConnectionData(Emitter* emitter, Type type): _connection{nullptr}, _emitter{emitter}, _lastHandledSignal{0}, _type{type} {}

    private:
        Connection* _connection;
        Emitter* _emitter;
        std::uint32_t _lastHandledSignal;
        Type _type;
};

class AbstractMemberConnectionData: public AbstractConnectionData {
    friend Interconnect::Emitter;

    public:
        template<class Emitter, class Receiver> explicit AbstractMemberConnectionData(Emitter* emitter, Receiver* receiver): AbstractConnectionData{emitter, Type::Member}, _receiver{receiver} {}

    private:
        Receiver* _receiver;
};

template<class ...Args> class BaseMemberConnectionData: public AbstractMemberConnectionData {
    friend Interconnect::Emitter;

    public:
        template<class Emitter, class Receiver> explicit BaseMemberConnectionData(Emitter* emitter, Receiver* receiver): AbstractMemberConnectionData{emitter, receiver} {}

    private:
        virtual void handle(Args... args) = 0;
};

#if defined(__GNUC__) && !defined(__clang__)
/* GCC complains that this function is used but never defined. Clang is sane.
   MSVC too. WHAT THE FUCK, GCC? */
template<class ...Args> void BaseMemberConnectionData<Args...>::handle(Args...) {
    CORRADE_ASSERT_UNREACHABLE(); /* LCOV_EXCL_LINE */
}
#endif

template<class Receiver, class ...Args> class MemberConnectionData: public BaseMemberConnectionData<Args...> {
    friend Interconnect::Emitter;

    public:
        typedef void(Receiver::*Slot)(Args...);

        template<class Emitter> explicit MemberConnectionData(Emitter* emitter, Receiver* receiver, void(Receiver::*slot)(Args...)): BaseMemberConnectionData<Args...>(emitter, receiver), _receiver{receiver}, _slot{slot} {}

    private:
        void handle(Args... args) override final {
            (_receiver->*_slot)(args...);
        }

        Receiver* _receiver;
        const Slot _slot;
};

template<class ...Args> class FunctionConnectionData: public AbstractConnectionData {
    friend Interconnect::Emitter;

    public:
        typedef void(*Slot)(Args...);

        template<class Emitter> explicit FunctionConnectionData(Emitter* emitter, Slot slot): AbstractConnectionData{emitter, Type::Function}, _slot{slot} {}

    private:
        void handle(Args... args) { _slot(args...); }

        const Slot _slot;
};

}

/**
@brief Connect signal to function slot
@param emitter       Emitter
@param signal        Signal
@param slot          Slot

Connects given signal to compatible slot. @p emitter must be subclass of
@ref Emitter, @p signal must be implemented signal and @p slot must be
non-member function or non-capturing lambda with `void` as return type. The
argument count and types must be exactly the same.

See @ref Interconnect-Emitter-connections "Emitter class documentation" for
more information about connections.

@see @ref Emitter::hasSignalConnections(), @ref Connection::isConnected(),
     @ref Emitter::signalConnectionCount()
*/
template<class EmitterObject, class Emitter, class ...Args> Connection connect(EmitterObject& emitter, Interconnect::Emitter::Signal(Emitter::*signal)(Args...), void(*slot)(Args...)) {
    static_assert(sizeof(Interconnect::Emitter::Signal(Emitter::*)(Args...)) <= 2*sizeof(void*),
        "Size of member function pointer is incorrectly assumed to be smaller than 2*sizeof(void*)");
    static_assert(std::is_base_of<Emitter, EmitterObject>::value,
        "Emitter object doesn't have given signal");

    #ifndef CORRADE_MSVC2017_COMPATIBILITY
    Implementation::SignalData signalData(signal);
    #else
    auto signalData = Implementation::SignalData::create<EmitterObject, Args...>(signal);
    #endif
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
@param emitter       Emitter
@param signal        Signal
@param receiver      Receiver
@param slot          Slot

Connects given signal to compatible slot in receiver object. @p emitter must be
subclass of @ref Emitter, @p signal must be implemented signal, @p receiver
must be subclass of @ref Receiver and @p slot must be non-constant member
function with `void` as return type. The argument count and types must be
exactly the same.

See @ref Interconnect-Emitter-connections "Emitter class documentation" for
more information about connections.

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

    #ifndef CORRADE_MSVC2017_COMPATIBILITY
    Implementation::SignalData signalData(signal);
    #else
    auto signalData = Implementation::SignalData::create<EmitterObject, Args...>(signal);
    #endif
    auto data = new Implementation::MemberConnectionData<ReceiverObject, Args...>(&emitter, &receiver, slot);
    Interconnect::Emitter::connectInternal(signalData, data);
    return Connection(signalData, data);
}

#ifndef DOXYGEN_GENERATING_OUTPUT
template<class Emitter_, class ...Args> Emitter::Signal Emitter::emit(Signal(Emitter_::*signal)(Args...), typename std::common_type<Args>::type... args) {
    _connectionsChanged = false;
    ++_lastHandledSignal;
    auto range = _connections.equal_range(
        #ifndef CORRADE_MSVC2017_COMPATIBILITY
        Implementation::SignalData(signal)
        #else
        Implementation::SignalData::create<Emitter_, Args...>(signal)
        #endif
        );
    auto it = range.first;
    while(it != range.second) {
        /* If not already handled, proceed and mark as such */
        if(it->second->_lastHandledSignal != _lastHandledSignal) {
            it->second->_lastHandledSignal = _lastHandledSignal;
            switch(it->second->_type) {
                case Implementation::AbstractConnectionData::Type::Function:
                    static_cast<Implementation::FunctionConnectionData<Args...>*>(it->second)->handle(args...);
                    break;
                case Implementation::AbstractConnectionData::Type::Member:
                    static_cast<Implementation::BaseMemberConnectionData<Args...>*>(it->second)->handle(args...);
                    break;
                default:
                    CORRADE_ASSERT_UNREACHABLE(); /* LCOV_EXCL_LINE */
            }

            /* Connections changed by the slot, go through again */
            if(_connectionsChanged) {
                range = _connections.equal_range(
                    #ifndef CORRADE_MSVC2017_COMPATIBILITY
                    Implementation::SignalData(signal)
                    #else
                    Implementation::SignalData::create<Emitter_, Args...>(signal)
                    #endif
                    );
                it = range.first;
                _connectionsChanged = false;
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
