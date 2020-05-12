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

#include <sstream>

#include "Corrade/Interconnect/StateMachine.h"
#include "Corrade/TestSuite/Tester.h"
#include "Corrade/Utility/DebugStl.h" /** @todo remove when <sstream> is gone */

namespace Corrade { namespace Interconnect { namespace Test { namespace {

struct StateMachineTest: TestSuite::Tester {
    explicit StateMachineTest();

    void signalData();
    void test();
};

StateMachineTest::StateMachineTest() {
    addTests({&StateMachineTest::signalData,
              &StateMachineTest::test});
}

enum class State: std::uint8_t {
    Start,
    End
};

enum class Input: std::uint8_t {
    KeyA,
    KeyB
};

typedef Interconnect::StateMachine<2, 2, State, Input> StateMachine;

void StateMachineTest::signalData() {
    #ifndef CORRADE_MSVC2019_COMPATIBILITY
    Implementation::SignalData data1{&StateMachine::entered<State::Start>};
    Implementation::SignalData data2{&StateMachine::entered<State::End>};
    Implementation::SignalData data3{&StateMachine::exited<State::Start>};

    Implementation::SignalData data4{&StateMachine::stepped<State::Start, State::End>};
    Implementation::SignalData data5{&StateMachine::stepped<State::End, State::Start>};
    #else
    auto data1 = Implementation::SignalData::create<StateMachine>(&StateMachine::entered<State::Start>);
    auto data2 = Implementation::SignalData::create<StateMachine>(&StateMachine::entered<State::End>);
    auto data3 = Implementation::SignalData::create<StateMachine>(&StateMachine::exited<State::Start>);

    auto data4 = Implementation::SignalData::create<StateMachine>(&StateMachine::stepped<State::Start, State::End>);
    auto data5 = Implementation::SignalData::create<StateMachine>(&StateMachine::stepped<State::End, State::Start>);
    #endif

    CORRADE_VERIFY(data1 != data2);
    CORRADE_VERIFY(data1 != data3);

    CORRADE_VERIFY(data4 != data5);
}

void StateMachineTest::test() {
    StateMachine m;
    m.addTransitions({
        {State::Start,  Input::KeyA,    State::End},
        {State::End,    Input::KeyB,    State::Start}
    });

    std::ostringstream out;
    Debug redirectDebug{&out};

    Interconnect::connect(m, &StateMachine::entered<State::Start>,
        [](State s) { Debug() << "start entered, previous" << std::uint8_t(s); });
    Interconnect::connect(m, &StateMachine::exited<State::Start>,
        [](State s) { Debug() << "start exited, next" << std::uint8_t(s); });
    Interconnect::connect(m, &StateMachine::entered<State::End>,
        [](State s) { Debug() << "end entered, previous" << std::uint8_t(s); });
    Interconnect::connect(m, &StateMachine::exited<State::End>,
        [](State s) { Debug() << "end exited, next" << std::uint8_t(s); });

    Interconnect::connect(m, &StateMachine::stepped<State::End, State::Start>,
        []() { Debug() << "going from end to start"; });
    Interconnect::connect(m, &StateMachine::stepped<State::Start, State::End>,
        []() { Debug() << "going from start to end"; });

    m.step(Input::KeyA)
     .step(Input::KeyB);
    CORRADE_COMPARE(out.str(), "start exited, next 1\n"
                               "going from start to end\n"
                               "end entered, previous 0\n"
                               "end exited, next 0\n"
                               "going from end to start\n"
                               "start entered, previous 1\n");
}

}}}}

CORRADE_TEST_MAIN(Corrade::Interconnect::Test::StateMachineTest)
