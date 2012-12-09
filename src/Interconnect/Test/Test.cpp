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

#include "Test.h"

#include "Interconnect/Emitter.h"
#include "Interconnect/Receiver.h"

CORRADE_TEST_MAIN(Corrade::Interconnect::Test::Test)

namespace Corrade { namespace Interconnect { namespace Test {

class Postman: public Interconnect::Emitter {
    public:
        inline Signal newMessage(int price, const std::string& message) {
            return emit(&Postman::newMessage, price, message);
        }

        inline Signal paymentRequested(int amount) {
            return emit(&Postman::paymentRequested, amount);
        }
};

class Mailbox: public Interconnect::Receiver {
    public:
        inline Mailbox(): money(0) {}

        inline void addMessage(int price, const std::string& message) {
            this->money += price;
            this->messages += message+'\n';
        }

        inline virtual void pay(int amount) {
            this->money -= amount;
        }

        int money;
        std::string messages;
};

Test::Test() {
    addTests(&Test::signalData,
             &Test::connect,
             &Test::disconnect,
             &Test::disconnectSignal,
             &Test::disconnectEmitter,
             &Test::disconnectReceiver,
             &Test::destroyEmitter,
             &Test::destroyReceiver,
             &Test::emit,
             &Test::emitterSubclass,
             &Test::virtualSlot,
             &Test::changeConnectionsInSlot,
             &Test::deleteReceiverInSlot);
}

void Test::signalData() {
    Implementation::SignalData data1(&Postman::newMessage);
    Implementation::SignalData data2(&Postman::newMessage);
    Implementation::SignalData data3(&Postman::paymentRequested);

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

void Test::connect() {
    Postman postman;
    Mailbox mailbox1, mailbox2;

    /* Verify returned connection */
    Connection connection = Emitter::connect(&postman, &Postman::newMessage, &mailbox1, &Mailbox::addMessage);
    CORRADE_VERIFY(connection.isConnectionPossible());
    CORRADE_VERIFY(connection.isConnected());

    /* Verify connection adding */
    Emitter::connect(&postman, &Postman::paymentRequested, &mailbox1, &Mailbox::pay);
    Emitter::connect(&postman, &Postman::newMessage, &mailbox2, &Mailbox::addMessage);
    CORRADE_VERIFY(postman.isConnected());
    CORRADE_COMPARE(postman.connectionCount(), 3);
    CORRADE_VERIFY(postman.isConnected(&Postman::newMessage));
    CORRADE_COMPARE(postman.connectionCount(&Postman::newMessage), 2);
    CORRADE_VERIFY(postman.isConnected(&Postman::paymentRequested));
    CORRADE_COMPARE(postman.connectionCount(&Postman::paymentRequested), 1);
    CORRADE_COMPARE(mailbox1.connectionCount(), 2);
    CORRADE_COMPARE(mailbox2.connectionCount(), 1);

    /* Allow multiple connections */
    Emitter::connect(&postman, &Postman::newMessage, &mailbox1, &Mailbox::addMessage);
    CORRADE_COMPARE(postman.connectionCount(), 4);
    CORRADE_COMPARE(postman.connectionCount(&Postman::newMessage), 3);
    CORRADE_COMPARE(mailbox1.connectionCount(), 3);
}

void Test::disconnect() {
    Postman postman;
    Mailbox mailbox1, mailbox2;

    Connection connection = Emitter::connect(&postman, &Postman::newMessage, &mailbox1, &Mailbox::addMessage);
    Emitter::connect(&postman, &Postman::paymentRequested, &mailbox1, &Mailbox::pay);
    Emitter::connect(&postman, &Postman::newMessage, &mailbox2, &Mailbox::addMessage);

    /* Verify disconnection response */
    connection.disconnect();
    CORRADE_VERIFY(connection.isConnectionPossible());
    CORRADE_VERIFY(!connection.isConnected());
    CORRADE_COMPARE(postman.connectionCount(&Postman::newMessage), 1);
    CORRADE_COMPARE(mailbox1.connectionCount(), 1);

    /* Verify connection response */
    connection.connect();
    CORRADE_VERIFY(connection.isConnectionPossible());
    CORRADE_VERIFY(connection.isConnected());
    CORRADE_COMPARE(postman.connectionCount(&Postman::newMessage), 2);
    CORRADE_COMPARE(mailbox1.connectionCount(), 2);
}

void Test::disconnectSignal() {
    Postman postman;
    Mailbox mailbox1, mailbox2;

    Connection c1 = Emitter::connect(&postman, &Postman::newMessage, &mailbox1, &Mailbox::addMessage);
    Connection c2 = Emitter::connect(&postman, &Postman::newMessage, &mailbox2, &Mailbox::addMessage);
    Connection c3 = Emitter::connect(&postman, &Postman::paymentRequested, &mailbox1, &Mailbox::pay);

    postman.disconnect(&Postman::newMessage);
    CORRADE_VERIFY(c1.isConnectionPossible());
    CORRADE_VERIFY(!c1.isConnected());
    CORRADE_VERIFY(c2.isConnectionPossible());
    CORRADE_VERIFY(!c2.isConnected());
    CORRADE_VERIFY(c3.isConnected());
    CORRADE_COMPARE(postman.connectionCount(), 1);
    CORRADE_VERIFY(!postman.isConnected(&Postman::newMessage));
    CORRADE_COMPARE(postman.connectionCount(&Postman::newMessage), 0);
    CORRADE_COMPARE(mailbox1.connectionCount(), 1);
    CORRADE_COMPARE(mailbox2.connectionCount(), 0);
}

void Test::disconnectEmitter() {
    Postman postman1, postman2;
    Mailbox mailbox;

    Connection c1 = Emitter::connect(&postman1, &Postman::newMessage, &mailbox, &Mailbox::addMessage);
    Connection c2 = Emitter::connect(&postman1, &Postman::paymentRequested, &mailbox, &Mailbox::pay);
    Connection c3 = Emitter::connect(&postman2, &Postman::newMessage, &mailbox, &Mailbox::addMessage);

    postman1.disconnect();
    CORRADE_VERIFY(c1.isConnectionPossible());
    CORRADE_VERIFY(!c1.isConnected());
    CORRADE_VERIFY(c2.isConnectionPossible());
    CORRADE_VERIFY(!c2.isConnected());
    CORRADE_VERIFY(c3.isConnected());
    CORRADE_VERIFY(!postman1.isConnected());
    CORRADE_COMPARE(postman1.connectionCount(), 0);
    CORRADE_VERIFY(postman2.isConnected());
    CORRADE_COMPARE(mailbox.connectionCount(), 1);
}

void Test::disconnectReceiver() {
    Postman postman;
    Mailbox mailbox1, mailbox2;

    Connection c1 = Emitter::connect(&postman, &Postman::newMessage, &mailbox1, &Mailbox::addMessage);
    Connection c2 = Emitter::connect(&postman, &Postman::paymentRequested, &mailbox1, &Mailbox::pay);
    Connection c3 = Emitter::connect(&postman, &Postman::newMessage, &mailbox2, &Mailbox::addMessage);

    mailbox1.disconnect();
    CORRADE_VERIFY(c1.isConnectionPossible());
    CORRADE_VERIFY(!c1.isConnected());
    CORRADE_VERIFY(c2.isConnectionPossible());
    CORRADE_VERIFY(!c2.isConnected());
    CORRADE_VERIFY(c3.isConnected());
    CORRADE_COMPARE(postman.connectionCount(), 1);
    CORRADE_VERIFY(!mailbox1.isConnected());
    CORRADE_COMPARE(mailbox2.connectionCount(), 1);
}

void Test::destroyEmitter() {
    Postman *postman1 = new Postman;
    Postman postman2;
    Mailbox mailbox;

    Connection c1 = Emitter::connect(postman1, &Postman::newMessage, &mailbox, &Mailbox::addMessage);
    Connection c2 = Emitter::connect(postman1, &Postman::paymentRequested, &mailbox, &Mailbox::pay);
    Connection c3 = Emitter::connect(&postman2, &Postman::newMessage, &mailbox, &Mailbox::addMessage);

    delete postman1;
    CORRADE_VERIFY(!c1.isConnectionPossible());
    CORRADE_VERIFY(!c1.isConnected());
    CORRADE_VERIFY(!c2.isConnectionPossible());
    CORRADE_VERIFY(!c2.isConnected());
    CORRADE_VERIFY(c3.isConnected());
    CORRADE_COMPARE(postman2.connectionCount(), 1);
    CORRADE_COMPARE(mailbox.connectionCount(), 1);
}

void Test::destroyReceiver() {
    Postman postman;
    Mailbox *mailbox1 = new Mailbox;
    Mailbox mailbox2;

    Connection c1 = Emitter::connect(&postman, &Postman::newMessage, mailbox1, &Mailbox::addMessage);
    Connection c2 = Emitter::connect(&postman, &Postman::paymentRequested, mailbox1, &Mailbox::pay);
    Connection c3 = Emitter::connect(&postman, &Postman::newMessage, &mailbox2, &Mailbox::addMessage);

    delete mailbox1;
    CORRADE_VERIFY(!c1.isConnectionPossible());
    CORRADE_VERIFY(!c1.isConnected());
    CORRADE_VERIFY(!c2.isConnectionPossible());
    CORRADE_VERIFY(!c2.isConnected());
    CORRADE_VERIFY(c3.isConnected());
    CORRADE_COMPARE(postman.connectionCount(), 1);
    CORRADE_COMPARE(mailbox2.connectionCount(), 1);
}

void Test::emit() {
    Postman postman;
    Mailbox mailbox1, mailbox2, mailbox3;
    Emitter::connect(&postman, &Postman::newMessage, &mailbox1, &Mailbox::addMessage);
    Emitter::connect(&postman, &Postman::newMessage, &mailbox2, &Mailbox::addMessage);
    Emitter::connect(&postman, &Postman::paymentRequested, &mailbox1, &Mailbox::pay);
    Emitter::connect(&postman, &Postman::paymentRequested, &mailbox2, &Mailbox::pay);
    Emitter::connect(&postman, &Postman::paymentRequested, &mailbox3, &Mailbox::pay);

    /* Verify signal handling */
    postman.newMessage(60, "hello");
    postman.paymentRequested(50);
    CORRADE_COMPARE(mailbox1.messages, "hello\n");
    CORRADE_COMPARE(mailbox1.money, 10);
    CORRADE_COMPARE(mailbox2.messages, "hello\n");
    CORRADE_COMPARE(mailbox2.money, 10);
    CORRADE_COMPARE(mailbox3.messages, "");
    CORRADE_COMPARE(mailbox3.money, -50);
}

void Test::emitterSubclass() {
    class BetterPostman: public Postman {
        public:
            inline Signal newRichTextMessage(int price, const std::string& value) {
                return emit(&BetterPostman::newRichTextMessage, price, "***"+value+"***");
            }
    };

    BetterPostman postman;
    Mailbox mailbox;

    /* Just test that this doesn't spit any compiler errors */
    Emitter::connect(&postman, &BetterPostman::newRichTextMessage, &mailbox, &Mailbox::addMessage);
    Emitter::connect(&postman, &BetterPostman::newMessage, &mailbox, &Mailbox::addMessage);

    /* Just to be sure */
    postman.newMessage(5, "hello");
    postman.newRichTextMessage(10, "ahoy");
    CORRADE_COMPARE(mailbox.messages, "hello\n***ahoy***\n");
    CORRADE_COMPARE(mailbox.money, 15);

    postman.disconnect(&BetterPostman::newMessage);
    CORRADE_VERIFY(postman.isConnected(&BetterPostman::newRichTextMessage));
    postman.disconnect(&BetterPostman::newRichTextMessage);
    CORRADE_VERIFY(!postman.isConnected());
}

void Test::virtualSlot() {
    class TaxDodgingMailbox: public Mailbox {
        public:
            void pay(int amount) override {
                this->money -= amount/5;
            }
    };

    Postman postman;
    Mailbox* mailbox = new TaxDodgingMailbox;

    /* It is important to connect to the original slot, not derived */
    Emitter::connect(&postman, &Postman::paymentRequested, mailbox, &Mailbox::pay);

    postman.paymentRequested(50);
    CORRADE_COMPARE(mailbox->money, -10);

    delete mailbox;
}

void Test::changeConnectionsInSlot() {
    Postman postman;
    Mailbox mailbox;

    class PropagatingMailbox: public Interconnect::Receiver {
        public:
            inline PropagatingMailbox(Postman* postman, Mailbox* mailbox): postman(postman), mailbox(mailbox) {}

            inline void addMessage(int, const std::string& message) {
                this->messages += message+'\n';
                Emitter::connect(postman, &Postman::newMessage, mailbox, &Mailbox::addMessage);
                Emitter::connect(postman, &Postman::paymentRequested, mailbox, &Mailbox::pay);
            }

            std::string messages;

        private:
            Postman* postman;
            Mailbox* mailbox;
    };

    PropagatingMailbox propagatingMailbox(&postman, &mailbox);
    Emitter::connect(&postman, &Postman::newMessage, &propagatingMailbox, &PropagatingMailbox::addMessage);

    /* Not connected to anything */
    postman.paymentRequested(50);
    CORRADE_COMPARE(mailbox.money, 0);

    /* Propagating mailbox connects the other mailbox, verify the proper slots
       are called proper times */
    postman.newMessage(19, "hello");
    CORRADE_COMPARE(propagatingMailbox.messages, "hello\n");
    CORRADE_COMPARE(mailbox.messages, "hello\n");
    CORRADE_COMPARE(mailbox.money, 19);
}

void Test::deleteReceiverInSlot() {
    class SuicideMailbox: public Interconnect::Receiver {
        public:
            inline void addMessage(int, const std::string&) {
                delete this;
            }
    };

    Postman postman;
    SuicideMailbox* mailbox1 = new SuicideMailbox;
    Mailbox mailbox2, mailbox3;

    Emitter::connect(&postman, &Postman::newMessage, mailbox1, &SuicideMailbox::addMessage);
    Emitter::connect(&postman, &Postman::newMessage, &mailbox2, &Mailbox::addMessage);
    Emitter::connect(&postman, &Postman::newMessage, &mailbox3, &Mailbox::addMessage);

    /* Verify that the message is propagated to all slots */
    CORRADE_COMPARE(postman.connectionCount(), 3);
    postman.newMessage(11, "hello");
    CORRADE_COMPARE(postman.connectionCount(), 2);
    CORRADE_COMPARE(mailbox2.messages, "hello\n");
    CORRADE_COMPARE(mailbox3.messages, "hello\n");
}

}}}
