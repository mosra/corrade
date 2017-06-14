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

#include <sstream>

#include "Corrade/TestSuite/Tester.h"
#include "Corrade/TestSuite/Compare/Container.h"
#include "Corrade/Interconnect/Emitter.h"
#include "Corrade/Interconnect/Receiver.h"

namespace Corrade { namespace Interconnect { namespace Test {

struct Test: TestSuite::Tester {
    explicit Test();

    void signalData();
    void templatedSignalData();

    void connect();
    void connectMoveConnection();

    void disconnect();
    void disconnectSignal();
    void disconnectEmitter();
    void disconnectReceiver();

    void destroyEmitter();
    void destroyReceiver();

    void emit();
    void emitterSubclass();
    void receiverSubclass();
    void slotInReceiverBase();
    void virtualSlot();
    void templatedSignal();

    void changeConnectionsInSlot();
    void deleteReceiverInSlot();

    void function();
};

class Postman: public Interconnect::Emitter {
    public:
        Signal newMessage(int price, const std::string& message) {
            return emit(&Postman::newMessage, price, message);
        }

        Signal paymentRequested(int amount) {
            return emit(&Postman::paymentRequested, amount);
        }
};

class TemplatedPostman: public Interconnect::Emitter {
    public:
        template<class T> Signal newMessage(int price, const std::string& message) {
            #ifdef _MSC_VER /* See _functionHash for explanation */
            _functionHash = sizeof(T);
            #endif
            return emit(&TemplatedPostman::newMessage<T>, price, message);
        }

        template<class T> Signal oldMessage(int price, const std::string& message) {
            #ifdef _MSC_VER /* See _functionHash for explanation */
            _functionHash = sizeof(T)*2;
            #endif
            return emit(&TemplatedPostman::oldMessage<T>, price, message);
        }

    private:
        #ifdef _MSC_VER
        /* MSVC has an optimization (/OPT:ICF) that merges functions with
           identical instructions. That would prevent template signals from
           working, thus we need to do some otherwise useless work to
           differentiate them. Ugly as hell but better than disabling the
           optimization globally. Details:
           http://blogs.msdn.com/b/oldnewthing/archive/2005/03/22/400373.aspx */
        int _functionHash;
        #endif
};

class Mailbox: public Interconnect::Receiver {
    public:
        Mailbox(): money(0) {}

        void addMessage(int price, const std::string& message) {
            money += price;
            messages.push_back(message);
        }

        void pay(int amount) {
            money -= amount;
        }

        int money;
        std::vector<std::string> messages;
};

Test::Test() {
    addTests({&Test::signalData,
              &Test::templatedSignalData,

              &Test::connect,
              &Test::connectMoveConnection,

              &Test::disconnect,
              &Test::disconnectSignal,
              &Test::disconnectEmitter,
              &Test::disconnectReceiver,

              &Test::destroyEmitter,
              &Test::destroyReceiver,

              &Test::emit,
              &Test::emitterSubclass,
              &Test::receiverSubclass,
              &Test::slotInReceiverBase,
              &Test::virtualSlot,
              &Test::templatedSignal,

              &Test::changeConnectionsInSlot,
              &Test::deleteReceiverInSlot,

              &Test::function});
}

void Test::signalData() {
    #ifndef CORRADE_MSVC2017_COMPATIBILITY
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

void Test::templatedSignalData()
{
    #ifndef CORRADE_MSVC2017_COMPATIBILITY
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

void Test::connect() {
    Postman postman;
    Mailbox mailbox1, mailbox2;

    /* Verify returned connection */
    Connection connection = Interconnect::connect(postman, &Postman::newMessage, mailbox1, &Mailbox::addMessage);
    CORRADE_VERIFY(connection.isConnectionPossible());
    CORRADE_VERIFY(connection.isConnected());

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

void Test::connectMoveConnection() {
    Postman postman;
    Mailbox mailbox1, mailbox2;

    Connection connection1 = Interconnect::connect(postman, &Postman::newMessage, mailbox1, &Mailbox::addMessage);
    Connection connection2 = Interconnect::connect(postman, &Postman::newMessage, mailbox2, &Mailbox::addMessage);
    connection2.disconnect();

    /* Move construction */
    CORRADE_VERIFY(connection1.isConnectionPossible());
    CORRADE_VERIFY(connection1.isConnected());
    Connection connection3{std::move(connection1)};
    CORRADE_VERIFY(!connection1.isConnectionPossible());
    CORRADE_VERIFY(connection3.isConnectionPossible());
    CORRADE_VERIFY(!connection1.isConnected());
    CORRADE_VERIFY(connection3.isConnected());

    /* Move assignment */
    CORRADE_VERIFY(connection3.isConnected());
    CORRADE_VERIFY(!connection2.isConnected());
    CORRADE_VERIFY(connection2.isConnectionPossible());
    CORRADE_VERIFY(connection3.isConnectionPossible());
    connection2 = std::move(connection3);
    CORRADE_VERIFY(connection2.isConnected());
    CORRADE_VERIFY(!connection3.isConnected());
    CORRADE_VERIFY(connection2.isConnectionPossible());
    CORRADE_VERIFY(connection3.isConnectionPossible());
}

void Test::disconnect() {
    Postman postman;
    Mailbox mailbox1, mailbox2;

    Connection connection = Interconnect::connect(postman, &Postman::newMessage, mailbox1, &Mailbox::addMessage);
    Interconnect::connect(postman, &Postman::paymentRequested, mailbox1, &Mailbox::pay);
    Interconnect::connect(postman, &Postman::newMessage, mailbox2, &Mailbox::addMessage);

    /* Verify disconnection response */
    connection.disconnect();
    CORRADE_VERIFY(connection.isConnectionPossible());
    CORRADE_VERIFY(!connection.isConnected());
    CORRADE_COMPARE(postman.signalConnectionCount(&Postman::newMessage), 1);
    CORRADE_COMPARE(mailbox1.slotConnectionCount(), 1);

    /* Verify connection response */
    connection.connect();
    CORRADE_VERIFY(connection.isConnectionPossible());
    CORRADE_VERIFY(connection.isConnected());
    CORRADE_COMPARE(postman.signalConnectionCount(&Postman::newMessage), 2);
    CORRADE_COMPARE(mailbox1.slotConnectionCount(), 2);
}

void Test::disconnectSignal() {
    Postman postman;
    Mailbox mailbox1, mailbox2;

    Connection c1 = Interconnect::connect(postman, &Postman::newMessage, mailbox1, &Mailbox::addMessage);
    Connection c2 = Interconnect::connect(postman, &Postman::newMessage, mailbox2, &Mailbox::addMessage);
    Connection c3 = Interconnect::connect(postman, &Postman::paymentRequested, mailbox1, &Mailbox::pay);

    postman.disconnectSignal(&Postman::newMessage);
    CORRADE_VERIFY(c1.isConnectionPossible());
    CORRADE_VERIFY(!c1.isConnected());
    CORRADE_VERIFY(c2.isConnectionPossible());
    CORRADE_VERIFY(!c2.isConnected());
    CORRADE_VERIFY(c3.isConnected());
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
    CORRADE_VERIFY(c1.isConnectionPossible());
    CORRADE_VERIFY(!c1.isConnected());
    CORRADE_VERIFY(c2.isConnectionPossible());
    CORRADE_VERIFY(!c2.isConnected());
    CORRADE_VERIFY(c3.isConnected());
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
    CORRADE_VERIFY(c1.isConnectionPossible());
    CORRADE_VERIFY(!c1.isConnected());
    CORRADE_VERIFY(c2.isConnectionPossible());
    CORRADE_VERIFY(!c2.isConnected());
    CORRADE_VERIFY(c3.isConnected());
    CORRADE_COMPARE(postman.signalConnectionCount(), 1);
    CORRADE_VERIFY(!mailbox1.hasSlotConnections());
    CORRADE_COMPARE(mailbox2.slotConnectionCount(), 1);
}

void Test::destroyEmitter() {
    Postman *postman1 = new Postman;
    Postman postman2;
    Mailbox mailbox;

    Connection c1 = Interconnect::connect(*postman1, &Postman::newMessage, mailbox, &Mailbox::addMessage);
    Connection c2 = Interconnect::connect(*postman1, &Postman::paymentRequested, mailbox, &Mailbox::pay);
    Connection c3 = Interconnect::connect(postman2, &Postman::newMessage, mailbox, &Mailbox::addMessage);

    delete postman1;
    CORRADE_VERIFY(!c1.isConnectionPossible());
    CORRADE_VERIFY(!c1.isConnected());
    CORRADE_VERIFY(!c2.isConnectionPossible());
    CORRADE_VERIFY(!c2.isConnected());
    CORRADE_VERIFY(c3.isConnected());
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
    CORRADE_VERIFY(!c1.isConnectionPossible());
    CORRADE_VERIFY(!c1.isConnected());
    CORRADE_VERIFY(!c2.isConnectionPossible());
    CORRADE_VERIFY(!c2.isConnected());
    CORRADE_VERIFY(c3.isConnected());
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
    CORRADE_COMPARE(mailbox1.messages, std::vector<std::string>{"hello"});
    CORRADE_COMPARE(mailbox1.money, 10);
    CORRADE_COMPARE(mailbox2.messages, std::vector<std::string>{"hello"});
    CORRADE_COMPARE(mailbox2.money, 10);
    CORRADE_COMPARE(mailbox3.messages, std::vector<std::string>());
    CORRADE_COMPARE(mailbox3.money, -50);
}

void Test::emitterSubclass() {
    class BetterPostman: public Postman {
        public:
            Signal newRichTextMessage(int price, const std::string& value) {
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
    CORRADE_COMPARE_AS(mailbox.messages, (std::vector<std::string>{"hello", "***ahoy***"}),
                       TestSuite::Compare::SortedContainer);
    CORRADE_COMPARE(mailbox.money, 15);

    postman.disconnectSignal(&BetterPostman::newMessage);
    CORRADE_VERIFY(postman.hasSignalConnections(&BetterPostman::newRichTextMessage));
    postman.disconnectSignal(&BetterPostman::newRichTextMessage);
    CORRADE_VERIFY(!postman.hasSignalConnections());
}

void Test::receiverSubclass() {
    class BlueMailbox: public Mailbox {
        public:
            void addBlueMessage(int price, const std::string& message) {
                money += price;
                messages.push_back("Blue " + message);
            }
    };

    Postman postman;
    BlueMailbox mailbox;

    /* Test that this doesn't spit any compiler errors */
    Interconnect::connect(postman, &Postman::newMessage, mailbox, &BlueMailbox::addMessage);
    Interconnect::connect(postman, &Postman::newMessage, mailbox, &BlueMailbox::addBlueMessage);

    /* Just to be sure */
    postman.newMessage(5, "hello");
    CORRADE_COMPARE_AS(mailbox.messages, (std::vector<std::string>{"Blue hello", "hello"}),
                       TestSuite::Compare::SortedContainer);
    CORRADE_COMPARE(mailbox.money, 10);
}

void Test::slotInReceiverBase() {
    class VintageMailbox {
        public:
            VintageMailbox(): money(0) {}

            void addMessage(int price, const std::string& message) {
                money += price;
                messages.push_back(message);
            }

            int money;
            std::vector<std::string> messages;
    };

    class ModernMailbox: public VintageMailbox, public Interconnect::Receiver {};

    Postman postman;
    ModernMailbox mailbox;

    /* Test that this doesn't spit any compiler errors */
    Interconnect::connect(postman, &Postman::newMessage, mailbox, &VintageMailbox::addMessage);

    /* Just to be sure */
    postman.newMessage(5, "hello");
    CORRADE_COMPARE(mailbox.messages, std::vector<std::string>{"hello"});
    CORRADE_COMPARE(mailbox.money, 5);
}

void Test::virtualSlot() {
    class VirtualMailbox: public Interconnect::Receiver {
        public:
            VirtualMailbox(): money(0) {}

            virtual ~VirtualMailbox() {}

            void addMessage(int price, const std::string& message) {
                money += price;
                messages.push_back(message);
            }

            virtual void pay(int amount) {
                money -= amount;
            }

            int money;
            std::vector<std::string> messages;
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
    CORRADE_COMPARE(intMailbox.messages, std::vector<std::string>{"integer"});
    CORRADE_COMPARE(stringMailbox.messages, std::vector<std::string>{"string"});
}

void Test::changeConnectionsInSlot() {
    Postman postman;
    Mailbox mailbox;

    class PropagatingMailbox: public Interconnect::Receiver {
        public:
            PropagatingMailbox(Postman* postman, Mailbox* mailbox): postman(postman), mailbox(mailbox) {}

            void addMessage(int, const std::string& message) {
                messages.push_back(message);
                Interconnect::connect(*postman, &Postman::newMessage, *mailbox, &Mailbox::addMessage);
                Interconnect::connect(*postman, &Postman::paymentRequested, *mailbox, &Mailbox::pay);
            }

            std::vector<std::string> messages;

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
    CORRADE_COMPARE(propagatingMailbox.messages, std::vector<std::string>{"hello"});
    CORRADE_COMPARE(mailbox.messages, std::vector<std::string>{"hello"});
    CORRADE_COMPARE(mailbox.money, 19);
}

void Test::deleteReceiverInSlot() {
    class SuicideMailbox: public Interconnect::Receiver {
        public:
            void addMessage(int, const std::string&) {
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
    CORRADE_COMPARE(mailbox2.messages, std::vector<std::string>{"hello"});
    CORRADE_COMPARE(mailbox3.messages, std::vector<std::string>{"hello"});
}

void Test::function() {
    std::ostringstream out;
    Debug redirectDebug{&out};

    Postman postman;
    Connection connection = Interconnect::connect(postman, &Postman::newMessage, [](int, const std::string& message) { Debug() << message; });

    postman.newMessage(0, "hello");
    CORRADE_COMPARE(out.str(), "hello\n");
    connection.disconnect();
    postman.newMessage(0, "heyy");
    CORRADE_COMPARE(out.str(), "hello\n");
}

}}}

CORRADE_TEST_MAIN(Corrade::Interconnect::Test::Test)
