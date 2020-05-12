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

#include <string>

#include "Corrade/Interconnect/Emitter.h"
#include "Corrade/Interconnect/Receiver.h"
#include "Corrade/Interconnect/StateMachine.h"

using namespace Corrade;

int main() {

{
/* [Emitter-signals] */
class Postman: public Interconnect::Emitter {
    public:
        Signal messageDelivered(const std::string& message, int price = 0) {
            return emit(&Postman::messageDelivered, message, price);
        }

        Signal paymentRequired(int amount) {
            return emit(&Postman::paymentRequired, amount);
        }
};
/* [Emitter-signals] */

{
/* [Emitter-emit] */
Postman postman;
postman.messageDelivered("hello");
postman.paymentRequired(245);
/* [Emitter-emit] */

/* [Emitter-connect] */
Interconnect::Connection c = Interconnect::connect(
    postman, &Postman::paymentRequired,
    [](int amount) { Utility::Debug{} << "pay" << amount; });

// ...

Interconnect::disconnect(postman, c);
/* [Emitter-connect] */
}

#if defined(__GNUC__) || defined( __clang__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#elif defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable: 4100)
#endif
{
/* [Emitter-connect-member-slot] */
class Mailbox: public Interconnect::Receiver {
    public:
        void addMessage(const std::string& message, int price) {}
};

Postman postman;
Mailbox mailbox;
Interconnect::connect(postman, &Postman::messageDelivered,
                      mailbox, &Mailbox::addMessage);
/* [Emitter-connect-member-slot] */
}
#if defined(__GNUC__) || defined( __clang__)
#pragma GCC diagnostic pop
#elif defined(_MSC_VER)
#pragma warning(pop)
#endif

{
/* [Emitter-disconnectSignal] */
Postman postman;
postman.disconnectSignal(&Postman::messageDelivered);
/* [Emitter-disconnectSignal] */
}
}

{
/* [Emitter-connect-emitter-type] */
class Base: public Interconnect::Emitter {
    public:
        Signal baseSignal() { return emit(&Base::baseSignal); }
};

class Derived: public Base {
    public:
        Signal derivedSignal() { return emit(&Derived::derivedSignal); }
};

Base* a = new Derived;
Derived* b = new Derived;
Interconnect::connect(*a, &Base::baseSignal, [](){});           // ok
Interconnect::connect(*b, &Base::baseSignal, [](){});           // ok
//Interconnect::connect(*a, &Derived::derivedSignal, [](){});   // error
Interconnect::connect(*b, &Derived::derivedSignal, [](){});     // ok
/* [Emitter-connect-emitter-type] */
}

{
/* [Emitter-connect-receiver-type] */
class Foo: public Interconnect::Emitter {
    public:
        Signal signal() { return emit(&Foo::signal); }
};

class Base: public Interconnect::Receiver {
    public:
        void baseSlot() {}
};

class Derived: public Base {
    public:
        void derivedSlot() {}
};

Foo foo;
Base* a = new Derived;
Derived* b = new Derived;

Interconnect::connect(foo, &Foo::signal, *a, &Base::baseSlot);         // ok
Interconnect::connect(foo, &Foo::signal, *b, &Base::baseSlot);         // ok
//Interconnect::connect(foo, &Foo::signal, *a, &Derived::derivedSlot); // error
Interconnect::connect(foo, &Foo::signal, *b, &Derived::derivedSlot);   // ok
/* [Emitter-connect-receiver-type] */

/* [Emitter-connect-receiver-multiple-inheritance] */
class MyString: public std::string, public Interconnect::Receiver {};

std::string c;
MyString d;

//Interconnect::connect(foo, &Foo::signal, c, &std::string::clear);    // error
Interconnect::connect(foo, &Foo::signal, d, &std::string::clear);      // ok
/* [Emitter-connect-receiver-multiple-inheritance] */
}

{
/* [StateMachine-states-inputs] */
enum class State: std::uint8_t {
    Ready,
    Printing,
    Finished
};

enum class Input: std::uint8_t {
    Operate,
    TakeDocument
};
/* [StateMachine-states-inputs] */

/* [StateMachine-typedef] */
typedef Interconnect::StateMachine<3, 2, State, Input> Printer;
/* [StateMachine-typedef] */

/* [StateMachine-transitions] */
Printer p;
p.addTransitions({
    {State::Ready,      Input::Operate,         State::Printing},
    {State::Printing,   Input::Operate,         State::Finished},
    {State::Finished,   Input::TakeDocument,    State::Ready}
});
/* [StateMachine-transitions] */

/* [StateMachine-connect] */
Interconnect::connect(p, &Printer::entered<State::Ready>,
    [](State) { Utility::Debug() << "Printer is ready."; });
Interconnect::connect(p, &Printer::entered<State::Finished>,
    [](State) { Utility::Debug() << "Finished. Please take the document."; });
Interconnect::connect(p, &Printer::entered<State::Printing>,
    [](State) { Utility::Debug() << "Starting the print..."; });
Interconnect::connect(p, &Printer::exited<State::Printing>,
    [](State) { Utility::Debug() << "Finishing the print..."; });
/* [StateMachine-connect] */

/* [StateMachine-step] */
p.step(Input::Operate);
p.step(Input::Operate);
p.step(Input::TakeDocument);
/* [StateMachine-step] */
}

}
