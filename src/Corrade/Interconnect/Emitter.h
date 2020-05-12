#ifndef Corrade_Interconnect_Emitter_h
#define Corrade_Interconnect_Emitter_h
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

/* Previously I used std::common_type<T>::type, but that decays inside so
   references are lost. OTOH std::type_identity is only C++20, so making my own
   thing here. */
template<class T> struct Identity {
    typedef T Type;
};

struct SignalDataHash {
    std::size_t operator()(const SignalData& data) const {
        std::size_t hash = 0;
        for(std::size_t i = 0; i != FunctionPointerSize; ++i)
            hash ^= data.data[i];
        return hash;
    }
};

enum class ConnectionType: std::uint8_t {
    Free,
    Member,
    Functor,
    FunctorWithDestructor
};

/* Thestd::has_trivial_copy_constructor is deprecated in GCC 5+ but we can't
   detect libstdc++ version when using Clang. The builtins aren't deprecated
   but for those GCC commits suicide with
    error: use of built-in trait ‘__has_trivial_copy(T)’ in function signature; use library traits instead
   so, well, i'm defining my own! See CORRADE_STD_IS_TRIVIALLY_TRAITS_SUPPORTED
   for even more fun stories. */
#ifndef CORRADE_STD_IS_TRIVIALLY_TRAITS_SUPPORTED
template<class T> struct IsTriviallyCopyableOnOldGcc: std::integral_constant<bool, __has_trivial_copy(T)> {};
#endif

struct CORRADE_INTERCONNECT_EXPORT ConnectionData {
    union CORRADE_ALIGNAS(sizeof(std::size_t)) Storage {
        char data[FunctionPointerSize*sizeof(std::size_t) + sizeof(void*)];
        void(*function)();
        struct {
            char data[FunctionPointerSize*sizeof(std::size_t)];
            Receiver* receiver;
        } member;
        struct {
            void* data;
            void(*destruct)(Storage&);
        } functor;
    };

    /* Construct a member function connection */
    template<class Receiver, class ReceiverObject, class ...Args> static ConnectionData createMember(ReceiverObject& receiver, void(Receiver::*slot)(Args...)) {
        ConnectionData out{ConnectionType::Member};
        reinterpret_cast<void(Receiver::*&)(Args...)>(out.storage.member.data) = slot;
        out.storage.member.receiver = &receiver;
        out.call = reinterpret_cast<void(*)()>(static_cast<void(*)(Storage&, Args&&...)>([](Storage& storage, Args&&... args) {
            (static_cast<ReceiverObject*>(storage.member.receiver)->*reinterpret_cast<void(Receiver::*&)(Args...)>(storage.member.data))(std::forward<Args>(args)...);
        }));
        return out;
    }

    /* Construct a free function connection */
    template<class ...Args, class F> static ConnectionData createFunctor(F&& f, typename std::enable_if<std::is_convertible<typename std::decay<F>::type, void(*)(Args...)>::value>::type* = nullptr) {
        ConnectionData out{ConnectionType::Free};
        out.storage.function = reinterpret_cast<void(*)()>(static_cast<void(*)(Args...)>(f));
        out.call = reinterpret_cast<void(*)()>(static_cast<void(*)(Storage&, Args&&...)>([](Storage& storage, Args&&... args) {
            reinterpret_cast<void(*)(Args...)>(storage.function)(std::forward<Args>(args)...);
        }));
        return out;
    }

    /* Construct a simple, small enough and trivial functor connection (which
       is not convertible to a function pointer) */
    template<class ...Args, class F> static ConnectionData createFunctor(F&& f, typename std::enable_if<!std::is_convertible<typename std::decay<F>::type, void(*)(Args...)>::value && sizeof(typename std::decay<F>::type) <= sizeof(Storage) &&
        #ifdef CORRADE_STD_IS_TRIVIALLY_TRAITS_SUPPORTED
        std::is_trivially_copyable<typename std::decay<F>::type>::value
        #else
        (IsTriviallyCopyableOnOldGcc<typename std::decay<F>::type>::value && std::is_trivially_destructible<typename std::decay<F>::type>::value)
        #endif
    >::type* = nullptr) {
        ConnectionData out{ConnectionType::Functor};
        new(&out.storage.data) typename std::decay<F>::type{std::move(f)};
        out.call = reinterpret_cast<void(*)()>(static_cast<void(*)(Storage&, Args&&...)>([](Storage& storage, Args&&... args) {
            reinterpret_cast<F&>(storage.data)(std::forward<Args>(args)...);
        }));
        return out;
    }

    /* Construct a non-trivial or too large functor connection -- if something
       is not trivially destructible, it also isn't trivially copyable
       (potentially non-copyable) and thus we need to allocate it on heap. */
    template<class ...Args, class F> static ConnectionData createFunctor(F&& f, typename std::enable_if<
        /* On MSVC *all* lambdas are *for some reason* not trivially compable
           so the call gets ambiguous without the is_convertible check */
        !std::is_convertible<typename std::decay<F>::type, void(*)(Args...)>::value
        && ((sizeof(typename std::decay<F>::type) > sizeof(Storage)) || !
            #ifdef CORRADE_STD_IS_TRIVIALLY_TRAITS_SUPPORTED
            std::is_trivially_copyable<typename std::decay<F>::type>::value
            #else
            (IsTriviallyCopyableOnOldGcc<typename std::decay<F>::type>::value && std::is_trivially_destructible<typename std::decay<F>::type>::value)
            #endif
        )
        >::type* = nullptr) {
        ConnectionData out{ConnectionType::FunctorWithDestructor};
        reinterpret_cast<typename std::decay<F>::type*&>(out.storage.functor.data) = new typename std::decay<F>::type{std::move(f)};
        out.storage.functor.destruct = [](Storage& storage) {
            delete reinterpret_cast<typename std::decay<F>::type*>(storage.functor.data);
        };
        out.call = reinterpret_cast<void(*)()>(static_cast<void(*)(Storage&, Args&&...)>([](Storage& storage, Args&&... args) {
            (*reinterpret_cast<typename std::decay<F>::type*>(storage.functor.data))(std::forward<Args>(args)...);
        }));
        return out;
    }

    ConnectionData(ConnectionType type) noexcept: type{type} {}

    ConnectionData(const ConnectionData&) = delete;
    ConnectionData(ConnectionData&& other) noexcept;

    ~ConnectionData();

    ConnectionData& operator=(const ConnectionData&) = delete;
    ConnectionData& operator=(ConnectionData&& other) noexcept;

    Storage storage;
    void(*call)();
    std::uint32_t lastHandledSignal{};
    ConnectionType type;
};

}

/**
@brief Emitter object

Contains signals and manages connections between signals and slots. See
@ref interconnect for introduction.

@section Interconnect-Emitter-signals Implementing signals

Signals are implemented as member functions with @ref Signal as return type,
argument count and types are not limited. Their body consists of a single
@ref emit() call, to which you pass pointer to the function and forward all
arguments. Example signal implementations:

@snippet Interconnect.cpp Emitter-signals

The implemented signal can be emitted simply by calling the function:

@snippet Interconnect.cpp Emitter-emit

If the signal is not declared as public function, it cannot be connected or
called from outside the class.

@anchor Interconnect-Emitter-msvc-icf

<b></b>

@m_class{m-block m-warning}

@par Connecting slots across shared objects
    If you're planning to use signal/slot connections across shared objects, be
    aware that in combination with GCC/Clang `-fvisibility-inlines-hidden`
    option (enabled by default when you use the `CORRADE_USE_PEDANTIC_FLAGS`
    @ref corrade-cmake "CMake property"), address of given signal inside a
    shared library will differ from its address outside of it. Copied from GCC
    documentation:
@par
    <blockquote>
    This switch declares that the user does not attempt to compare pointers
    to inline methods where the addresses of the two functions were taken in
    different shared objects.
    </blockquote>
@par
    There are three possible solutions:
@par
    1.  Compile without `-fvisibility-inlines-hidden`. Not recommended, as you
        usually have no control over externally specified flags. This flag is
        not available on MSVC or MinGW and while MSVC "just works", MinGW
        doesn't.
    2.  Make the signal definition non-inline by moving its definition to a
        `*.cpp` file. Potentially very verbose, but works everywhere including
        MinGW.
    3.  Annotating the function with an export macro such as
        @ref CORRADE_VISIBILITY_INLINE_MEMBER_EXPORT from
        @ref Corrade/Utility/VisibilityMacros.h that puts its visibility back
        to default. Doesn't work on MinGW either.
@par
    Similar issue happens with the `-Wl,-Bsymbolic-functions` option passed to
    the linker. This flag is unfortunately present by default when building
    Ubuntu packages and has to be explicitly removed in order to make the
    signal connections work correctly. See the `package/debian/rules` file for
    an example how to do it.

<b></b>

@m_class{m-block m-danger}

@par MSVC and identical function merging
    MSVC linker has an optimization called "identical COMDAT folding"
    (`/OPT:ICF`) that looks for functions with identical generated machine code
    and merges them together to reduce binary size. This is unfortunately done
    also for functions which have their pointer taken and compared, making it a
    [non-conformant behavior](https://www.reddit.com/r/cpp/comments/3brezz/do_you_know_that_different_functions_may_have_the/)
    --- the C++ standard says that different functions should have different
    pointers.
@par
    Since the core functionality of @ref Emitter is based around taking
    pointers to signals and comparing them, this optimization breaks it. The
    only reliable solution is disabling the optimization altogether using
    `/OPT:NOICF,REF` (the `REF` part means that you don't want the functions
    merged but you still want to include only functions that are referenced ---
    a valid optimization). If you're using CMake and linking to
    `Corrade::Interconnect` (even transitively), this flag is added implicitly.
    If you're using a custom buildsystem, you have to add this flag yourself to
    prevent erratic behavior.

@section Interconnect-Emitter-connections Connecting signals to slots

Signals implemented on @ref Emitter subclasses can be connected to slots using
various @ref connect() functions. The argument count and types of slot function
must be exactly the same as of the signal function. When a connection is
established, returned @ref Connection object can be used together with
@ref disconnect() to remove given connection:

@snippet Interconnect.cpp Emitter-connect

Note that the @ref Connection object is just a handle --- its destruction
* *doesn't* lead to the connection being removed. You can also call
@ref disconnectSignal() or @ref disconnectAllSignals() on the emitter to remove
the connections. All emitter connections are automatically removed when emitter
object is destroyed.

@note It is possible to connect given signal to given slot more than
    once, because there is no way to check whether the connection already
    exists. As a result, after signal is emitted, the slot function will be
    then called more than once.
@note In the slot you can add or remove connections, however you can't
    @cpp delete @ce the emitter object, as it would lead to undefined behavior.

You can connect any signal, as long as the emitter object is of proper type ---
in particular, referring a signal from a derived type while passing an emitter
of base type is not allowed:

@snippet Interconnect.cpp Emitter-connect-emitter-type

@section Interconnect-Emitter-free-slots Free function, lambda and function object slots

Slots can be simply free functions. Non-capturing lambdas, shown above, are
converted to function pointers and treated the same. These have the least call
overhead.

Capturing lambdas and function objects, as long as they are trivially copyable
and destructible and small enough (not more than three pointers) are stored
by-value similarly to free functions, but the call overhead is slightly larger.

Lambdas and function objects larger than three pointers or having non-trivial
copy / destruction are allocated on heap. This is the case of @ref std::function,
for example, and the overhead is offset by another indirection.

@attention
    Note that when using capturing lambdas or stateful function objects you
    have to ensure that all referenced data are kept in scope for as long as
    the connection is alive, which might be tricky. A better recommendation is
    to use member function slots on a @ref Receiver object as shown below ---
    there the connection gets automatically removed once the receiver goes out
    of scope.

<b></b>

@m_class{m-block m-danger}

@par MSVC and non-trivially-copyable lambdas
    For some reason, @ref std::is_trivially_copyable returns @cpp false @ce for
    any lambda type on MSVC and thus all lambdas that are not convertible to
    plain function pointers are allocated on heap on this compiler. If this
    proves to be a performance bottleneck, you can work around this limitation
    by creating a function object capturing the needed state manually.

@section Interconnect-Emitter-member-slots Member function slots

Finally, it's possible to connect signals to member functions. The receiving
object must be a subclass of @ref Receiver and @p slot must be a non-constant
member function with @cpp void @ce as a return type. In addition to the cases
mentioned above, the connection is automatically removed also when receiver
object is destroyed. You can also use @ref Receiver::disconnectAllSlots() to
disconnect the receiver from everything.

@note Unlike with emitters, it's perfectly fine to destroy the *receiver*
    object in its own slot.

@snippet Interconnect.cpp Emitter-connect-member-slot

You can connect to any member function, as long as @ref Receiver exists
somewhere in given object type hierarchy:

@snippet Interconnect.cpp Emitter-connect-receiver-type

It is also possible to connect to member function of class which itself isn't
subclass of @ref Receiver, just add @ref Receiver using multiple inheritance.
Convoluted example:

@snippet Interconnect.cpp Emitter-connect-receiver-multiple-inheritance

@see @ref Receiver, @ref Connection
@todo Allow move
*/
class CORRADE_INTERCONNECT_EXPORT Emitter {
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
         * @see @ref Receiver::hasSlotConnections(), @ref isConnected(),
         *      @ref signalConnectionCount()
         */
        bool hasSignalConnections() const {
            return !_connections.empty();
        }

        /**
         * @brief Whether given signal is connected to any slot
         *
         * @see @ref Receiver::hasSlotConnections(), @ref isConnected(),
         *      @ref signalConnectionCount()
         */
        template<class Emitter, class ...Args> bool hasSignalConnections(Signal(Emitter::*signal)(Args...)) const {
            return _connections.count(
                #ifndef CORRADE_MSVC2019_COMPATIBILITY
                Implementation::SignalData(signal)
                #else
                Implementation::SignalData::create<Emitter, Args...>(signal)
                #endif
                ) != 0;
        }

        /**
         * @brief Whether given connection still exists
         *
         * Checks if the @ref Connection object returned by @ref connect()
         * still refers to an existing connection. It's the user responsibility
         * to ensure that the @p connection corresponds to proper @ref Emitter
         * instance.
         * @see @ref hasSignalConnections(),
         *      @ref Receiver::hasSlotConnections(), @ref disconnect()
         */
        bool isConnected(const Connection& connection) const;

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
                #ifndef CORRADE_MSVC2019_COMPATIBILITY
                Implementation::SignalData(signal)
                #else
                Implementation::SignalData::create<Emitter, Args...>(signal)
                #endif
                );
        }

        /**
         * @brief Disconnect signal
         *
         * Disconnects all slots connected to given signal. Example usage:
         *
         * @snippet Interconnect.cpp Emitter-disconnectSignal
         *
         * @see @ref Connection::disconnect(), @ref disconnectAllSignals(),
         *      @ref Receiver::disconnectAllSlots(),
         *      @ref hasSignalConnections()
         */
        template<class Emitter, class ...Args> void disconnectSignal(Signal(Emitter::*signal)(Args...)) {
            disconnectInternal(
                #ifndef CORRADE_MSVC2019_COMPATIBILITY
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
        template<class Emitter, class ...Args> Signal emit(Signal(Emitter::*signal)(Args...), typename Implementation::Identity<Args>::Type... args);

    private:
        /* https://bugzilla.gnome.org/show_bug.cgi?id=776986. Also the class
           docs link to this connect() instead of Interconnect::connect(). Ugh. */
        #ifndef DOXYGEN_GENERATING_OUTPUT
        friend Connection;
        friend Receiver;

        template<class EmitterObject, class Emitter, class Receiver, class ReceiverObject, class ...Args> friend Connection connect(EmitterObject&, Signal(Emitter::*)(Args...), ReceiverObject&, void(Receiver::*)(Args...));
        template<class EmitterObject, class Emitter, class Functor, class ...Args> friend Connection connect(EmitterObject&, Signal(Emitter::*)(Args...), Functor&&);
        friend CORRADE_INTERCONNECT_EXPORT bool disconnect(Emitter&, const Connection&);
        #endif

        /* Returns the actual location of the connection in the hashmap */
        Implementation::ConnectionData& connectInternal(const Implementation::SignalData& signal, Implementation::ConnectionData&& data);
        CORRADE_INTERCONNECT_LOCAL void disconnectFromReceiver(const Implementation::ConnectionData& data);

        void disconnectInternal(const Implementation::SignalData& signal);

        std::unordered_multimap<Implementation::SignalData, Implementation::ConnectionData, Implementation::SignalDataHash> _connections;
        std::uint32_t _lastHandledSignal;
        bool _connectionsChanged;
};

/** @relatesalso Emitter
@brief Connect signal to function slot
@param emitter       Emitter
@param signal        Signal
@param slot          Slot

Connects given signal to compatible slot. @p emitter must be subclass of
@ref Emitter, @p signal must be implemented signal and @p slot can be either
a non-member function, a lambda or any other function object. The argument
count and types must be exactly the same.

@attention Note that in case the lambda or function object references external
    data, you need to ensure the data are in scope for the whole lifetime of
    the connection. A much safer alternative is to make use of the
    @ref Receiver object --- in that case the connection is automatically
    removed when it goes out of scope.

See @ref Interconnect-Emitter-connections "Emitter class documentation" for
more information about connections.

@see @ref Emitter::hasSignalConnections(), @ref Connection::isConnected(),
     @ref Emitter::signalConnectionCount()
*/
template<class EmitterObject, class Emitter, class Functor, class ...Args> Connection connect(EmitterObject& emitter, Interconnect::Emitter::Signal(Emitter::*signal)(Args...), Functor&& slot) {
    static_assert(sizeof(Interconnect::Emitter::Signal(Emitter::*)(Args...)) <= sizeof(Implementation::SignalData),
        "size of member function pointer is incorrectly assumed to be smaller");
    static_assert(std::is_base_of<Emitter, EmitterObject>::value,
        "Emitter object doesn't have given signal");

    #ifndef CORRADE_MSVC2019_COMPATIBILITY
    Implementation::SignalData signalData(signal);
    #else
    auto signalData = Implementation::SignalData::create<Emitter, Args...>(signal);
    #endif
    return Connection{
        #ifdef CORRADE_BUILD_DEPRECATED
        emitter,
        #endif
        signalData, emitter.connectInternal(signalData, Implementation::ConnectionData::createFunctor<Args...>(std::move(slot)))};
}

/** @relatesalso Emitter
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
    static_assert(sizeof(Interconnect::Emitter::Signal(Emitter::*)(Args...)) <= sizeof(Implementation::SignalData),
        "Size of member function pointer is incorrectly assumed to be smaller");
    static_assert(std::is_base_of<Emitter, EmitterObject>::value,
        "Emitter object doesn't have given signal");
    static_assert(std::is_base_of<Receiver, ReceiverObject>::value,
        "Receiver object doesn't have given slot");

    #ifndef CORRADE_MSVC2019_COMPATIBILITY
    Implementation::SignalData signalData(signal);
    #else
    auto signalData = Implementation::SignalData::create<Emitter, Args...>(signal);
    #endif
    return Connection{
        #ifdef CORRADE_BUILD_DEPRECATED
        emitter,
        #endif
        signalData, emitter.connectInternal(signalData, Implementation::ConnectionData::createMember<Receiver, ReceiverObject, Args...>(receiver, slot))};
}

/** @relatesalso Emitter
@brief Disconnect a signal/slot connection
@param emitter      Emitter
@param connection   Connection handle returned by @ref connect()

It's the user responsibility to ensure that @p connection corresponds to given
@p emitter instance. See @ref Interconnect-Emitter-connections "Emitter class documentation"
for more information about connections.
*/
CORRADE_INTERCONNECT_EXPORT bool disconnect(Emitter& emitter, const Connection& connection);

#ifndef DOXYGEN_GENERATING_OUTPUT
template<class Emitter_, class ...Args> Emitter::Signal Emitter::emit(Signal(Emitter_::*signal)(Args...), typename Implementation::Identity<Args>::Type... args) {
    _connectionsChanged = false;
    ++_lastHandledSignal;
    auto range = _connections.equal_range(
        #ifndef CORRADE_MSVC2019_COMPATIBILITY
        Implementation::SignalData(signal)
        #else
        Implementation::SignalData::create<Emitter_, Args...>(signal)
        #endif
        );
    auto it = range.first;
    while(it != range.second) {
        /* Caching this actually helps *immensely* with debug runtime perf */
        Implementation::ConnectionData& data = it->second;

        /* If not already handled, proceed and mark as such */
        if(data.lastHandledSignal != _lastHandledSignal) {
            data.lastHandledSignal = _lastHandledSignal;

            reinterpret_cast<void(*)(Implementation::ConnectionData::Storage&, Args&&...)>(data.call)(data.storage, std::forward<Args>(args)...);

            /* Connections changed by the slot, go through again */
            if(_connectionsChanged) {
                range = _connections.equal_range(
                    #ifndef CORRADE_MSVC2019_COMPATIBILITY
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
