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

#include <functional>

#include "Corrade/Containers/Optional.h"
#include "Corrade/Interconnect/Emitter.h"
#include "Corrade/Interconnect/Receiver.h"
#include "Corrade/TestSuite/Tester.h"

namespace Corrade { namespace Interconnect { namespace Test { namespace {

struct Benchmark: TestSuite::Tester {
    explicit Benchmark();

    void connect1kFunctions();
    void connect1kMembers();

    void destructBaseline();
    void destruct1kFunctions();
    void destruct1kMembersEmitterFirst();
    void destruct1kMembersReceiverFirst();

    void call1kFunctions();
    void call1kStdFunctions();
    void call1kFunctionConnectionData();
    void call1kLambdaConnectionData();
    void call1kLambdaHeapConnectionData();
    void call1kMemberConnectionData();
    void callSlotFunction1000x();
    void call1kSlotFunctions();
    void call1kSlotLambdas();
    void call1kSlotLambdasHeap();
    void call1kSlotMembers();
};

Benchmark::Benchmark() {
    addBenchmarks({&Benchmark::connect1kFunctions,
                   &Benchmark::connect1kMembers}, 10);

    addBenchmarks({&Benchmark::destructBaseline,
                   &Benchmark::destruct1kFunctions,
                   &Benchmark::destruct1kMembersEmitterFirst}, 100);

    /* Takes like two hours on libstdc++:
       https://gcc.gnu.org/bugzilla/show_bug.cgi?id=41975 */
    addBenchmarks({&Benchmark::destruct1kMembersReceiverFirst}, 10);

    addBenchmarks({&Benchmark::call1kFunctions,
                   &Benchmark::call1kStdFunctions,
                   &Benchmark::call1kFunctionConnectionData,
                   &Benchmark::call1kLambdaConnectionData,
                   &Benchmark::call1kLambdaHeapConnectionData,
                   &Benchmark::call1kMemberConnectionData,
                   &Benchmark::callSlotFunction1000x,
                   &Benchmark::call1kSlotFunctions,
                   &Benchmark::call1kSlotLambdas,
                   &Benchmark::call1kSlotLambdasHeap,
                   &Benchmark::call1kSlotMembers}, 25);
}

int gloablOutput;

CORRADE_NEVER_INLINE void freeFunctionSlot() {
    ++gloablOutput;
}

void Benchmark::connect1kFunctions() {
    struct E: Emitter {
        Signal fire() {
            return emit(&E::fire);
        }
    } emitter;

    CORRADE_BENCHMARK(1000)
        connect(emitter, &E::fire, freeFunctionSlot);

    CORRADE_COMPARE(emitter.signalConnectionCount(), 1000);
}

void Benchmark::connect1kMembers() {
    struct E: Emitter {
        Signal fire() {
            return emit(&E::fire);
        }
    } emitter;

    struct R: Receiver {
        int output = 0;

        void receive() { ++output; }
    } receiver;

    CORRADE_BENCHMARK(1000)
        connect(emitter, &E::fire, receiver, &R::receive);

    CORRADE_COMPARE(emitter.signalConnectionCount(), 1000);
}

void Benchmark::destructBaseline() {
    struct E: Emitter {
        Signal fire() {
            return emit(&E::fire);
        }
    };

    Containers::Optional<E> emitter{Containers::InPlaceInit};

    CORRADE_BENCHMARK(1)
        emitter = Containers::NullOpt;
}

void Benchmark::destruct1kFunctions() {
    struct E: Emitter {
        Signal fire() {
            return emit(&E::fire);
        }
    };

    Containers::Optional<E> emitter{Containers::InPlaceInit};

    for(std::size_t i = 0; i != 1000; ++i)
        connect(*emitter, &E::fire, freeFunctionSlot);

    CORRADE_BENCHMARK(1)
        emitter = Containers::NullOpt;
}

void Benchmark::destruct1kMembersEmitterFirst() {
    struct E: Emitter {
        Signal fire() {
            return emit(&E::fire);
        }
    };

    Containers::Optional<E> emitter{Containers::InPlaceInit};

    struct R: Receiver {
        int output = 0;

        void receive() { ++output; }
    } receiver;

    for(std::size_t i = 0; i != 1000; ++i)
        connect(*emitter, &E::fire, receiver, &R::receive);

    CORRADE_BENCHMARK(1)
        emitter = Containers::NullOpt;
}

void Benchmark::destruct1kMembersReceiverFirst() {
    struct E: Emitter {
        Signal fire() {
            return emit(&E::fire);
        }
    } emitter;

    struct R: Receiver {
        int output = 0;

        void receive() { ++output; }
    };

    Containers::Optional<R> receiver{Containers::InPlaceInit};

    for(std::size_t i = 0; i != 1000; ++i)
        connect(emitter, &E::fire, *receiver, &R::receive);

    CORRADE_BENCHMARK(1)
        receiver = Containers::NullOpt;
}

void Benchmark::call1kFunctions() {
    gloablOutput = 0;

    CORRADE_BENCHMARK(100) {
        for(std::size_t i = 0; i != 1000; ++i)
            freeFunctionSlot();
    }

    CORRADE_COMPARE(gloablOutput, 1000*100);
}

void Benchmark::call1kStdFunctions() {
    gloablOutput = 0;

    std::function<void()> a{freeFunctionSlot};

    CORRADE_BENCHMARK(100) {
        for(std::size_t i = 0; i != 1000; ++i)
            a();
    }

    CORRADE_COMPARE(gloablOutput, 1000*100);
}

void Benchmark::call1kFunctionConnectionData() {
    gloablOutput = 0;

    struct: Emitter {} emitter;

    auto d = Implementation::ConnectionData::createFunctor(freeFunctionSlot);

    CORRADE_BENCHMARK(100) {
        for(std::size_t i = 0; i != 1000; ++i)
            reinterpret_cast<void(*)(Implementation::ConnectionData::Storage&)>(d.call)(d.storage);
    }

    CORRADE_COMPARE(gloablOutput, 1000*100);
}

void Benchmark::call1kLambdaConnectionData() {
    int output = 0;

    struct: Emitter {} emitter;

    auto d = Implementation::ConnectionData::createFunctor([&output]() { ++output; });

    CORRADE_BENCHMARK(100) {
        for(std::size_t i = 0; i != 1000; ++i)
            reinterpret_cast<void(*)(Implementation::ConnectionData::Storage&)>(d.call)(d.storage);
    }

    CORRADE_COMPARE(output, 1000*100);
}

void Benchmark::call1kLambdaHeapConnectionData() {
    struct Destructor {
        int value = 1;
        ~Destructor() { gloablOutput += 1; }
    } a;

    int output = 0;

    struct: Emitter {} emitter;

    auto d = Implementation::ConnectionData::createFunctor([a, &output]() { output += a.value; });

    CORRADE_BENCHMARK(100) {
        for(std::size_t i = 0; i != 1000; ++i)
            reinterpret_cast<void(*)(Implementation::ConnectionData::Storage&)>(d.call)(d.storage);
    }

    CORRADE_COMPARE(output, 1000*100);
}

void Benchmark::call1kMemberConnectionData() {
    struct: Emitter {} emitter;
    struct R: Receiver {
        int output = 0;

        void receive() { ++output; }
    } receiver;

    auto d = Implementation::ConnectionData::createMember<R, R>(receiver, &R::receive);

    CORRADE_BENCHMARK(100) {
        for(std::size_t i = 0; i != 1000; ++i)
            reinterpret_cast<void(*)(Implementation::ConnectionData::Storage&)>(d.call)(d.storage);
    }

    CORRADE_COMPARE(receiver.output, 1000*100);
}

void Benchmark::callSlotFunction1000x() {
    gloablOutput = 0;

    struct E: Emitter {
        Signal fire() {
            return emit(&E::fire);
        }
    } emitter;

    connect(emitter, &E::fire, freeFunctionSlot);

    CORRADE_BENCHMARK(100)
        for(std::size_t i = 0; i != 1000; ++i)
            emitter.fire();

    CORRADE_COMPARE(gloablOutput, 1000*100);
}

void Benchmark::call1kSlotFunctions() {
    gloablOutput = 0;

    struct E: Emitter {
        Signal fire() {
            return emit(&E::fire);
        }
    } emitter;

    for(std::size_t i = 0; i != 1000; ++i)
        connect(emitter, &E::fire, freeFunctionSlot);

    CORRADE_BENCHMARK(100)
        emitter.fire();

    CORRADE_COMPARE(gloablOutput, 1000*100);
}

void Benchmark::call1kSlotLambdas() {
    gloablOutput = 0;

    struct E: Emitter {
        Signal fire() {
            return emit(&E::fire);
        }
    } emitter;

    for(std::size_t i = 0; i != 1000; ++i)
        connect(emitter, &E::fire, [](){ ++gloablOutput; });

    CORRADE_BENCHMARK(100)
        emitter.fire();

    CORRADE_COMPARE(gloablOutput, 1000*100);
}

void Benchmark::call1kSlotLambdasHeap() {
    struct Destructor {
        int value = 1;
        ~Destructor() { gloablOutput += 1; }
    } a;

    int output = 0;

    struct E: Emitter {
        Signal fire() {
            return emit(&E::fire);
        }
    } emitter;

    for(std::size_t i = 0; i != 1000; ++i)
        connect(emitter, &E::fire, [a, &output](){ output += a.value; });

    CORRADE_BENCHMARK(100)
        emitter.fire();

    CORRADE_COMPARE(output, 1000*100);
}

void Benchmark::call1kSlotMembers() {
    struct E: Emitter {
        Signal fire() {
            return emit(&E::fire);
        }
    } emitter;

    struct R: Receiver {
        int output = 0;

        void receive() { ++output; }
    } receiver;

    for(std::size_t i = 0; i != 1000; ++i)
        connect(emitter, &E::fire, receiver, &R::receive);

    CORRADE_BENCHMARK(100)
        emitter.fire();

    CORRADE_COMPARE(receiver.output, 1000*100);
}

}}}}

CORRADE_TEST_MAIN(Corrade::Interconnect::Test::Benchmark)
