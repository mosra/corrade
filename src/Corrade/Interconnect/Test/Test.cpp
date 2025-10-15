/*
    This file is part of Corrade.

    Copyright © 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016,
                2017, 2018, 2019, 2020, 2021, 2022, 2023, 2024, 2025
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

#include <functional>
#include <string>

#include "Corrade/Containers/GrowableArray.h"
#include "Corrade/Containers/String.h"
#include "Corrade/Containers/StringIterable.h"
#include "Corrade/Interconnect/Emitter.h"
#include "Corrade/Interconnect/Receiver.h"
#include "Corrade/TestSuite/Tester.h"
#include "Corrade/TestSuite/Compare/SortedContainer.h"
#include "Corrade/TestSuite/Compare/Numeric.h"

namespace Corrade { namespace Interconnect { namespace Test { namespace {

struct Test: TestSuite::Tester {
    explicit Test();

    void signalData();
    void templatedSignalData();

    void connectionDataFree();
    void connectionDataMember();
    void connectionDataLambda();
    void connectionDataLambdaDestructor();
    void connectionDataLambdaHeap();

    void connect();

    void disconnect();
    void disconnectSignal();
    void disconnectEmitter();
    void disconnectReceiver();

    void destroyEmitter();
    void destroyReceiver();

    void emit();
    void emitterSubclass();
    void emitterMultipleInheritance();
    void emitterMultipleInheritanceVirtual();
    void emitterIdenticalSignals();

    void receiverSubclass();
    void slotInReceiverBase();
    void virtualSlot();
    void templatedSignal();

    void changeConnectionsInSlot();
    void deleteReceiverInSlot();

    void function();
    void capturingLambda();
    void stdFunction();

    void nonCopyableParameter();
};

class Postman: public Interconnect::Emitter {
    public:
        Signal newMessage(int price, Containers::StringView message) {
            return emit(&Postman::newMessage, price, message);
        }

        Signal paymentRequested(int amount) {
            return emit(&Postman::paymentRequested, amount);
        }
};

class TemplatedPostman: public Interconnect::Emitter {
    public:
        template<class T> Signal newMessage(int price, Containers::StringView message) {
            #ifdef CORRADE_TARGET_MSVC /* See _functionHash for explanation */
            _functionHash = sizeof(T);
            #endif
            return emit(&TemplatedPostman::newMessage<T>, price, message);
        }

        template<class T> Signal oldMessage(int price, Containers::StringView message) {
            #ifdef CORRADE_TARGET_MSVC /* See _functionHash for explanation */
            _functionHash = sizeof(T)*2;
            #endif
            return emit(&TemplatedPostman::oldMessage<T>, price, message);
        }

    private:
        #ifdef CORRADE_TARGET_MSVC
        /* MSVC has an optimization (/OPT:ICF) that merges functions with
           identical instructions. That would prevent template signals from
           working, thus we need to do some otherwise useless work to
           differentiate them. Ugly as hell but better than disabling the
           optimization globally. Details:
           https://web.archive.org/web/20150703232520/http://blogs.msdn.com/b/oldnewthing/archive/2005/03/22/400373.aspx */
        int _functionHash;
        #endif
};

class Mailbox: public Interconnect::Receiver {
    public:
        Mailbox(): money(0) {}

        void addMessage(int price, Containers::StringView message) {
            money += price;
            arrayAppend(messages, message);
        }

        void pay(int amount) {
            money -= amount;
        }

        int money;
        /* Test::emitterSubclass(), emitterMultipleInheritance*(),
           receiverSubclass() create a string at runtime, so can't store just
           views */
        Containers::Array<Containers::String> messages;
};

Test::Test() {
    addTests({&Test::signalData,
              &Test::templatedSignalData,

              &Test::connectionDataFree,
              &Test::connectionDataMember,
              &Test::connectionDataLambda,
              &Test::connectionDataLambdaDestructor,
              &Test::connectionDataLambdaHeap,

              &Test::connect,

              &Test::disconnect,
              &Test::disconnectSignal,
              &Test::disconnectEmitter,
              &Test::disconnectReceiver,

              &Test::destroyEmitter,
              &Test::destroyReceiver,

              &Test::emit,
              &Test::emitterSubclass,
              &Test::emitterMultipleInheritance,
              &Test::emitterMultipleInheritanceVirtual,
              &Test::emitterIdenticalSignals,

              &Test::receiverSubclass,
              &Test::slotInReceiverBase,
              &Test::virtualSlot,
              &Test::templatedSignal,

              &Test::changeConnectionsInSlot,
              &Test::deleteReceiverInSlot,

              &Test::function,
              &Test::capturingLambda,
              &Test::stdFunction,

              &Test::nonCopyableParameter});
}

void Test::signalData() {
    /* Still broken even on MSVC 2022. Maybe 2025 will be the year when MSVC
       can finally do plain C++11? */
    #if !defined(CORRADE_TARGET_MSVC) || defined(CORRADE_TARGET_CLANG_CL) || _MSC_VER >= 1940
    Implementation::SignalData data1(&Postman::newMessage);
    Implementation::SignalData data2(&Postman::newMessage);
    Implementation::SignalData data3(&Postman::paymentRequested);
    #else
    auto data1 = Implementation::SignalData::create<Postman>(&Postman::newMessage);
    auto data2 = Implementation::SignalData::create<Postman>(&Postman::newMessage);
    auto data3 = Implementation::SignalData::create<Postman>(&Postman::paymentRequested);
    #endif

    CORRADE_VERIFY(data1 == data1);
    CORRADE_VERIFY(data2 == data2);
    CORRADE_VERIFY(data3 == data3);
    CORRADE_VERIFY(data1 == data2);
    CORRADE_VERIFY(data1 != data3);
    CORRADE_VERIFY(data2 != data3);

    CORRADE_VERIFY(Implementation::SignalDataHash()(data1) == Implementation::SignalDataHash()(data1));
    CORRADE_VERIFY(Implementation::SignalDataHash()(data1) == Implementation::SignalDataHash()(data2));
    CORRADE_VERIFY(Implementation::SignalDataHash()(data1) != Implementation::SignalDataHash()(data3));
}

void Test::templatedSignalData() {
    /* Still broken even on MSVC 2022. Maybe 2025 will be the year when MSVC
       can finally do plain C++11? */
    #if !defined(CORRADE_TARGET_MSVC) || defined(CORRADE_TARGET_CLANG_CL) || _MSC_VER >= 1940
    Implementation::SignalData data1(&TemplatedPostman::newMessage<std::int32_t>);
    Implementation::SignalData data2(&TemplatedPostman::newMessage<std::string>);
    Implementation::SignalData data3(&TemplatedPostman::oldMessage<std::int32_t>);
    #else
    auto data1 = Implementation::SignalData::create<TemplatedPostman>(&TemplatedPostman::newMessage<std::int32_t>);
    auto data2 = Implementation::SignalData::create<TemplatedPostman>(&TemplatedPostman::newMessage<std::string>);
    auto data3 = Implementation::SignalData::create<TemplatedPostman>(&TemplatedPostman::oldMessage<std::int32_t>);
    #endif

    CORRADE_VERIFY(data1 != data2);
    CORRADE_VERIFY(data1 != data3);
}

int globalCounter;

void incrementCounter() { ++globalCounter; }

void Test::connectionDataFree() {
    auto d = Implementation::ConnectionData::createFunctor(incrementCounter);
    CORRADE_VERIFY(d.type == Implementation::ConnectionType::Free);
    CORRADE_VERIFY(d.storage.function == &incrementCounter);
    CORRADE_VERIFY(d.call);

    globalCounter = 0;
    reinterpret_cast<void(*)(Implementation::ConnectionData::Storage&)>(d.call)(d.storage);
    CORRADE_COMPARE(globalCounter, 1);

    Implementation::ConnectionData d2{std::move(d)};
    CORRADE_VERIFY(d2.type == Implementation::ConnectionType::Free);
    CORRADE_VERIFY(d2.storage.function == &incrementCounter);
    CORRADE_VERIFY(d2.call);

    reinterpret_cast<void(*)(Implementation::ConnectionData::Storage&)>(d2.call)(d2.storage);
    CORRADE_COMPARE(globalCounter, 2);

    Implementation::ConnectionData d3{Implementation::ConnectionType::Member};
    d3 = std::move(d2);
    CORRADE_VERIFY(d3.type == Implementation::ConnectionType::Free);
    CORRADE_VERIFY(d3.storage.function == &incrementCounter);
    CORRADE_VERIFY(d3.call);

    reinterpret_cast<void(*)(Implementation::ConnectionData::Storage&)>(d3.call)(d3.storage);
    CORRADE_COMPARE(globalCounter, 3);
}

void Test::connectionDataMember() {
    struct R: Receiver {
        int output = 0;

        void receive() { ++output; }
    } receiver;

    auto d = Implementation::ConnectionData::createMember(receiver, &R::receive);
    CORRADE_VERIFY(d.type == Implementation::ConnectionType::Member);
    CORRADE_COMPARE(d.storage.member.receiver, &receiver);
    CORRADE_VERIFY(d.call);

    reinterpret_cast<void(*)(Implementation::ConnectionData::Storage&)>(d.call)(d.storage);
    CORRADE_COMPARE(receiver.output, 1);

    Implementation::ConnectionData d2{std::move(d)};
    CORRADE_VERIFY(d2.type == Implementation::ConnectionType::Member);
    CORRADE_COMPARE(d2.storage.member.receiver, &receiver);
    CORRADE_VERIFY(d2.call);

    reinterpret_cast<void(*)(Implementation::ConnectionData::Storage&)>(d2.call)(d2.storage);
    CORRADE_COMPARE(receiver.output, 2);

    Implementation::ConnectionData d3{Implementation::ConnectionType::Free};
    d3 = std::move(d2);
    CORRADE_VERIFY(d3.type == Implementation::ConnectionType::Member);
    CORRADE_COMPARE(d3.storage.member.receiver, &receiver);
    CORRADE_VERIFY(d3.call);

    reinterpret_cast<void(*)(Implementation::ConnectionData::Storage&)>(d3.call)(d3.storage);
    CORRADE_COMPARE(receiver.output, 3);
}

void Test::connectionDataLambda() {
    int counter = 0;

    /* Lambdas are not trivially copyable under MSVC, working around that with
       a handmade function object */
    #ifndef CORRADE_TARGET_MSVC
    auto d = Implementation::ConnectionData::createFunctor([&counter]() { ++counter; });
    #else
    struct Lambda {
        Lambda(int& counter): counter{&counter} {}
        void operator()() const { ++*counter; }
        int* counter;
    };
    static_assert(std::is_trivially_copyable<Lambda>::value,
        "everything is wrong, let's put the world on fire");
    auto d = Implementation::ConnectionData::createFunctor(Lambda{counter});
    #endif

    CORRADE_VERIFY(d.type == Implementation::ConnectionType::Functor);
    CORRADE_VERIFY(d.call);

    reinterpret_cast<void(*)(Implementation::ConnectionData::Storage&)>(d.call)(d.storage);
    CORRADE_COMPARE(counter, 1);

    Implementation::ConnectionData d2{std::move(d)};
    CORRADE_VERIFY(d2.type == Implementation::ConnectionType::Functor);
    CORRADE_VERIFY(d2.call);

    reinterpret_cast<void(*)(Implementation::ConnectionData::Storage&)>(d2.call)(d2.storage);
    CORRADE_COMPARE(counter, 2);

    Implementation::ConnectionData d3{Implementation::ConnectionType::Member};
    d3 = std::move(d2);
    CORRADE_VERIFY(d3.type == Implementation::ConnectionType::Functor);
    CORRADE_VERIFY(d3.call);

    reinterpret_cast<void(*)(Implementation::ConnectionData::Storage&)>(d3.call)(d3.storage);
    CORRADE_COMPARE(counter, 3);
}

void Test::connectionDataLambdaDestructor() {
    struct Destructor {
        int value = 3;
        ~Destructor() { globalCounter += 7; }
    } a;

    {
        auto d = Implementation::ConnectionData::createFunctor([a](){ globalCounter += a.value; });
        CORRADE_VERIFY(d.type == Implementation::ConnectionType::FunctorWithDestructor);
        CORRADE_VERIFY(d.storage.functor.destruct);
        CORRADE_VERIFY(d.call);

        globalCounter = 0;
        reinterpret_cast<void(*)(Implementation::ConnectionData::Storage&)>(d.call)(d.storage);
        CORRADE_COMPARE(globalCounter, 3);

        Implementation::ConnectionData d2{std::move(d)};
        CORRADE_VERIFY(d.type == Implementation::ConnectionType::Functor);
        CORRADE_VERIFY(d2.type == Implementation::ConnectionType::FunctorWithDestructor);
        CORRADE_VERIFY(d2.storage.functor.destruct);
        CORRADE_VERIFY(d2.call);

        reinterpret_cast<void(*)(Implementation::ConnectionData::Storage&)>(d2.call)(d2.storage);
        CORRADE_COMPARE(globalCounter, 6);

        Implementation::ConnectionData d3{Implementation::ConnectionType::Member};
        d3 = std::move(d2);
        CORRADE_VERIFY(d2.type == Implementation::ConnectionType::Member);
        CORRADE_VERIFY(d3.type == Implementation::ConnectionType::FunctorWithDestructor);
        CORRADE_VERIFY(d3.storage.functor.destruct);
        CORRADE_VERIFY(d3.call);

        reinterpret_cast<void(*)(Implementation::ConnectionData::Storage&)>(d3.call)(d3.storage);
        CORRADE_COMPARE(globalCounter, 9);
    }

    CORRADE_COMPARE(globalCounter, 16);
}

void Test::connectionDataLambdaHeap() {
    int counter = 0;

    std::function<void()> f{[&counter](){ ++counter; }};
    CORRADE_COMPARE_AS(sizeof(f), sizeof(Implementation::ConnectionData::Storage),
        TestSuite::Compare::Greater);

    auto d = Implementation::ConnectionData::createFunctor(f);
    CORRADE_VERIFY(d.type == Implementation::ConnectionType::FunctorWithDestructor);
    CORRADE_VERIFY(d.storage.functor.destruct);
    CORRADE_VERIFY(d.call);

    reinterpret_cast<void(*)(Implementation::ConnectionData::Storage&)>(d.call)(d.storage);
    CORRADE_COMPARE(counter, 1);

    Implementation::ConnectionData d2{std::move(d)};
    CORRADE_VERIFY(d.type == Implementation::ConnectionType::Functor);
    CORRADE_VERIFY(d2.type == Implementation::ConnectionType::FunctorWithDestructor);
    CORRADE_VERIFY(d2.storage.functor.destruct);
    CORRADE_VERIFY(d2.call);

    reinterpret_cast<void(*)(Implementation::ConnectionData::Storage&)>(d2.call)(d2.storage);
    CORRADE_COMPARE(counter, 2);

    Implementation::ConnectionData d3{Implementation::ConnectionType::Member};
    d3 = std::move(d2);
    CORRADE_VERIFY(d2.type == Implementation::ConnectionType::Member);
    CORRADE_VERIFY(d3.type == Implementation::ConnectionType::FunctorWithDestructor);
    CORRADE_VERIFY(d3.storage.functor.destruct);
    CORRADE_VERIFY(d3.call);

    reinterpret_cast<void(*)(Implementation::ConnectionData::Storage&)>(d3.call)(d3.storage);
    CORRADE_COMPARE(counter, 3);
}

void Test::connect() {
    Postman postman;
    Mailbox mailbox1, mailbox2;

    /* Verify returned connection */
    Connection connection = Interconnect::connect(postman, &Postman::newMessage, mailbox1, &Mailbox::addMessage);
    CORRADE_VERIFY(postman.isConnected(connection));

    /* Verify connection adding */
    Interconnect::connect(postman, &Postman::paymentRequested, mailbox1, &Mailbox::pay);
    Interconnect::connect(postman, &Postman::newMessage, mailbox2, &Mailbox::addMessage);
    CORRADE_VERIFY(postman.hasSignalConnections());
    CORRADE_COMPARE(postman.signalConnectionCount(), 3);
    CORRADE_VERIFY(postman.hasSignalConnections(&Postman::newMessage));
    CORRADE_COMPARE(postman.signalConnectionCount(&Postman::newMessage), 2);
    CORRADE_VERIFY(postman.hasSignalConnections(&Postman::paymentRequested));
    CORRADE_COMPARE(postman.signalConnectionCount(&Postman::paymentRequested), 1);
    CORRADE_COMPARE(mailbox1.slotConnectionCount(), 2);
    CORRADE_COMPARE(mailbox2.slotConnectionCount(), 1);

    /* Allow multiple connections */
    Interconnect::connect(postman, &Postman::newMessage, mailbox1, &Mailbox::addMessage);
    CORRADE_COMPARE(postman.signalConnectionCount(), 4);
    CORRADE_COMPARE(postman.signalConnectionCount(&Postman::newMessage), 3);
    CORRADE_COMPARE(mailbox1.slotConnectionCount(), 3);
}

void Test::disconnect() {
    Postman postman;
    Mailbox mailbox1, mailbox2;

    Connection connection = Interconnect::connect(postman, &Postman::newMessage, mailbox1, &Mailbox::addMessage);
    Interconnect::connect(postman, &Postman::paymentRequested, mailbox1, &Mailbox::pay);
    Interconnect::connect(postman, &Postman::newMessage, mailbox2, &Mailbox::addMessage);

    /* Verify disconnection response */
    CORRADE_VERIFY(Interconnect::disconnect(postman, connection));
    CORRADE_VERIFY(!postman.isConnected(connection));
    CORRADE_COMPARE(postman.signalConnectionCount(&Postman::newMessage), 1);
    CORRADE_COMPARE(mailbox1.slotConnectionCount(), 1);

    /* Disconnecting the second time fails */
    CORRADE_VERIFY(!Interconnect::disconnect(postman, connection));
}

void Test::disconnectSignal() {
    Postman postman;
    Mailbox mailbox1, mailbox2;

    Connection c1 = Interconnect::connect(postman, &Postman::newMessage, mailbox1, &Mailbox::addMessage);
    Connection c2 = Interconnect::connect(postman, &Postman::newMessage, mailbox2, &Mailbox::addMessage);
    Connection c3 = Interconnect::connect(postman, &Postman::paymentRequested, mailbox1, &Mailbox::pay);

    postman.disconnectSignal(&Postman::newMessage);
    CORRADE_VERIFY(!postman.isConnected(c1));
    CORRADE_VERIFY(!postman.isConnected(c2));
    CORRADE_VERIFY(postman.isConnected(c3));
    CORRADE_COMPARE(postman.signalConnectionCount(), 1);
    CORRADE_VERIFY(!postman.hasSignalConnections(&Postman::newMessage));
    CORRADE_COMPARE(postman.signalConnectionCount(&Postman::newMessage), 0);
    CORRADE_COMPARE(mailbox1.slotConnectionCount(), 1);
    CORRADE_COMPARE(mailbox2.slotConnectionCount(), 0);
}

void Test::disconnectEmitter() {
    Postman postman1, postman2;
    Mailbox mailbox;

    Connection c1 = Interconnect::connect(postman1, &Postman::newMessage, mailbox, &Mailbox::addMessage);
    Connection c2 = Interconnect::connect(postman1, &Postman::paymentRequested, mailbox, &Mailbox::pay);
    Connection c3 = Interconnect::connect(postman2, &Postman::newMessage, mailbox, &Mailbox::addMessage);

    postman1.disconnectAllSignals();
    CORRADE_VERIFY(!postman1.isConnected(c1));
    CORRADE_VERIFY(!postman1.isConnected(c2));
    CORRADE_VERIFY(postman2.isConnected(c3));
    CORRADE_VERIFY(!postman1.hasSignalConnections());
    CORRADE_COMPARE(postman1.signalConnectionCount(), 0);
    CORRADE_VERIFY(postman2.hasSignalConnections());
    CORRADE_COMPARE(mailbox.slotConnectionCount(), 1);
}

void Test::disconnectReceiver() {
    Postman postman;
    Mailbox mailbox1, mailbox2;

    Connection c1 = Interconnect::connect(postman, &Postman::newMessage, mailbox1, &Mailbox::addMessage);
    Connection c2 = Interconnect::connect(postman, &Postman::paymentRequested, mailbox1, &Mailbox::pay);
    Connection c3 = Interconnect::connect(postman, &Postman::newMessage, mailbox2, &Mailbox::addMessage);

    mailbox1.disconnectAllSlots();
    CORRADE_VERIFY(!postman.isConnected(c1));
    CORRADE_VERIFY(!postman.isConnected(c2));
    CORRADE_VERIFY(postman.isConnected(c3));
    CORRADE_COMPARE(postman.signalConnectionCount(), 1);
    CORRADE_VERIFY(!mailbox1.hasSlotConnections());
    CORRADE_COMPARE(mailbox2.slotConnectionCount(), 1);
}

void Test::destroyEmitter() {
    Postman *postman1 = new Postman;
    Postman postman2;
    Mailbox mailbox;

    Interconnect::connect(*postman1, &Postman::newMessage, mailbox, &Mailbox::addMessage);
    Interconnect::connect(*postman1, &Postman::paymentRequested, mailbox, &Mailbox::pay);
    Connection c3 = Interconnect::connect(postman2, &Postman::newMessage, mailbox, &Mailbox::addMessage);

    CORRADE_COMPARE(postman2.signalConnectionCount(), 1);
    CORRADE_COMPARE(mailbox.slotConnectionCount(), 3);

    delete postman1;
    CORRADE_VERIFY(postman2.isConnected(c3));
    CORRADE_COMPARE(postman2.signalConnectionCount(), 1);
    CORRADE_COMPARE(mailbox.slotConnectionCount(), 1);
}

void Test::destroyReceiver() {
    Postman postman;
    Mailbox *mailbox1 = new Mailbox;
    Mailbox mailbox2;

    Connection c1 = Interconnect::connect(postman, &Postman::newMessage, *mailbox1, &Mailbox::addMessage);
    Connection c2 = Interconnect::connect(postman, &Postman::paymentRequested, *mailbox1, &Mailbox::pay);
    Connection c3 = Interconnect::connect(postman, &Postman::newMessage, mailbox2, &Mailbox::addMessage);

    delete mailbox1;
    CORRADE_VERIFY(!postman.isConnected(c1));
    CORRADE_VERIFY(!postman.isConnected(c2));
    CORRADE_VERIFY(postman.isConnected(c3));
    CORRADE_COMPARE(postman.signalConnectionCount(), 1);
    CORRADE_COMPARE(mailbox2.slotConnectionCount(), 1);
}

void Test::emit() {
    Postman postman;
    Mailbox mailbox1, mailbox2, mailbox3;
    Interconnect::connect(postman, &Postman::newMessage, mailbox1, &Mailbox::addMessage);
    Interconnect::connect(postman, &Postman::newMessage, mailbox2, &Mailbox::addMessage);
    Interconnect::connect(postman, &Postman::paymentRequested, mailbox1, &Mailbox::pay);
    Interconnect::connect(postman, &Postman::paymentRequested, mailbox2, &Mailbox::pay);
    Interconnect::connect(postman, &Postman::paymentRequested, mailbox3, &Mailbox::pay);

    /* Verify signal handling */
    postman.newMessage(60, "hello");
    postman.paymentRequested(50);
    CORRADE_COMPARE_AS(mailbox1.messages, Containers::StringIterable{
        "hello"
    }, TestSuite::Compare::Container);
    CORRADE_COMPARE(mailbox1.money, 10);
    CORRADE_COMPARE_AS(mailbox2.messages, Containers::StringIterable{
        "hello"
    }, TestSuite::Compare::Container);
    CORRADE_COMPARE(mailbox2.money, 10);
    CORRADE_COMPARE_AS(mailbox3.messages, Containers::StringIterable{
    }, TestSuite::Compare::Container);
    CORRADE_COMPARE(mailbox3.money, -50);
}

void Test::emitterSubclass() {
    class BetterPostman: public Postman {
        public:
            Signal newRichTextMessage(int price, Containers::StringView value) {
                return emit(&BetterPostman::newRichTextMessage, price, "***"+value+"***");
            }
    };

    BetterPostman postman;
    Mailbox mailbox;

    /* Test that this doesn't spit any compiler errors */
    Interconnect::connect(postman, &BetterPostman::newRichTextMessage, mailbox, &Mailbox::addMessage);
    Interconnect::connect(postman, &BetterPostman::newMessage, mailbox, &Mailbox::addMessage);

    /* Just to be sure */
    postman.newMessage(5, "hello");
    postman.newRichTextMessage(10, "ahoy");
    CORRADE_COMPARE_AS(mailbox.messages, (Containers::StringIterable{
        "hello", "***ahoy***"
    }), TestSuite::Compare::SortedContainer);
    CORRADE_COMPARE(mailbox.money, 15);

    postman.disconnectSignal(&BetterPostman::newMessage);
    CORRADE_VERIFY(postman.hasSignalConnections(&BetterPostman::newRichTextMessage));
    postman.disconnectSignal(&BetterPostman::newRichTextMessage);
    CORRADE_VERIFY(!postman.hasSignalConnections());
}

void Test::emitterMultipleInheritance() {
    struct A {
        int foo;
    };

    struct Diamond: A, Postman {
        Signal newDiamondCladMessage(int price, Containers::StringView value) {
            return emit(&Diamond::newDiamondCladMessage, price, "<>"+value+"<>");
        }
    };

    Diamond postman;
    Mailbox mailbox;

    Interconnect::connect(postman, &Diamond::newDiamondCladMessage, mailbox, &Mailbox::addMessage);
    Interconnect::connect(postman, &Diamond::newMessage, mailbox, &Mailbox::addMessage);

    postman.newDiamondCladMessage(10, "ahoy");
    postman.newMessage(5, "hello");

    {
        #if defined(CORRADE_TARGET_MSVC) && !defined(CORRADE_TARGET_CLANG_CL) && _MSC_VER >= 1930
        /* This worked back when it was a `const std::string&`. I suppose with
           a string view that's copied by value and/or a significantly smaller
           generated code for the string concatenation it somehow fell under a
           threshold for some new optimization to kick in, making the connnect
           and emit each operate with an entirely different function. Or
           something. Given all other pain with these, I think the library is
           unsalvageable in its current form. */
        CORRADE_EXPECT_FAIL("MSVC 2022 doesn't correctly emit the signal with non-virtual multiple inheritance.");
        #endif
        CORRADE_COMPARE_AS(mailbox.messages, (Containers::StringIterable{
            "hello", "<>ahoy<>"
        }), TestSuite::Compare::SortedContainer);
        CORRADE_COMPARE(mailbox.money, 15);
    }

    CORRADE_VERIFY(postman.hasSignalConnections(&Diamond::newMessage));
    postman.disconnectSignal(&Diamond::newMessage);
    /* But then this says true on MSVC 2022?! What's going on?? */
    CORRADE_VERIFY(postman.hasSignalConnections(&Diamond::newDiamondCladMessage));
    postman.disconnectSignal(&Diamond::newDiamondCladMessage);
    CORRADE_VERIFY(!postman.hasSignalConnections());
}

void Test::emitterMultipleInheritanceVirtual() {
    /* Same as above, but with A derived virtually */

    struct A {
        int foo;
    };

    struct Diamond: virtual A, Postman {
        Signal newDiamondCladMessage(int price, Containers::StringView value) {
            return emit(&Diamond::newDiamondCladMessage, price, "<>"+value+"<>");
        }
    };

    Diamond postman;
    Mailbox mailbox;

    /* Virtual bases have extra big pointer sizes on MSVC (16 bytes on 32bit).
       Ensure this is handled correctly. */
    Interconnect::connect(postman, &Diamond::newDiamondCladMessage, mailbox, &Mailbox::addMessage);
    Interconnect::connect(postman, &Diamond::newDiamondCladMessage, [](int, Containers::StringView){});
    Interconnect::connect(postman, &Diamond::newMessage, mailbox, &Mailbox::addMessage);

    postman.newDiamondCladMessage(10, "ahoy");
    postman.newMessage(5, "hello");
    CORRADE_COMPARE_AS(mailbox.messages, (Containers::StringIterable{
        "hello", "<>ahoy<>"
    }), TestSuite::Compare::SortedContainer);
    CORRADE_COMPARE(mailbox.money, 15);

    CORRADE_VERIFY(postman.hasSignalConnections(&Diamond::newMessage));
    postman.disconnectSignal(&Diamond::newMessage);
    CORRADE_VERIFY(postman.hasSignalConnections(&Diamond::newDiamondCladMessage));
    postman.disconnectSignal(&Diamond::newDiamondCladMessage);
    CORRADE_VERIFY(!postman.hasSignalConnections());
}

void Test::emitterIdenticalSignals() {
    /* This is mainly to verify that identical looking functions are not merged
       under MSVC (the /OPT:ICF linker flag) */

    struct Widget: Emitter {
        Signal tapped() {
            return emit(&Widget::tapped);
        }

        Signal pressed() {
            return emit(&Widget::pressed);
        }

        Signal released() {
            return emit(&Widget::released);
        }
    };

    Widget a;
    Widget b;

    Interconnect::connect(a, &Widget::pressed, [](){ Debug{} << "a pressed!"; });
    Interconnect::connect(a, &Widget::released, [](){ Debug{} << "a released!"; });
    Interconnect::connect(b, &Widget::tapped, [](){ Debug{} << "b tapped!"; });

    Containers::String out;
    Debug redirectOutput{&out};
    a.pressed();
    a.released();
    a.tapped();

    b.pressed();
    b.released();
    b.tapped();

    CORRADE_VERIFY(&Widget::tapped != &Widget::pressed);
    CORRADE_VERIFY(&Widget::tapped != &Widget::released);

    CORRADE_COMPARE(out,
        "a pressed!\n"
        "a released!\n"
        "b tapped!\n");
}

void Test::receiverSubclass() {
    class BlueMailbox: public Mailbox {
        public:
            void addBlueMessage(int price, Containers::StringView message) {
                money += price;
                arrayAppend(messages, "Blue " + message);
            }
    };

    Postman postman;
    BlueMailbox mailbox;

    /* Test that this doesn't spit any compiler errors */
    Interconnect::connect(postman, &Postman::newMessage, mailbox, &BlueMailbox::addMessage);
    Interconnect::connect(postman, &Postman::newMessage, mailbox, &BlueMailbox::addBlueMessage);

    /* Just to be sure */
    postman.newMessage(5, "hello");
    CORRADE_COMPARE_AS(mailbox.messages, (Containers::StringIterable{
        "Blue hello", "hello"
    }), TestSuite::Compare::SortedContainer);
    CORRADE_COMPARE(mailbox.money, 10);
}

void Test::slotInReceiverBase() {
    class VintageMailbox {
        public:
            VintageMailbox(): money(0) {}

            void addMessage(int price, Containers::StringView message) {
                money += price;
                arrayAppend(messages, message);
            }

            int money;
            Containers::Array<Containers::StringView> messages;
    };

    class ModernMailbox: public VintageMailbox, public Interconnect::Receiver {};

    Postman postman;
    ModernMailbox mailbox;

    /* Test that this doesn't spit any compiler errors */
    Interconnect::connect(postman, &Postman::newMessage, mailbox, &VintageMailbox::addMessage);

    /* Just to be sure */
    postman.newMessage(5, "hello");
    CORRADE_COMPARE_AS(mailbox.messages, Containers::StringIterable{
        "hello"
    }, TestSuite::Compare::Container);
    CORRADE_COMPARE(mailbox.money, 5);
}

void Test::virtualSlot() {
    class VirtualMailbox: public Interconnect::Receiver {
        public:
            VirtualMailbox(): money(0) {}

            virtual ~VirtualMailbox() {}

            virtual void pay(int amount) {
                money -= amount;
            }

            int money;
    };

    class TaxDodgingMailbox: public VirtualMailbox {
        public:
            void pay(int amount) override {
                money -= amount/5;
            }
    };

    Postman postman;
    VirtualMailbox* mailbox = new TaxDodgingMailbox;

    /* It is important to connect to the original slot, not derived */
    Interconnect::connect(postman, &Postman::paymentRequested, *mailbox, &VirtualMailbox::pay);

    postman.paymentRequested(50);
    CORRADE_COMPARE(mailbox->money, -10);

    delete mailbox;
}

void Test::templatedSignal() {
    TemplatedPostman postman;
    Mailbox intMailbox, stringMailbox;

    /* Connect different types to slots in different objects */
    Interconnect::connect(postman, &TemplatedPostman::newMessage<std::int32_t>, intMailbox, &Mailbox::addMessage);
    Interconnect::connect(postman, &TemplatedPostman::newMessage<std::string>, stringMailbox, &Mailbox::addMessage);

    postman.newMessage<std::int32_t>(0, "integer");
    postman.newMessage<std::string>(0, "string");
    CORRADE_COMPARE_AS(intMailbox.messages, Containers::StringIterable{
        "integer"
    }, TestSuite::Compare::Container);
    CORRADE_COMPARE_AS(stringMailbox.messages, Containers::StringIterable{
        "string"
    }, TestSuite::Compare::Container);
}

void Test::changeConnectionsInSlot() {
    Postman postman;
    Mailbox mailbox;

    class PropagatingMailbox: public Interconnect::Receiver {
        public:
            PropagatingMailbox(Postman* postman, Mailbox* mailbox): postman(postman), mailbox(mailbox) {}

            void addMessage(int, Containers::StringView message) {
                arrayAppend(messages, message);
                Interconnect::connect(*postman, &Postman::newMessage, *mailbox, &Mailbox::addMessage);
                Interconnect::connect(*postman, &Postman::paymentRequested, *mailbox, &Mailbox::pay);
            }

            Containers::Array<Containers::StringView> messages;

        private:
            Postman* postman;
            Mailbox* mailbox;
    };

    PropagatingMailbox propagatingMailbox(&postman, &mailbox);
    Interconnect::connect(postman, &Postman::newMessage, propagatingMailbox, &PropagatingMailbox::addMessage);

    /* Not connected to anything */
    postman.paymentRequested(50);
    CORRADE_COMPARE(mailbox.money, 0);

    /* Propagating mailbox connects the other mailbox, verify the proper slots
       are called proper times */
    postman.newMessage(19, "hello");
    CORRADE_COMPARE_AS(propagatingMailbox.messages, Containers::StringIterable{
        "hello"
    }, TestSuite::Compare::Container);
    CORRADE_COMPARE_AS(mailbox.messages, Containers::StringIterable{
        "hello"
    }, TestSuite::Compare::Container);
    CORRADE_COMPARE(mailbox.money, 19);
}

void Test::deleteReceiverInSlot() {
    class SuicideMailbox: public Interconnect::Receiver {
        public:
            void addMessage(int, Containers::StringView) {
                delete this;
            }
    };

    Postman postman;
    SuicideMailbox* mailbox1 = new SuicideMailbox;
    Mailbox mailbox2, mailbox3;

    Interconnect::connect(postman, &Postman::newMessage, *mailbox1, &SuicideMailbox::addMessage);
    Interconnect::connect(postman, &Postman::newMessage, mailbox2, &Mailbox::addMessage);
    Interconnect::connect(postman, &Postman::newMessage, mailbox3, &Mailbox::addMessage);

    /* Verify that the message is propagated to all slots */
    CORRADE_COMPARE(postman.signalConnectionCount(), 3);
    postman.newMessage(11, "hello");
    CORRADE_COMPARE(postman.signalConnectionCount(), 2);
    CORRADE_COMPARE_AS(mailbox2.messages, Containers::StringIterable{
        "hello"
    }, TestSuite::Compare::Container);
    CORRADE_COMPARE_AS(mailbox3.messages, Containers::StringIterable{
        "hello"
    }, TestSuite::Compare::Container);
}

void Test::function() {
    Containers::String out;
    Debug redirectDebug{&out};

    Postman postman;
    Connection connection = Interconnect::connect(postman, &Postman::newMessage, [](int, Containers::StringView message) { Debug() << message; });

    postman.newMessage(0, "hello");
    CORRADE_COMPARE(out, "hello\n");
    Interconnect::disconnect(postman, connection);
    postman.newMessage(0, "heyy");
    CORRADE_COMPARE(out, "hello\n");
}

void Test::capturingLambda() {
    Containers::String out;

    Postman postman;
    Connection connection = Interconnect::connect(postman, &Postman::newMessage, [&out](int, Containers::StringView message) { Debug{&out} << message; });

    postman.newMessage(0, "hello");
    CORRADE_COMPARE(out, "hello\n");
    Interconnect::disconnect(postman, connection);
    postman.newMessage(0, "heyy");
    CORRADE_COMPARE(out, "hello\n");
}

void Test::stdFunction() {
    Containers::String out;
    std::function<void(int, Containers::StringView)> f{[&out](int, Containers::StringView message) { Debug{&out} << message; }};

    Postman postman;
    Connection connection = Interconnect::connect(postman, &Postman::newMessage, f);

    postman.newMessage(0, "hello");
    CORRADE_COMPARE(out, "hello\n");
    Interconnect::disconnect(postman, connection);
    postman.newMessage(0, "heyy");
    CORRADE_COMPARE(out, "hello\n");
}

void Test::nonCopyableParameter() {
    struct NonCopyable {
        explicit NonCopyable(int a): a{a} {}

        NonCopyable(const NonCopyable&) = delete;
        NonCopyable& operator=(const NonCopyable&) = delete;

        int a;
    };

    struct E: Emitter {
        Signal send(const NonCopyable& a) {
            return emit(&E::send, a);
        }
    } emitter;

    struct R: Receiver {
        void receive(const NonCopyable& a) {
            received += a.a;
        }

        int received{};
    } receiver;

    Interconnect::connect(emitter, &E::send, receiver, &R::receive);
    NonCopyable a{42};
    emitter.send(a);
    CORRADE_COMPARE(receiver.received, 42);
}

}}}}

CORRADE_TEST_MAIN(Corrade::Interconnect::Test::Test)
