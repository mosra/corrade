/*
    This file is part of Corrade.

    Copyright © 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016,
                2017, 2018, 2019, 2020, 2021, 2022, 2023, 2024
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

#include "Corrade/Containers/Function.h"
#include "Corrade/Containers/String.h"
#include "Corrade/TestSuite/Tester.h"

namespace Corrade { namespace Containers { namespace Test { namespace {

struct FunctionTest: TestSuite::Tester {
    explicit FunctionTest();

    void resetCounters();

    void noAllocateInitTagNoDefaultConstructor();
    void noAllocateInitTagInlineDefinition();

    void isFunctor();
    void isFunctorOverload();
    void isFunctorLambda();

    void constructDefault();
    void constructDefaultData();

    void constructFreeFunction();
    /* Lambda as a free function gets tested in constructStatelessLambda()
       below */
    void constructFreeFunctionNull();
    void constructFreeFunctionOverload();
    void constructMemberFunction();
    void constructMemberFunctionLvalue();
    void constructMemberFunctionConst();
    void constructMemberFunctionConstLvalue();
    void constructMemberFunctionNull();
    void constructMemberFunctionOverload();
    void constructMemberFunctionInBase();
    void constructMemberFunctionInBaseLvalue();
    void constructMemberFunctionInBaseConst();
    void constructMemberFunctionInBaseConstLvalue();
    void constructMemberFunctionMultipleInheritance();
    void constructMemberFunctionMultipleVirtualInheritance();

    void constructStatelessFunctor();
    void constructStatelessLambda();
    void constructStatefulTrivialSmallFunctor();
    void constructStatefulTrivialSmallLambda();
    void constructStatefulTrivialLargeFunctor();
    void constructStatefulSmallFunctor();
    void constructStatefulLargeFunctor();

    void constructMoveOnlyFunctor();
    void constructTriviallyCopyableMoveOnlyFunctor();
    void constructNonTriviallyDestructibleFunctor();
    void constructNonTriviallyDestructibleFunctorData();
    void constructNonTriviallyCopyableFunctor();

    /* Compared to e.g. ArrayTest::constructorExplicitInCopyInitialization(),
       neither the functor nor any of the argument / return types are default
       constructed here, so that variant doesn't need to be tested here */
    void constructTrivialFunctorPlainStruct();
    void constructFunctorPlainStruct();

    void constructTrivialFunctorOverload();
    void constructFunctorOverload();
    void constructTrivialFunctorRvalueOverload();
    void constructFunctorRvalueOverload();

    void constructCopy();
    void constructCopyData();
    void constructMove();
    void constructMoveData();

    void implicitlyConvertibleArgumentFunction();
    void implicitlyConvertibleArgumentMemberFunction();
    void implicitlyConvertibleArgumentStatelessFunctor();
    void implicitlyConvertibleArgumentTrivialFunctor();
    void implicitlyConvertibleArgumentFunctor();
    void implicitlyConvertibleResultFunction();
    void implicitlyConvertibleResultMemberFunction();
    void implicitlyConvertibleResultStatelessFunctor();
    void implicitlyConvertibleResultTrivialFunctor();
    void implicitlyConvertibleResultFunctor();
    void implicitlyConvertibleFunctorOverload();

    void rvalueArgumentFunction();
    void rvalueArgumentMemberFunction();
    void rvalueArgumentTrivialFunctor();
    void rvalueArgumentFunctor();
    void rvalueResultFunction();
    void rvalueResultMemberFunction();
    void rvalueResultTrivialFunctor();
    void rvalueResultFunctor();

    void moveOnlyArgumentFunction();
    void moveOnlyArgumentMemberFunction();
    void moveOnlyArgumentTrivialFunctor();
    void moveOnlyArgumentFunctor();
    void moveOnlyResultFunction();
    void moveOnlyResultMemberFunction();
    void moveOnlyResultTrivialFunctor();
    void moveOnlyResultFunctor();

    void functionArgumentOverloadFunction();
    void functionArgumentOverloadMemberFunction();
    void functionArgumentOverloadTrivialFunctor();
    void functionArgumentOverloadFunctor();
    void functionArgumentOverloadLambda();
    void functionResultOverloadFunction();
    void functionResultOverloadMemberFunction();
    void functionResultOverloadTrivialFunctor();
    void functionResultOverloadFunctor();
    void functionResultOverloadLambda();
};

FunctionTest::FunctionTest() {
    addTests({&FunctionTest::noAllocateInitTagNoDefaultConstructor,
              &FunctionTest::noAllocateInitTagInlineDefinition,

              &FunctionTest::isFunctor,
              &FunctionTest::isFunctorOverload,
              &FunctionTest::isFunctorLambda,

              &FunctionTest::constructDefault,
              &FunctionTest::constructDefaultData,

              &FunctionTest::constructFreeFunction,
              &FunctionTest::constructFreeFunctionNull,
              &FunctionTest::constructFreeFunctionOverload,
              &FunctionTest::constructMemberFunction,
              &FunctionTest::constructMemberFunctionLvalue,
              &FunctionTest::constructMemberFunctionConst,
              &FunctionTest::constructMemberFunctionConstLvalue,
              &FunctionTest::constructMemberFunctionNull,
              &FunctionTest::constructMemberFunctionOverload,
              &FunctionTest::constructMemberFunctionInBase,
              &FunctionTest::constructMemberFunctionInBaseLvalue,
              &FunctionTest::constructMemberFunctionInBaseConst,
              &FunctionTest::constructMemberFunctionInBaseConstLvalue,
              &FunctionTest::constructMemberFunctionMultipleInheritance,
              &FunctionTest::constructMemberFunctionMultipleVirtualInheritance,

              &FunctionTest::constructStatelessFunctor,
              &FunctionTest::constructStatelessLambda,
              &FunctionTest::constructStatefulTrivialSmallFunctor,
              &FunctionTest::constructStatefulTrivialSmallLambda,
              &FunctionTest::constructStatefulTrivialLargeFunctor,
              &FunctionTest::constructStatefulSmallFunctor,
              &FunctionTest::constructStatefulLargeFunctor});

    addTests({&FunctionTest::constructMoveOnlyFunctor},
             &FunctionTest::resetCounters,
             &FunctionTest::resetCounters);

    addTests({&FunctionTest::constructTriviallyCopyableMoveOnlyFunctor});

    addTests({&FunctionTest::constructNonTriviallyDestructibleFunctor,
              &FunctionTest::constructNonTriviallyDestructibleFunctorData,
              &FunctionTest::constructNonTriviallyCopyableFunctor},
             &FunctionTest::resetCounters,
             &FunctionTest::resetCounters);

    addTests({&FunctionTest::constructTrivialFunctorPlainStruct,
              &FunctionTest::constructFunctorPlainStruct,

              &FunctionTest::constructTrivialFunctorOverload,
              &FunctionTest::constructFunctorOverload,
              &FunctionTest::constructTrivialFunctorRvalueOverload,
              &FunctionTest::constructFunctorRvalueOverload,

              &FunctionTest::constructCopy,
              &FunctionTest::constructCopyData});

    addTests({&FunctionTest::constructMove,
              &FunctionTest::constructMoveData},
            &FunctionTest::resetCounters,
            &FunctionTest::resetCounters);

    addTests({&FunctionTest::implicitlyConvertibleArgumentFunction,
              &FunctionTest::implicitlyConvertibleArgumentMemberFunction,
              &FunctionTest::implicitlyConvertibleArgumentStatelessFunctor,
              &FunctionTest::implicitlyConvertibleArgumentTrivialFunctor,
              &FunctionTest::implicitlyConvertibleArgumentFunctor,
              &FunctionTest::implicitlyConvertibleResultFunction,
              &FunctionTest::implicitlyConvertibleResultMemberFunction,
              &FunctionTest::implicitlyConvertibleResultStatelessFunctor,
              &FunctionTest::implicitlyConvertibleResultTrivialFunctor,
              &FunctionTest::implicitlyConvertibleResultFunctor,
              &FunctionTest::implicitlyConvertibleFunctorOverload,

              &FunctionTest::rvalueArgumentFunction,
              &FunctionTest::rvalueArgumentMemberFunction,
              &FunctionTest::rvalueArgumentTrivialFunctor,
              &FunctionTest::rvalueArgumentFunctor,
              &FunctionTest::rvalueResultFunction,
              &FunctionTest::rvalueResultMemberFunction,
              &FunctionTest::rvalueResultTrivialFunctor,
              &FunctionTest::rvalueResultFunctor});

    addTests({&FunctionTest::moveOnlyArgumentFunction,
              &FunctionTest::moveOnlyArgumentMemberFunction,
              &FunctionTest::moveOnlyArgumentTrivialFunctor,
              &FunctionTest::moveOnlyArgumentFunctor,
              &FunctionTest::moveOnlyResultFunction,
              &FunctionTest::moveOnlyResultMemberFunction,
              &FunctionTest::moveOnlyResultTrivialFunctor,
              &FunctionTest::moveOnlyResultFunctor},
             &FunctionTest::resetCounters,
             &FunctionTest::resetCounters);

    addTests({&FunctionTest::functionArgumentOverloadFunction,
              &FunctionTest::functionArgumentOverloadMemberFunction,
              &FunctionTest::functionArgumentOverloadTrivialFunctor,
              &FunctionTest::functionArgumentOverloadFunctor,
              &FunctionTest::functionArgumentOverloadLambda,
              &FunctionTest::functionResultOverloadFunction,
              &FunctionTest::functionResultOverloadMemberFunction,
              &FunctionTest::functionResultOverloadTrivialFunctor,
              &FunctionTest::functionResultOverloadFunctor,
              &FunctionTest::functionResultOverloadLambda});
}

struct MoveOnlyAccumulator {
    static int constructed;
    static int destructed;
    static int moved;

    explicit MoveOnlyAccumulator() noexcept { ++constructed; }
    MoveOnlyAccumulator(const MoveOnlyAccumulator&) = delete;
    MoveOnlyAccumulator(MoveOnlyAccumulator&& other) noexcept: a{other.a} {
        ++constructed;
        ++moved;
    }
    ~MoveOnlyAccumulator() { ++destructed; }

    MoveOnlyAccumulator& operator=(const MoveOnlyAccumulator&) = delete;
    /* Clang says this one is unused, but removing it could have undesirable
       consequences, so just suppress the warning */
    CORRADE_UNUSED MoveOnlyAccumulator& operator=(MoveOnlyAccumulator&& other) noexcept {
        a = other.a;
        ++moved;
        return *this;
    }

    int operator()(int value) {
        a += value;
        return a;
    }

    int a = 13;
};

int MoveOnlyAccumulator::constructed = 0;
int MoveOnlyAccumulator::destructed = 0;
int MoveOnlyAccumulator::moved = 0;

/* It's non-copyable but is std::is_trivially_copyable, as explained here -- it
   wouldn't be if any of the move constructors did something extra:
   https://www.foonathan.net/2021/03/trivially-copyable/ The Function internals
   have to properly move it on construction, but then can continue to rely on
   simple memory copy. */
struct TriviallyCopyableMoveOnlyAccumulator {
    explicit TriviallyCopyableMoveOnlyAccumulator() = default;
    TriviallyCopyableMoveOnlyAccumulator(const TriviallyCopyableMoveOnlyAccumulator&) = delete;
    /* Clang says this one is unused, but removing it could have undesirable
       consequences, so just suppress the warning */
    CORRADE_UNUSED TriviallyCopyableMoveOnlyAccumulator(TriviallyCopyableMoveOnlyAccumulator&&) = default;
    TriviallyCopyableMoveOnlyAccumulator& operator=(const TriviallyCopyableMoveOnlyAccumulator&) = delete;
    /* Clang says this one is unused, but removing it could have undesirable
       consequences, so just suppress the warning */
    CORRADE_UNUSED TriviallyCopyableMoveOnlyAccumulator& operator=(TriviallyCopyableMoveOnlyAccumulator&& other) = default;

    int operator()(int value) {
        a += value;
        return a;
    }

    int a = 13;
};

struct NonTriviallyDestructibleAccumulator {
    static int constructed;
    static int destructed;

    explicit NonTriviallyDestructibleAccumulator() noexcept { ++constructed; }
    ~NonTriviallyDestructibleAccumulator() { ++destructed; }

    int operator()(int value) {
        a += value;
        return a;
    }

    int a = 13;
};

int NonTriviallyDestructibleAccumulator::constructed = 0;
int NonTriviallyDestructibleAccumulator::destructed = 0;

struct NonTriviallyCopyableAccumulator {
    static int constructed;
    static int copied;
    static int moved;

    explicit NonTriviallyCopyableAccumulator() noexcept { ++constructed; }
    NonTriviallyCopyableAccumulator(const NonTriviallyCopyableAccumulator& other) noexcept: a{other.a} {
        ++constructed;
        ++copied;
    }
    /* Clang says this one is unused, but removing it could have undesirable
       consequences, so just suppress the warning */
    CORRADE_UNUSED NonTriviallyCopyableAccumulator(NonTriviallyCopyableAccumulator&& other) noexcept: a{other.a} {
        ++constructed;
        ++moved;
    }

    NonTriviallyCopyableAccumulator& operator=(const NonTriviallyCopyableAccumulator& other) noexcept {
        a = other.a;
        ++copied;
        return *this;
    }
    /* Clang says this one is unused, but removing it could have undesirable
       consequences, so just suppress the warning */
    CORRADE_UNUSED NonTriviallyCopyableAccumulator& operator=(NonTriviallyCopyableAccumulator&& other) noexcept {
        a = other.a;
        ++moved;
        return *this;
    }

    int operator()(int value) {
        a += value;
        return a;
    }

    int a = 13;
};

int NonTriviallyCopyableAccumulator::constructed = 0;
int NonTriviallyCopyableAccumulator::copied = 0;
int NonTriviallyCopyableAccumulator::moved = 0;

struct MoveOnly {
    static int constructed;
    static int destructed;
    static int moved;

    explicit MoveOnly(int a) noexcept: a{a} { ++constructed; }
    MoveOnly(const MoveOnly&) = delete;
    /* Clang says this one is unused, but removing it could have undesirable
       consequences, so just suppress the warning */
    CORRADE_UNUSED MoveOnly(MoveOnly&& other) noexcept: a{other.a} {
        ++constructed;
        ++moved;
    }
    ~MoveOnly() { ++destructed; }

    MoveOnly& operator=(const MoveOnly&) = delete;
    /* Clang says this one is unused, but removing it could have undesirable
       consequences, so just suppress the warning */
    CORRADE_UNUSED MoveOnly& operator=(MoveOnly&& other) noexcept {
        a = other.a;
        ++moved;
        return *this;
    }

    /* Clang says this one is unused, but we need it in order to have its
       signature checked against the expected */
    CORRADE_UNUSED int operator()(int value) {
        a += value;
        return a;
    }

    int a = 13;
};

int MoveOnly::constructed = 0;
int MoveOnly::destructed = 0;
int MoveOnly::moved = 0;

void FunctionTest::resetCounters() {
    MoveOnlyAccumulator::constructed =
        MoveOnlyAccumulator::destructed =
            MoveOnlyAccumulator::moved =
                NonTriviallyDestructibleAccumulator::constructed =
                    NonTriviallyDestructibleAccumulator::destructed =
                        NonTriviallyCopyableAccumulator::constructed =
                            NonTriviallyCopyableAccumulator::copied =
                                NonTriviallyCopyableAccumulator::moved =
                                    MoveOnly::constructed =
                                        MoveOnly::destructed =
                                            MoveOnly::moved = 0;
}

/** @todo move these to TagsTest once the tag gets used outside of Function */
void FunctionTest::noAllocateInitTagNoDefaultConstructor() {
    /* Isn't default constructible to prevent ambiguity when calling
       foo({}) if both foo(TagT) and foo(whatever) is available */
    CORRADE_VERIFY(!std::is_default_constructible<NoAllocateInitT>::value);
}

void FunctionTest::noAllocateInitTagInlineDefinition() {
    /* Just a sanity check that the types match */
    CORRADE_VERIFY(std::is_same<decltype(NoAllocateInit), const NoAllocateInitT>::value);
}

void FunctionTest::isFunctor() {
    /* Non-function types aren't functors */
    CORRADE_VERIFY(!Implementation::IsFunctor<int, int()>::value);

    /* Plain function, function pointers and member function pointers aren't
       functors */
    CORRADE_VERIFY(!Implementation::IsFunctor<int(int, int), int(int, int)>::value);
    CORRADE_VERIFY(!Implementation::IsFunctor<int(*)(int, int), int(int, int)>::value);
    CORRADE_VERIFY(!Implementation::IsFunctor<int(FunctionTest::*)(int, int), int(int, int)>::value);

    /* Struct types without any operator() aren't functors */
    struct Empty {};
    CORRADE_VERIFY(!Implementation::IsFunctor<Empty, int()>::value);

    struct Functor {
        int operator()(int a, int b) {
            return a + b;
        }
    };
    struct FunctorLvalue {
        int operator()(int a, int b) & {
            return a + b;
        }
    };
    struct FunctorConst {
        int operator()(int a, int b) const {
            return a + b;
        }
    };
    struct FunctorConstLvalue {
        int operator()(int a, int b) const & {
            return a + b;
        }
    };

    /* Functors ... are functors, with all possible r-value overloads */
    CORRADE_VERIFY(Implementation::IsFunctor<Functor, int(int, int)>::value);
    CORRADE_VERIFY(Implementation::IsFunctor<FunctorLvalue, int(int, int)>::value);
    CORRADE_VERIFY(Implementation::IsFunctor<FunctorConst, int(int, int)>::value);
    CORRADE_VERIFY(Implementation::IsFunctor<FunctorConstLvalue, int(int, int)>::value);

    /* But only if they have matching argument types */
    CORRADE_VERIFY(!Implementation::IsFunctor<Functor, int(int, float)>::value);
    CORRADE_VERIFY(!Implementation::IsFunctor<FunctorLvalue, int(float, int)>::value);
    CORRADE_VERIFY(!Implementation::IsFunctor<FunctorConst, int(int, float)>::value);
    CORRADE_VERIFY(!Implementation::IsFunctor<FunctorConstLvalue, int(float, int)>::value);

    /* And a matching result type */
    CORRADE_VERIFY(!Implementation::IsFunctor<Functor, float(int, int)>::value);
    CORRADE_VERIFY(!Implementation::IsFunctor<FunctorLvalue, float(int, int)>::value);
    CORRADE_VERIFY(!Implementation::IsFunctor<FunctorConst, float(int, int)>::value);
    CORRADE_VERIFY(!Implementation::IsFunctor<FunctorConstLvalue, float(int, int)>::value);
}

void FunctionTest::isFunctorOverload() {
    struct FunctorOverload {
        int operator()(int a, int b) {
            return a + b;
        }
        float operator()(float a, float b) {
            return a + b;
        }
    };

    /* Overloads are functors if they match */
    CORRADE_VERIFY(Implementation::IsFunctor<FunctorOverload, int(int, int)>::value);
    CORRADE_VERIFY(Implementation::IsFunctor<FunctorOverload, float(float, float)>::value);
    CORRADE_VERIFY(!Implementation::IsFunctor<FunctorOverload, int(int, float)>::value);
    CORRADE_VERIFY(!Implementation::IsFunctor<FunctorOverload, float(int, int)>::value);
}

void FunctionTest::isFunctorLambda() {
    auto stateless = [](int a, int b) -> int {
        return a + b;
    };

    /* Stateless lambdas (convertible to function pointers) aren't functors */
    CORRADE_VERIFY(!Implementation::IsFunctor<decltype(stateless), int(int, int)>::value);

    int x = 0;
    auto stateful = [x](int a, int b) -> int {
        return x + a + b;
    };
    auto statefulDifferentArgument = [x](int a, float b) -> int {
        return x + a + int(b);
    };
    auto statefulDifferentResult = [x](int a, int b) -> float {
        return float(x + a + b);
    };

    /* Stateful lambdas are functors but only if they have a matching
       signature */
    CORRADE_VERIFY(Implementation::IsFunctor<decltype(stateful), int(int, int)>::value);
    CORRADE_VERIFY(!Implementation::IsFunctor<decltype(statefulDifferentArgument), int(int, int)>::value);
    CORRADE_VERIFY(!Implementation::IsFunctor<decltype(statefulDifferentResult), int(int, int)>::value);
}

void FunctionTest::constructDefault() {
    Function<int(int)> a;
    Function<int(int)> b = nullptr;
    CORRADE_VERIFY(!a);
    CORRADE_VERIFY(!b);
    CORRADE_VERIFY(!a.isAllocated());
    CORRADE_VERIFY(!b.isAllocated());

    /* Making it testable would mean returning early with a default-constructed
       return value, which isn't possible for arbitrary types */
    CORRADE_SKIP("Can't reliably test null function call assertion.");
}

void FunctionTest::constructDefaultData() {
    FunctionData a;
    FunctionData b = nullptr;
    CORRADE_VERIFY(!a);
    CORRADE_VERIFY(!b);
    CORRADE_VERIFY(!a.isAllocated());
    CORRADE_VERIFY(!b.isAllocated());
}

int increment(int value) {
    return value + 1;
}

void FunctionTest::constructFreeFunction() {
    Function<int(int)> a = increment;
    CORRADE_VERIFY(a);
    CORRADE_VERIFY(!a.isAllocated());

    CORRADE_COMPARE(a(3), 4);
    CORRADE_COMPARE(a(-3), -2);
}

void FunctionTest::constructFreeFunctionNull() {
    /* Not doing a = nullptr because that'd pick the default constructor
       instead */
    int(*function)(int) = nullptr;
    Function<int(int)> a = function;
    CORRADE_VERIFY(!a);
    CORRADE_VERIFY(!a.isAllocated());

    /* Making it testable would mean returning early with a default-constructed
       return value, which isn't possible for arbitrary types */
    CORRADE_SKIP("Can't reliably test null function call assertion.");
}

int sum(int a, int b) {
    return a + b;
}
float sum(float a, float b) {
    return a + b;
}

void FunctionTest::constructFreeFunctionOverload() {
    Function<int(int, int)> a = sum;
    Function<float(float, float)> b = sum;
    CORRADE_VERIFY(a);
    CORRADE_VERIFY(b);
    CORRADE_VERIFY(!a.isAllocated());
    CORRADE_VERIFY(!b.isAllocated());

    CORRADE_COMPARE(a(3, 5), 8);
    CORRADE_COMPARE(b(3.1f, 5.1f), 8.2f);
}

void FunctionTest::constructMemberFunction() {
    struct Accumulator {
        int add(int value) {
            a += value;
            return a;
        }

        int a = 13;
    } accumulator;

    Function<int(int)> a{accumulator, &Accumulator::add};
    CORRADE_VERIFY(a);
    CORRADE_VERIFY(!a.isAllocated());

    CORRADE_COMPARE(a(2), 15);
    CORRADE_COMPARE(accumulator.a, 15);

    CORRADE_COMPARE(a(-7), 8);
    CORRADE_COMPARE(accumulator.a, 8);

    /* Member function pointers usually have a size of 2 pointers, except for
       non-MinGW Windows where they're just one pointer */
    #if !defined(CORRADE_TARGET_WINDOWS) || defined(CORRADE_TARGET_MINGW)
    CORRADE_COMPARE(sizeof(&Accumulator::add), 2*sizeof(std::size_t));
    #else
    CORRADE_COMPARE(sizeof(&Accumulator::add), sizeof(std::size_t));
    #endif
}

void FunctionTest::constructMemberFunctionLvalue() {
    struct Accumulator {
        int add(int value) & {
            a += value;
            return a;
        }

        int a = 13;
    } accumulator;

    Function<int(int)> a{accumulator, &Accumulator::add};
    CORRADE_VERIFY(a);
    CORRADE_VERIFY(!a.isAllocated());

    CORRADE_COMPARE(a(2), 15);
    CORRADE_COMPARE(accumulator.a, 15);

    CORRADE_COMPARE(a(-7), 8);
    CORRADE_COMPARE(accumulator.a, 8);

    /* Member function pointers usually have a size of 2 pointers, except for
       non-MinGW Windows where they're just one pointer */
    #if !defined(CORRADE_TARGET_WINDOWS) || defined(CORRADE_TARGET_MINGW)
    CORRADE_COMPARE(sizeof(&Accumulator::add), 2*sizeof(std::size_t));
    #else
    CORRADE_COMPARE(sizeof(&Accumulator::add), sizeof(std::size_t));
    #endif
}

void FunctionTest::constructMemberFunctionConst() {
    struct Accumulator {
        int add(int value) const {
            return a + value;
        }

        int a = 13;
    } accumulator;

    Function<int(int)> a{accumulator, &Accumulator::add};
    CORRADE_VERIFY(a);
    CORRADE_VERIFY(!a.isAllocated());

    CORRADE_COMPARE(a(2), 15);
    /* The function is const, the member is thus unaffected */
    CORRADE_COMPARE(a(-7), 6);
    CORRADE_COMPARE(accumulator.a, 13);

    /* Member function pointers usually have a size of 2 pointers, except for
       non-MinGW Windows where they're just one pointer */
    #if !defined(CORRADE_TARGET_WINDOWS) || defined(CORRADE_TARGET_MINGW)
    CORRADE_COMPARE(sizeof(&Accumulator::add), 2*sizeof(std::size_t));
    #else
    CORRADE_COMPARE(sizeof(&Accumulator::add), sizeof(std::size_t));
    #endif
}

void FunctionTest::constructMemberFunctionConstLvalue() {
    struct Accumulator {
        int add(int value) const & {
            return a + value;
        }

        int a = 13;
    } accumulator;

    Function<int(int)> a{accumulator, &Accumulator::add};
    CORRADE_VERIFY(a);
    CORRADE_VERIFY(!a.isAllocated());

    CORRADE_COMPARE(a(2), 15);
    /* The function is const, the member is thus unaffected */
    CORRADE_COMPARE(a(-7), 6);
    CORRADE_COMPARE(accumulator.a, 13);

    /* Member function pointers usually have a size of 2 pointers, except for
       non-MinGW Windows where they're just one pointer */
    #if !defined(CORRADE_TARGET_WINDOWS) || defined(CORRADE_TARGET_MINGW)
    CORRADE_COMPARE(sizeof(&Accumulator::add), 2*sizeof(std::size_t));
    #else
    CORRADE_COMPARE(sizeof(&Accumulator::add), sizeof(std::size_t));
    #endif
}

void FunctionTest::constructMemberFunctionNull() {
    struct Accumulator {
    } accumulator;

    /* nullptr isn't implicitly convertible to a member function pointer,
       apparently, so there has to be one extra overload for std::nullptr_t */
    int(Accumulator::*function)(int) = nullptr;
    int(Accumulator::*functionLvalue)(int) & = nullptr;
    int(Accumulator::*functionConst)(int) const = nullptr;
    int(Accumulator::*functionConstLvalue)(int) const & = nullptr;
    Function<int(int)> a{accumulator, function};
    Function<int(int)> b{accumulator, functionLvalue};
    Function<int(int)> c{accumulator, functionConst};
    Function<int(int)> d{accumulator, functionConstLvalue};
    Function<int(int)> e{accumulator, nullptr};
    CORRADE_VERIFY(!a);
    CORRADE_VERIFY(!b);
    CORRADE_VERIFY(!c);
    CORRADE_VERIFY(!d);
    CORRADE_VERIFY(!e);
    CORRADE_VERIFY(!a.isAllocated());
    CORRADE_VERIFY(!b.isAllocated());
    CORRADE_VERIFY(!c.isAllocated());
    CORRADE_VERIFY(!d.isAllocated());
    CORRADE_VERIFY(!e.isAllocated());

    /* Making it testable would mean returning early with a default-constructed
       return value, which isn't possible for arbitrary types */
    CORRADE_SKIP("Can't reliably test null function call assertion.");
}

void FunctionTest::constructMemberFunctionOverload() {
    struct Accumulator {
        int add(int value) {
            a += float(value);
            return int(a);
        }
        float add(float value) {
            a += float(value);
            return a;
        }

        float a = 13.1f;
    } accumulator;

    Function<int(int)> a{accumulator, &Accumulator::add};
    Function<float(float)> b{accumulator, &Accumulator::add};
    CORRADE_VERIFY(a);
    CORRADE_VERIFY(b);
    CORRADE_VERIFY(!a.isAllocated());
    CORRADE_VERIFY(!b.isAllocated());

    CORRADE_COMPARE(a(2), 15);
    CORRADE_COMPARE(accumulator.a, 15.1f);

    CORRADE_COMPARE(b(3.1f), 18.2f);
    CORRADE_COMPARE(accumulator.a, 18.2f);
}

void FunctionTest::constructMemberFunctionInBase() {
    struct Accumulator {
        int add(int value) {
            a += value;
            return a;
        }

        int a = 13;
    };

    struct Derived: Accumulator {
        int b = 26;
    } derived;

    Function<int(int)> a{derived, &Accumulator::add};
    CORRADE_VERIFY(a);
    CORRADE_VERIFY(!a.isAllocated());

    CORRADE_COMPARE(a(2), 15);
    CORRADE_COMPARE(derived.a, 15);
    CORRADE_COMPARE(derived.b, 26);

    /* Member function pointers usually have a size of 2 pointers, except for
       non-MinGW Windows where they're just one pointer */
    #if !defined(CORRADE_TARGET_WINDOWS) || defined(CORRADE_TARGET_MINGW)
    CORRADE_COMPARE(sizeof(&Derived::add), 2*sizeof(std::size_t));
    #else
    CORRADE_COMPARE(sizeof(&Derived::add), sizeof(std::size_t));
    #endif

    /* Uncomment to verify that it's not possible to wrap a function in a
       subclass of the instance. It causes an error inside a lambda, so it's
       not possible to verify with is_constructible. */
    #if 0
    struct Derived2: Derived {
        int sub(int value) {
            a -= value;
            return a;
        }
    };

    Function<int(int)> b{derived, &Derived2::sub};
    #endif
}

void FunctionTest::constructMemberFunctionInBaseLvalue() {
    struct Accumulator {
        int add(int value) & {
            a += value;
            return a;
        }

        int a = 13;
    };

    struct Derived: Accumulator {
        int b = 26;
    } derived;

    Function<int(int)> a{derived, &Accumulator::add};
    CORRADE_VERIFY(a);
    CORRADE_VERIFY(!a.isAllocated());

    CORRADE_COMPARE(a(2), 15);
    CORRADE_COMPARE(derived.a, 15);
    CORRADE_COMPARE(derived.b, 26);

    /* Member function pointers usually have a size of 2 pointers, except for
       non-MinGW Windows where they're just one pointer */
    #if !defined(CORRADE_TARGET_WINDOWS) || defined(CORRADE_TARGET_MINGW)
    CORRADE_COMPARE(sizeof(&Derived::add), 2*sizeof(std::size_t));
    #else
    CORRADE_COMPARE(sizeof(&Derived::add), sizeof(std::size_t));
    #endif

    /* Uncomment to verify that it's not possible to wrap a function in a
       subclass of the instance. It causes an error inside a lambda, so it's
       not possible to verify with is_constructible. */
    #if 0
    struct Derived2: Derived {
        int sub(int value) & {
            a -= value;
            return a;
        }
    };

    Function<int(int)> b{derived, &Derived2::sub};
    #endif
}

void FunctionTest::constructMemberFunctionInBaseConst() {
    struct Accumulator {
        int add(int value) const {
            return a + value;
        }

        int a = 13;
    };

    struct Derived: Accumulator {
        int b = 26;
    } derived;

    Function<int(int)> a{derived, &Accumulator::add};
    CORRADE_VERIFY(a);
    CORRADE_VERIFY(!a.isAllocated());

    CORRADE_COMPARE(a(2), 15);
    /* The function is const, the member is thus unaffected */
    CORRADE_COMPARE(derived.a, 13);
    CORRADE_COMPARE(derived.b, 26);

    /* Member function pointers usually have a size of 2 pointers, except for
       non-MinGW Windows where they're just one pointer */
    #if !defined(CORRADE_TARGET_WINDOWS) || defined(CORRADE_TARGET_MINGW)
    CORRADE_COMPARE(sizeof(&Derived::add), 2*sizeof(std::size_t));
    #else
    CORRADE_COMPARE(sizeof(&Derived::add), sizeof(std::size_t));
    #endif

    /* Uncomment to verify that it's not possible to wrap a function in a
       subclass of the instance. It causes an error inside a lambda, so it's
       not possible to verify with is_constructible. */
    #if 0
    struct Derived2: Derived {
        int sub(int value) const {
            return a - value;
        }
    };

    Function<int(int)> b{derived, &Derived2::sub};
    #endif
}

void FunctionTest::constructMemberFunctionInBaseConstLvalue() {
    struct Accumulator {
        int add(int value) const & {
            return a + value;
        }

        int a = 13;
    };

    struct Derived: Accumulator {
        int b = 26;
    } derived;

    Function<int(int)> a{derived, &Accumulator::add};
    CORRADE_VERIFY(a);
    CORRADE_VERIFY(!a.isAllocated());

    CORRADE_COMPARE(a(2), 15);
    /* The function is const, the member is thus unaffected */
    CORRADE_COMPARE(derived.a, 13);
    CORRADE_COMPARE(derived.b, 26);

    /* Member function pointers usually have a size of 2 pointers, except for
       non-MinGW Windows where they're just one pointer */
    #if !defined(CORRADE_TARGET_WINDOWS) || defined(CORRADE_TARGET_MINGW)
    CORRADE_COMPARE(sizeof(&Derived::add), 2*sizeof(std::size_t));
    #else
    CORRADE_COMPARE(sizeof(&Derived::add), sizeof(std::size_t));
    #endif

    /* Uncomment to verify that it's not possible to wrap a function in a
       subclass of the instance. It causes an error inside a lambda, so it's
       not possible to verify with is_constructible. */
    #if 0
    struct Derived2: Derived {
        int sub(int value) const & {
            return a - value;
        }
    };

    Function<int(int)> b{derived, &Derived2::sub};
    #endif
}

void FunctionTest::constructMemberFunctionMultipleInheritance() {
    struct First {
        int b = 26;
    };

    struct Accumulator {
        int a = 13;
    };

    struct Derived: First, Accumulator {
        int add(int value) {
            a += value;
            return a;
        }

        int a = 13;
    } derived;

    Function<int(int)> a{derived, &Derived::add};
    CORRADE_VERIFY(a);
    CORRADE_VERIFY(!a.isAllocated());

    CORRADE_COMPARE(a(2), 15);
    CORRADE_COMPARE(derived.a, 15);
    CORRADE_COMPARE(derived.b, 26);

    /* Member function pointers with multiple inheritance have a size of 2
       pointers */
    CORRADE_COMPARE(sizeof(&Derived::add), 2*sizeof(std::size_t));
}

void FunctionTest::constructMemberFunctionMultipleVirtualInheritance() {
    /* Same as constructMemberFunctionMultipleInheritance(), just that the base
       is inherited virtually */

    struct First {
        int b = 26;
    };

    struct Accumulator {
        int a = 13;
    };

    struct Derived: First, virtual Accumulator {
        int add(int value) {
            a += value;
            return a;
        }

        int a = 13;
    } derived;

    Function<int(int)> a{derived, &Derived::add};
    CORRADE_VERIFY(a);
    CORRADE_VERIFY(!a.isAllocated());

    CORRADE_COMPARE(a(2), 15);
    CORRADE_COMPARE(derived.a, 15);
    CORRADE_COMPARE(derived.b, 26);

    /* Member function pointers have a size of 2 pointers, except for virtual
       inheritance on Windows where it is 12 / 16 bytes on both 32 / 64 bits.
       This is also the maximum to which the internal constant is scaled. */
    #ifndef CORRADE_TARGET_WINDOWS
    CORRADE_COMPARE(sizeof(&Derived::add), 2*sizeof(std::size_t));
    #elif defined(CORRADE_TARGET_32BIT)
    CORRADE_COMPARE(sizeof(&Derived::add), 12);
    #else
    CORRADE_COMPARE(sizeof(&Derived::add), 16);
    #endif
    CORRADE_COMPARE(sizeof(&Derived::add), Implementation::FunctionPointerSize*sizeof(std::size_t));
}

void FunctionTest::constructStatelessFunctor() {
    struct {
        int operator()(int a, int b) {
            return a + b;
        }
    } sum;

    Function<int(int, int)> a = sum;
    Function<int(int, int)> b{NoAllocateInit, sum};
    CORRADE_VERIFY(a);
    CORRADE_VERIFY(b);
    CORRADE_VERIFY(!a.isAllocated());
    CORRADE_VERIFY(!b.isAllocated());

    CORRADE_COMPARE(a(3, 5), 8);
    CORRADE_COMPARE(b(3, 5), 8);
}

void FunctionTest::constructStatelessLambda() {
    auto sum = [](int a, int b) { return a + b; };

    Function<int(int, int)> a = sum;
    /* Passing lambdas convertible to function pointers to the NoAllocateInit
       overload is not allowed */
    CORRADE_VERIFY(!std::is_constructible<Function<int(int, int)>, NoAllocateInitT, decltype(sum)>::value);
    CORRADE_VERIFY(a);
    CORRADE_VERIFY(!a.isAllocated());

    CORRADE_COMPARE(a(3, 5), 8);
}

void FunctionTest::constructStatefulTrivialSmallFunctor() {
    struct Accumulator {
        int operator()(int value) {
            a += value;
            return a;
        }

        int a = 13;
    } accumulator;

    #ifndef CORRADE_NO_STD_IS_TRIVIALLY_TRAITS
    CORRADE_VERIFY(std::is_trivially_copyable<Accumulator>::value);
    #else
    CORRADE_VERIFY(__has_trivial_copy(Accumulator));
    #endif
    CORRADE_VERIFY(std::is_trivially_destructible<Accumulator>::value);

    Function<int(int)> a = accumulator;
    Function<int(int)> b{NoAllocateInit, accumulator};
    CORRADE_VERIFY(a);
    CORRADE_VERIFY(b);
    CORRADE_VERIFY(!a.isAllocated());
    CORRADE_VERIFY(!b.isAllocated());

    CORRADE_COMPARE(a(2), 15);
    CORRADE_COMPARE(b(-7), 6);
    /* The functor gets copied, the original instance is thus unaffected */
    CORRADE_COMPARE(accumulator.a, 13);
}

void FunctionTest::constructStatefulTrivialSmallLambda() {
    int accumulatorA = 13;
    auto accumulator = [&accumulatorA](int value) {
        accumulatorA += value;
        return accumulatorA;
    };

    {
        #ifdef CORRADE_MSVC2017_COMPATIBILITY
        CORRADE_EXPECT_FAIL("All lambdas are non-trivially-copyable on MSVC 2015 and 2017.");
        #endif
        #ifndef CORRADE_NO_STD_IS_TRIVIALLY_TRAITS
        CORRADE_VERIFY(std::is_trivially_copyable<decltype(accumulator)>::value);
        #else
        CORRADE_VERIFY(__has_trivial_copy(decltype(accumulator)));
        #endif
    }
    CORRADE_VERIFY(std::is_trivially_destructible<decltype(accumulator)>::value);

    Function<int(int)> a = accumulator;
    Function<int(int)> b{
        /* All lambdas are non-trivially-copyable on MSVC 2015 and 2017, so
           this would fail to compile there */
        #ifndef CORRADE_MSVC2017_COMPATIBILITY
        NoAllocateInit,
        #endif
        accumulator};
    CORRADE_VERIFY(a);
    CORRADE_VERIFY(b);
    {
        #ifdef CORRADE_MSVC2017_COMPATIBILITY
        CORRADE_EXPECT_FAIL("All lambdas are non-trivially-copyable on MSVC 2015 and 2017.");
        #endif
        CORRADE_VERIFY(!a.isAllocated());
        CORRADE_VERIFY(!b.isAllocated());
    }

    CORRADE_COMPARE(a(2), 15);
    CORRADE_COMPARE(accumulatorA, 15);

    CORRADE_COMPARE(b(-7), 8);
    CORRADE_COMPARE(accumulatorA, 8);
}

void FunctionTest::constructStatefulTrivialLargeFunctor() {
    struct Accumulator {
        int operator()(int value) {
            a[1] += value;
            return a[1];
        }

        /* Up to 3 pointers on 64bit and up to 4 on 32bit can fit inline, 5
           pointers will allocate */
        std::size_t a[5]{0, 13, 2, 1};
    } accumulator;

    #ifndef CORRADE_NO_STD_IS_TRIVIALLY_TRAITS
    CORRADE_VERIFY(std::is_trivially_copyable<Accumulator>::value);
    #else
    CORRADE_VERIFY(__has_trivial_copy(Accumulator));
    #endif
    CORRADE_VERIFY(std::is_trivially_destructible<Accumulator>::value);

    Function<int(int)> a = accumulator;
    CORRADE_VERIFY(a);
    CORRADE_VERIFY(a.isAllocated());

    CORRADE_COMPARE(a(2), 15);
    CORRADE_COMPARE(a(-7), 8);
    /* The functor gets copied, the original instance is thus unaffected */
    CORRADE_COMPARE(accumulator.a[1], 13);

    /* Uncomment this to test that the NoAllocateInit variant fails here. It
       can't be tested with is_constructible, because the check is a
       static_assert() inside. */
    #if 0
    Function<int(int)> b{NoAllocateInit, accumulator};
    #endif
}

void FunctionTest::constructStatefulSmallFunctor() {
    struct Accumulator {
        Containers::StringView operator()(Containers::StringView value) {
            a = a + value;
            return a;
        }

        Containers::String a = "hello";
    } accumulator;

    CORRADE_VERIFY(std::is_copy_constructible<Accumulator>::value);
    #ifndef CORRADE_NO_STD_IS_TRIVIALLY_TRAITS
    CORRADE_VERIFY(!std::is_trivially_copyable<Accumulator>::value);
    #else
    CORRADE_VERIFY(!__has_trivial_copy(Accumulator));
    #endif
    CORRADE_VERIFY(!std::is_trivially_destructible<Accumulator>::value);

    Function<Containers::StringView(Containers::StringView)> a = accumulator;
    CORRADE_VERIFY(a);
    CORRADE_VERIFY(a.isAllocated());

    CORRADE_COMPARE(a("!!"), "hello!!");
    CORRADE_COMPARE(a("?"), "hello!!?");
    /* The functor gets copied, the original instance is thus unaffected */
    CORRADE_COMPARE(accumulator.a, "hello");

    /* Uncomment this to test that the NoAllocateInit variant fails here. It
       can't be tested with is_constructible, because the check is a
       static_assert() inside. */
    #if 0
    Function<int(int)> b{NoAllocateInit, accumulator};
    #endif
}

void FunctionTest::constructStatefulLargeFunctor() {
    struct Accumulator {
        Containers::StringView operator()(Containers::StringView value) {
            a[1] = a[1] + value;
            return a[1];
        }

        Containers::String a[2]{
            "", "hello"
        };
    } accumulator;

    CORRADE_VERIFY(std::is_copy_constructible<Accumulator>::value);
    #ifndef CORRADE_NO_STD_IS_TRIVIALLY_TRAITS
    CORRADE_VERIFY(!std::is_trivially_copyable<Accumulator>::value);
    #else
    CORRADE_VERIFY(!__has_trivial_copy(Accumulator));
    #endif
    CORRADE_VERIFY(!std::is_trivially_destructible<Accumulator>::value);

    Function<Containers::StringView(Containers::StringView)> a = Accumulator{};
    CORRADE_VERIFY(a);
    CORRADE_VERIFY(a.isAllocated());

    CORRADE_COMPARE(a("!!"), "hello!!");
    CORRADE_COMPARE(a("?"), "hello!!?");
    /* The functor gets copied, the original instance is thus unaffected */
    CORRADE_COMPARE(accumulator.a[1], "hello");

    /* Uncomment this to test that the NoAllocateInit variant fails here. It
       can't be tested with is_constructible, because the check is a
       static_assert() inside. */
    #if 0
    Function<int(int)> b{NoAllocateInit, accumulator};
    #endif
}

void FunctionTest::constructMoveOnlyFunctor() {
    CORRADE_VERIFY(!std::is_copy_constructible<MoveOnlyAccumulator>::value);
    CORRADE_VERIFY(!std::is_trivially_destructible<MoveOnlyAccumulator>::value);

    {
        Function<int(int)> a = MoveOnlyAccumulator{};
        CORRADE_VERIFY(a);
        CORRADE_VERIFY(a.isAllocated());

        CORRADE_COMPARE(a(2), 15);
        CORRADE_COMPARE(a(-7), 8);

        /* 1 temporary that was moved once */
        CORRADE_COMPARE(MoveOnlyAccumulator::constructed, 2);
        CORRADE_COMPARE(MoveOnlyAccumulator::destructed, 1);
        CORRADE_COMPARE(MoveOnlyAccumulator::moved, 1);
    }

    CORRADE_COMPARE(MoveOnlyAccumulator::constructed, 2);
    CORRADE_COMPARE(MoveOnlyAccumulator::destructed, 2);
    CORRADE_COMPARE(MoveOnlyAccumulator::moved, 1);

    /* Uncomment this to test that the NoAllocateInit variant fails here. It
       can't be tested with is_constructible, because the check is a
       static_assert() inside. */
    #if 0
    Function<int(int)> b{NoAllocateInit, MoveOnlyAccumulator{}};
    #endif
}

void FunctionTest::constructTriviallyCopyableMoveOnlyFunctor() {
    #ifndef CORRADE_NO_STD_IS_TRIVIALLY_TRAITS
    {
        #ifdef CORRADE_MSVC2017_COMPATIBILITY
        CORRADE_EXPECT_FAIL("MSVC 2015 and 2017 has a different std::is_trivially_copyable behavior.");
        #endif
        CORRADE_VERIFY(std::is_trivially_copyable<TriviallyCopyableMoveOnlyAccumulator>::value);
    }
    CORRADE_VERIFY(!std::is_trivially_copy_constructible<TriviallyCopyableMoveOnlyAccumulator>::value);
    #else
    {
        CORRADE_EXPECT_FAIL("GCC 4.8 __has_trivial_copy() has a different behavior than std::is_trivially_copyable.");
        CORRADE_VERIFY(__has_trivial_copy(TriviallyCopyableMoveOnlyAccumulator));
    }
    #endif

    Function<int(int)> a = TriviallyCopyableMoveOnlyAccumulator{};
    CORRADE_VERIFY(a);

    {
        #ifdef CORRADE_NO_STD_IS_TRIVIALLY_TRAITS
        CORRADE_EXPECT_FAIL("GCC 4.8 __has_trivial_copy() has a different behavior than std::is_trivially_copyable.");
        #elif defined(CORRADE_MSVC2017_COMPATIBILITY)
        CORRADE_EXPECT_FAIL("MSVC 2015 and 2017 has a different std::is_trivially_copyable behavior.");
        #endif
        CORRADE_VERIFY(!a.isAllocated());
    }
    CORRADE_COMPARE(a(2), 15);

    /* With __has_trivial_copy() and MSVC 2015 / 2017 this would assert because
       it's not deemed trivially copyable */
    #if !defined(CORRADE_NO_STD_IS_TRIVIALLY_TRAITS) && !defined(CORRADE_MSVC2017_COMPATIBILITY)
    Function<int(int)> b{NoAllocateInit, TriviallyCopyableMoveOnlyAccumulator{}};
    CORRADE_VERIFY(b);
    CORRADE_VERIFY(!b.isAllocated());
    CORRADE_COMPARE(b(-7), 6);
    #endif
}

void FunctionTest::constructNonTriviallyDestructibleFunctor() {
    /* The type not being trivially destructible implies it's also not
       trivially copyable (with std::is_trivially_copyable at least, not with
       the __has_trivial_copy() tho). The other direction doesn't hold however,
       see the constructNonTriviallyCopyableFunctor() test below. */
    #ifndef CORRADE_NO_STD_IS_TRIVIALLY_TRAITS
    CORRADE_VERIFY(!std::is_trivially_copyable<NonTriviallyDestructibleAccumulator>::value);
    #else
    CORRADE_VERIFY(__has_trivial_copy(NonTriviallyDestructibleAccumulator));
    #endif
    CORRADE_VERIFY(!std::is_trivially_destructible<NonTriviallyDestructibleAccumulator>::value);

    NonTriviallyDestructibleAccumulator accumulator;
    {
        Function<int(int)> a = accumulator;
        CORRADE_VERIFY(a);
        CORRADE_VERIFY(a.isAllocated());

        CORRADE_COMPARE(a(2), 15);
        CORRADE_COMPARE(a(-7), 8);
        /* The functor gets copied, the original instance is thus unaffected */
        CORRADE_COMPARE(accumulator.a, 13);

        /* 1 instance that was copied once, with an implicitly generated copy
           constructor */
        CORRADE_COMPARE(NonTriviallyDestructibleAccumulator::constructed, 1);
        CORRADE_COMPARE(NonTriviallyDestructibleAccumulator::destructed, 0);
    }

    CORRADE_COMPARE(NonTriviallyDestructibleAccumulator::constructed, 1);
    CORRADE_COMPARE(NonTriviallyDestructibleAccumulator::destructed, 1);

    /* Uncomment this to test that the NoAllocateInit variant fails here. It
       can't be tested with is_constructible, because the check is a
       static_assert() inside. */
    #if 0
    Function<int(int)> b{NoAllocateInit, accumulator};
    #endif
}

void FunctionTest::constructNonTriviallyDestructibleFunctorData() {
    /* Like constructNonTriviallyDestructibleFunctor(), but verifying that the
       destruction happens at the right placewhen the object is sliced to the
       FunctionData base */

    NonTriviallyDestructibleAccumulator accumulator;
    {
        FunctionData a = Function<int(int)>{accumulator};
        CORRADE_VERIFY(static_cast<const Function<int(int)>&>(a));
        CORRADE_VERIFY(static_cast<const Function<int(int)>&>(a).isAllocated());

        CORRADE_COMPARE(static_cast<Function<int(int)>&>(a)(2), 15);
        CORRADE_COMPARE(static_cast<Function<int(int)>&>(a)(-7), 8);
        /* The functor gets copied, the original instance is thus unaffected */
        CORRADE_COMPARE(accumulator.a, 13);

        /* 1 instance that was copied once, with an implicitly generated copy
           constructor */
        CORRADE_COMPARE(NonTriviallyDestructibleAccumulator::constructed, 1);
        CORRADE_COMPARE(NonTriviallyDestructibleAccumulator::destructed, 0);
    }

    CORRADE_COMPARE(NonTriviallyDestructibleAccumulator::constructed, 1);
    CORRADE_COMPARE(NonTriviallyDestructibleAccumulator::destructed, 1);
}

void FunctionTest::constructNonTriviallyCopyableFunctor() {
    /* It isn't trivially copyable, but is trivially destructible. The class
       shouldn't confuse the two and still allocate it on heap. */
    #ifndef CORRADE_NO_STD_IS_TRIVIALLY_TRAITS
    CORRADE_VERIFY(!std::is_trivially_copyable<NonTriviallyCopyableAccumulator>::value);
    #else
    CORRADE_VERIFY(!__has_trivial_copy(NonTriviallyCopyableAccumulator));
    #endif
    CORRADE_VERIFY(std::is_trivially_destructible<NonTriviallyCopyableAccumulator>::value);

    NonTriviallyCopyableAccumulator accumulator;
    {
        Function<int(int)> a = accumulator;
        CORRADE_VERIFY(a);
        CORRADE_VERIFY(a.isAllocated());

        CORRADE_COMPARE(a(2), 15);
        CORRADE_COMPARE(a(-7), 8);
        /* The functor gets copied, the original instance is thus unaffected */
        CORRADE_COMPARE(accumulator.a, 13);

        /* 1 instance that was copied once */
        CORRADE_COMPARE(NonTriviallyCopyableAccumulator::constructed, 2);
        CORRADE_COMPARE(NonTriviallyCopyableAccumulator::copied, 1);
        CORRADE_COMPARE(NonTriviallyCopyableAccumulator::moved, 0);
    }

    CORRADE_COMPARE(NonTriviallyCopyableAccumulator::constructed, 2);
    CORRADE_COMPARE(NonTriviallyCopyableAccumulator::copied, 1);
    CORRADE_COMPARE(NonTriviallyCopyableAccumulator::moved, 0);

    /* Uncomment this to test that the NoAllocateInit variant fails here. It
       can't be tested with is_constructible, because the check is a
       static_assert() inside. */
    #if 0
    Function<int(int)> b{NoAllocateInit, accumulator};
    #endif
}

void FunctionTest::constructTrivialFunctorPlainStruct() {
    struct ExtremelyTrivial {
        int a;
        char c;

        int operator()() { return 3; }
    };

    /* This needs special handling on GCC 4.8, where T{b} (copy-construction)
       attempts to convert ExtremelyTrivial to int to initialize the first
       argument and fails miserably. */
    Function<int()> a = ExtremelyTrivial{};
    CORRADE_VERIFY(a);
    CORRADE_VERIFY(!a.isAllocated());
    CORRADE_COMPARE(a(), 3);
}

void FunctionTest::constructFunctorPlainStruct() {
    struct MoveOnlyStruct {
        int a;
        char c;
        Pointer<int> b;

        int operator()() { return 3; }
    };

    /* This needs special handling on GCC 4.8, where T{Utility::move(b)}
       attempts to convert MoveOnlyStruct to int to initialize the first
       argument and fails miserably. */
    Function<int()> a = MoveOnlyStruct{};
    CORRADE_VERIFY(a);
    CORRADE_VERIFY(a.isAllocated());
    CORRADE_COMPARE(a(), 3);
}

void FunctionTest::constructTrivialFunctorOverload() {
    /* If a functor has multiple overloads, it picks one that matches the
       signature */

    struct {
        int operator()(int a, int b) {
            return a + b;
        }
        float operator()(float a, float b) {
            return a * b;
        }
    } sumOrMultiply;

    Function<int(int, int)> a1 = sumOrMultiply;
    Function<int(int, int)> a2{NoAllocateInit, sumOrMultiply};
    Function<float(float, float)> b1 = sumOrMultiply;
    Function<float(float, float)> b2{NoAllocateInit, sumOrMultiply};
    CORRADE_VERIFY(!a1.isAllocated());
    CORRADE_VERIFY(!a2.isAllocated());
    CORRADE_VERIFY(!b1.isAllocated());
    CORRADE_VERIFY(!b2.isAllocated());
    CORRADE_COMPARE(a1(3, 5), 8);
    CORRADE_COMPARE(a2(3, 5), 8);
    CORRADE_COMPARE(b1(3.5f, 5.0f), 17.5f);
    CORRADE_COMPARE(b2(3.5f, 5.0f), 17.5f);
}

void FunctionTest::constructFunctorOverload() {
    /* Like moveOnlyResultTrivialFunctor(), just with the non-trivial
       destructor added */

    struct SumOrMultiply {
        /* To make it non-trivially-copyable */
        ~SumOrMultiply() {}

        int operator()(int a, int b) {
            return a + b;
        }
        float operator()(float a, float b) {
            return a * b;
        }
    } sumOrMultiply;

    Function<int(int, int)> a = sumOrMultiply;
    Function<float(float, float)> b = sumOrMultiply;
    CORRADE_VERIFY(a.isAllocated());
    CORRADE_VERIFY(b.isAllocated());
    CORRADE_COMPARE(a(3, 5), 8);
    CORRADE_COMPARE(b(3.0f, 5.0f), 15.0f);
}

void FunctionTest::constructTrivialFunctorRvalueOverload() {
    struct {
        int operator()(int a, int b) {
            return a + b;
        }
    } sum;
    struct {
        int operator()(int a, int b) & {
            return a + b;
        }
    } sumLvalue;
    struct {
        int operator()(int a, int b) const {
            return a + b;
        }
    } sumConst;
    struct {
        int operator()(int a, int b) const & {
            return a + b;
        }
    } sumConstLvalue;

    Function<int(int, int)> a1 = sum;
    Function<int(int, int)> a2{NoAllocateInit, sum};
    Function<int(int, int)> b1 = sumLvalue;
    Function<int(int, int)> b2{NoAllocateInit, sumLvalue};
    Function<int(int, int)> c1 = sumConst;
    Function<int(int, int)> c2{NoAllocateInit, sumConst};
    Function<int(int, int)> d1 = sumConstLvalue;
    Function<int(int, int)> d2{NoAllocateInit, sumConstLvalue};
    CORRADE_VERIFY(!a1.isAllocated());
    CORRADE_VERIFY(!a2.isAllocated());
    CORRADE_VERIFY(!b1.isAllocated());
    CORRADE_VERIFY(!b2.isAllocated());
    CORRADE_VERIFY(!c1.isAllocated());
    CORRADE_VERIFY(!c2.isAllocated());
    CORRADE_VERIFY(!d1.isAllocated());
    CORRADE_VERIFY(!d2.isAllocated());
    CORRADE_COMPARE(a1(3, 5), 8);
    CORRADE_COMPARE(a2(3, 5), 8);
    CORRADE_COMPARE(b1(3, 5), 8);
    CORRADE_COMPARE(b2(3, 5), 8);
    CORRADE_COMPARE(c1(3, 5), 8);
    CORRADE_COMPARE(c2(3, 5), 8);
    CORRADE_COMPARE(d1(3, 5), 8);
    CORRADE_COMPARE(d2(3, 5), 8);
}

void FunctionTest::constructFunctorRvalueOverload() {
    /* Like constructTrivialFunctorRvalueOverload(), just with the non-trivial
       destructors added */

    struct Sum {
        /* To make it non-trivially-copyable */
        ~Sum() {}

        int operator()(int a, int b) {
            return a + b;
        }
    } sum;

    struct SumLvalue {
        /* To make it non-trivially-copyable */
        ~SumLvalue() {}

        int operator()(int a, int b) & {
            return a + b;
        }
    } sumLvalue;

    struct SumConst {
        /* To make it non-trivially-copyable */
        ~SumConst() {}

        int operator()(int a, int b) const {
            return a + b;
        }
    } sumConst;

    struct SumConstLvalue {
        /* To make it non-trivially-copyable */
        ~SumConstLvalue() {}

        int operator()(int a, int b) const & {
            return a + b;
        }
    } sumConstLvalue;

    Function<int(int, int)> a = sum;
    Function<int(int, int)> b = sumLvalue;
    Function<int(int, int)> c = sumConst;
    Function<int(int, int)> d = sumConstLvalue;
    CORRADE_VERIFY(a.isAllocated());
    CORRADE_VERIFY(b.isAllocated());
    CORRADE_VERIFY(c.isAllocated());
    CORRADE_VERIFY(d.isAllocated());
    CORRADE_COMPARE(a(3, 5), 8);
    CORRADE_COMPARE(b(3, 5), 8);
    CORRADE_COMPARE(c(3, 5), 8);
    CORRADE_COMPARE(d(3, 5), 8);
}

void FunctionTest::constructCopy() {
    CORRADE_VERIFY(!std::is_copy_constructible<Function<int(int, int)>>::value);
    CORRADE_VERIFY(!std::is_copy_assignable<Function<int(int, int)>>::value);
}

void FunctionTest::constructCopyData() {
    CORRADE_VERIFY(!std::is_copy_constructible<FunctionData>::value);
    CORRADE_VERIFY(!std::is_copy_assignable<FunctionData>::value);
}

void FunctionTest::constructMove() {
    {
        Function<int(int)> a = MoveOnlyAccumulator{};
        CORRADE_VERIFY(a);
        CORRADE_VERIFY(a.isAllocated());
        CORRADE_COMPARE(a(3), 16);

        /* It should be constructed just once as a temporary, then moved and
           then not moved again after at all */
        CORRADE_COMPARE(MoveOnlyAccumulator::constructed, 2);
        CORRADE_COMPARE(MoveOnlyAccumulator::moved, 1);
        CORRADE_COMPARE(MoveOnlyAccumulator::destructed, 1);

        Function<int(int)> b = Utility::move(a);
        CORRADE_VERIFY(!a);
        CORRADE_VERIFY(b);
        CORRADE_VERIFY(b.isAllocated());
        CORRADE_COMPARE(b(-7), 9);

        Function<int(int)> c = [](int a) { return a; };
        CORRADE_VERIFY(c);
        CORRADE_VERIFY(!c.isAllocated());

        c = Utility::move(b);
        CORRADE_VERIFY(c);
        CORRADE_VERIFY(c.isAllocated());
        CORRADE_COMPARE(c(22), 31);
    }

    CORRADE_COMPARE(MoveOnlyAccumulator::constructed, 2);
    CORRADE_COMPARE(MoveOnlyAccumulator::moved, 1);
    CORRADE_COMPARE(MoveOnlyAccumulator::destructed, 2);

    CORRADE_VERIFY(std::is_nothrow_move_constructible<Function<int(int)>>::value);
    CORRADE_VERIFY(std::is_nothrow_move_assignable<Function<int(int)>>::value);
    CORRADE_VERIFY(std::is_nothrow_move_constructible<Function<int(int)>>::value);
    CORRADE_VERIFY(std::is_nothrow_move_assignable<Function<int(int)>>::value);
}

void FunctionTest::constructMoveData() {
    /* Like constructMove(), but the type being saved is the FunctionData base.
       It should still work when cast back, and it should destruct at the end
       as well. */

    {
        FunctionData a = Function<int(int)>{MoveOnlyAccumulator{}};
        CORRADE_VERIFY(static_cast<const Function<int(int)>&>(a));
        CORRADE_VERIFY(static_cast<const Function<int(int)>&>(a).isAllocated());
        CORRADE_COMPARE(static_cast<Function<int(int)>&>(a)(3), 16);

        /* It should be constructed just once as a temporary, then moved and
           then not moved again after at all */
        CORRADE_COMPARE(MoveOnlyAccumulator::constructed, 2);
        CORRADE_COMPARE(MoveOnlyAccumulator::moved, 1);
        CORRADE_COMPARE(MoveOnlyAccumulator::destructed, 1);

        FunctionData b = Utility::move(a);
        CORRADE_VERIFY(!static_cast<const Function<int(int)>&>(a));
        CORRADE_VERIFY(static_cast<const Function<int(int)>&>(b));
        CORRADE_VERIFY(static_cast<const Function<int(int)>&>(b).isAllocated());
        CORRADE_COMPARE(static_cast<Function<int(int)>&>(b)(-7), 9);

        FunctionData c = Function<int(int)>{[](int a) { return a; }};
        CORRADE_VERIFY(static_cast<const Function<int(int)>&>(c));
        CORRADE_VERIFY(!static_cast<const Function<int(int)>&>(c).isAllocated());

        c = Utility::move(b);
        CORRADE_VERIFY(static_cast<const Function<int(int)>&>(c));
        CORRADE_VERIFY(static_cast<const Function<int(int)>&>(c).isAllocated());
        CORRADE_COMPARE(static_cast<Function<int(int)>&>(c)(22), 31);
    }

    CORRADE_COMPARE(MoveOnlyAccumulator::constructed, 2);
    CORRADE_COMPARE(MoveOnlyAccumulator::moved, 1);
    CORRADE_COMPARE(MoveOnlyAccumulator::destructed, 2);

    CORRADE_VERIFY(std::is_nothrow_move_constructible<FunctionData>::value);
    CORRADE_VERIFY(std::is_nothrow_move_assignable<FunctionData>::value);
    CORRADE_VERIFY(std::is_nothrow_move_constructible<FunctionData>::value);
    CORRADE_VERIFY(std::is_nothrow_move_assignable<FunctionData>::value);
}

void FunctionTest::implicitlyConvertibleArgumentFunction() {
    CORRADE_VERIFY(std::is_constructible<Function<int(int, int)>, int(int, int)>::value);
    CORRADE_VERIFY(std::is_constructible<Function<int(int, int)>, int(*)(int, int)>::value);
    /* Passing functions / function pointers to the NoAllocateInit overload
       shouldn't be possible even if they match */
    CORRADE_VERIFY(!std::is_constructible<Function<int(int, int)>, NoAllocateInitT, int(int, int)>::value);
    CORRADE_VERIFY(!std::is_constructible<Function<int(int, int)>, NoAllocateInitT, int(*)(int, int)>::value);

    CORRADE_VERIFY(!std::is_constructible<Function<int(int, int)>, int(int, float)>::value);
    CORRADE_VERIFY(!std::is_constructible<Function<int(int, int)>, int(*)(float, int)>::value);
    /* Passing different functions / function pointers to the NoAllocateInit
       overload shouldn't be possible either */
    CORRADE_VERIFY(!std::is_constructible<Function<int(int, int)>, NoAllocateInitT, int(float, int)>::value);
    CORRADE_VERIFY(!std::is_constructible<Function<int(int, int)>, NoAllocateInitT, int(*)(int, float)>::value);
}

void FunctionTest::implicitlyConvertibleArgumentMemberFunction() {
    struct Functor {};

    CORRADE_VERIFY(std::is_constructible<Function<int(int, int)>, Functor&, int(Functor::*)(int, int)>::value);
    CORRADE_VERIFY(std::is_constructible<Function<int(int, int)>, Functor&, int(Functor::*)(int, int) &>::value);
    CORRADE_VERIFY(std::is_constructible<Function<int(int, int)>, Functor&, int(Functor::*)(int, int) const>::value);
    CORRADE_VERIFY(std::is_constructible<Function<int(int, int)>, Functor&, int(Functor::*)(int, int) const &>::value);

    CORRADE_VERIFY(!std::is_constructible<Function<int(int, int)>, Functor&, int(Functor::*)(float, int)>::value);
    CORRADE_VERIFY(!std::is_constructible<Function<int(int, int)>, Functor&, int(Functor::*)(float, int) &>::value);
    CORRADE_VERIFY(!std::is_constructible<Function<int(int, int)>, Functor&, int(Functor::*)(float, int) const>::value);
    CORRADE_VERIFY(!std::is_constructible<Function<int(int, int)>, Functor&, int(Functor::*)(float, int) const &>::value);
}

void FunctionTest::implicitlyConvertibleArgumentStatelessFunctor() {
    auto sum = [](int a, int b) {
        return a + b;
    };

    CORRADE_VERIFY(!std::is_constructible<Function<int(float, int)>, decltype(sum)>::value);
    CORRADE_VERIFY(!std::is_constructible<Function<int(int, float)>, NoAllocateInitT, decltype(sum)>::value);
}

void FunctionTest::implicitlyConvertibleArgumentTrivialFunctor() {
    /* Like implicitlyConvertibleArgumentStatelessFunctor(), just with the
       lambda converted to a function struct */

    struct {
        int operator()(int a, int b) {
            return a + b;
        }
    } sum;

    CORRADE_VERIFY(!std::is_constructible<Function<int(float, int)>, decltype(sum)>::value);
    CORRADE_VERIFY(!std::is_constructible<Function<int(int, float)>, NoAllocateInitT, decltype(sum)>::value);
}

void FunctionTest::implicitlyConvertibleArgumentFunctor() {
    /* Like implicitlyConvertibleArgumentTrivialFunctor(), just with the
       non-trivial destructor added */

    struct Sum {
        /* To make it non-trivially-copyable */
        ~Sum() {}

        int operator()(int a, int b) {
            return a + b;
        }
    } sum;

    CORRADE_VERIFY(!std::is_constructible<Function<int(float, int)>, decltype(sum)>::value);
    CORRADE_VERIFY(!std::is_constructible<Function<int(int, float)>, decltype(sum)>::value);
}

void FunctionTest::implicitlyConvertibleResultFunction() {
    CORRADE_VERIFY(std::is_constructible<Function<int(int, int)>, int(int, int)>::value);
    CORRADE_VERIFY(std::is_constructible<Function<int(int, int)>, int(*)(int, int)>::value);
    /* Passing functions / function pointers to the NoAllocateInit overload
       shouldn't be possible even if they match */
    CORRADE_VERIFY(!std::is_constructible<Function<int(int, int)>, NoAllocateInitT, int(int, int)>::value);
    CORRADE_VERIFY(!std::is_constructible<Function<int(int, int)>, NoAllocateInitT, int(*)(int, int)>::value);

    CORRADE_VERIFY(!std::is_constructible<Function<int(int, int)>, float(int, int)>::value);
    CORRADE_VERIFY(!std::is_constructible<Function<int(int, int)>, float(*)(int, int)>::value);
    /* Passing different functions / function pointers to the NoAllocateInit
       overload shouldn't be possible either */
    CORRADE_VERIFY(!std::is_constructible<Function<int(int, int)>, NoAllocateInitT, float(int, int)>::value);
    CORRADE_VERIFY(!std::is_constructible<Function<int(int, int)>, NoAllocateInitT, float(*)(int, int)>::value);
}

void FunctionTest::implicitlyConvertibleResultMemberFunction() {
    struct Functor {};

    CORRADE_VERIFY(std::is_constructible<Function<int(int, int)>, Functor&, int(Functor::*)(int, int)>::value);
    CORRADE_VERIFY(std::is_constructible<Function<int(int, int)>, Functor&, int(Functor::*)(int, int) &>::value);
    CORRADE_VERIFY(std::is_constructible<Function<int(int, int)>, Functor&, int(Functor::*)(int, int) const>::value);
    CORRADE_VERIFY(std::is_constructible<Function<int(int, int)>, Functor&, int(Functor::*)(int, int) const &>::value);

    CORRADE_VERIFY(!std::is_constructible<Function<int(int, int)>, Functor&, float(Functor::*)(int, int)>::value);
    CORRADE_VERIFY(!std::is_constructible<Function<int(int, int)>, Functor&, float(Functor::*)(int, int) &>::value);
    CORRADE_VERIFY(!std::is_constructible<Function<int(int, int)>, Functor&, float(Functor::*)(int, int) const>::value);
    CORRADE_VERIFY(!std::is_constructible<Function<int(int, int)>, Functor&, float(Functor::*)(int, int) const &>::value);
}

void FunctionTest::implicitlyConvertibleResultStatelessFunctor() {
    auto divide = [](int a, int b) {
        return float(a)/b;
    };

    CORRADE_VERIFY(!std::is_constructible<Function<int(int, int)>, decltype(divide)>::value);
    CORRADE_VERIFY(!std::is_constructible<Function<int(int, int)>, NoAllocateInitT, decltype(divide)>::value);
}

void FunctionTest::implicitlyConvertibleResultTrivialFunctor() {
    /* Like implicitlyConvertibleResultStatelessFunctor(), just with the
       lambda converted to a function struct */

    struct {
        float operator()(int a, int b) {
            return float(a)/b;
        }
    } divide;

    CORRADE_VERIFY(!std::is_constructible<Function<int(int, int)>, decltype(divide)>::value);
    CORRADE_VERIFY(!std::is_constructible<Function<int(int, int)>, NoAllocateInitT, decltype(divide)>::value);
}

void FunctionTest::implicitlyConvertibleResultFunctor() {
    /* Like the second half od implicitlyConvertibleResultTrivialFunctor(),
       just with the non-trivial destructor added */

    struct Divide {
        /* To make it non-trivially-copyable */
        ~Divide() {}

        float operator()(int a, int b) {
            return float(a)/b;
        }
    } divide;

    CORRADE_VERIFY(!std::is_constructible<Function<int(int, int)>, decltype(divide)>::value);
}

void FunctionTest::implicitlyConvertibleFunctorOverload() {
    struct {
        int operator()(int a, int b) {
            return a + b;
        }
        float operator()(float a, float b) {
            return a * b;
        }
    } sumOrMultiply;

    /* Just to be sure, these *are* constructible. This also makes Clang stop
       complaining that they're unused (while it doesn't complain if there's
       just one overload, heh). */
    CORRADE_VERIFY(std::is_constructible<Function<int(int, int)>, decltype(sumOrMultiply)>::value);
    CORRADE_VERIFY(std::is_constructible<Function<float(float, float)>, NoAllocateInitT, decltype(sumOrMultiply)>::value);

    CORRADE_VERIFY(!std::is_constructible<Function<float(int, int)>, decltype(sumOrMultiply)>::value);
    CORRADE_VERIFY(!std::is_constructible<Function<float(int, int)>, NoAllocateInitT, decltype(sumOrMultiply)>::value);
    CORRADE_VERIFY(!std::is_constructible<Function<int(float, float)>, decltype(sumOrMultiply)>::value);
    CORRADE_VERIFY(!std::is_constructible<Function<int(float, float)>, NoAllocateInitT, decltype(sumOrMultiply)>::value);
}

struct Immovable {
    explicit Immovable(int a): a{a} {}
    Immovable(const Immovable&) = delete;
    Immovable(Immovable&&) = delete;
    Immovable& operator=(const Immovable&) = delete;
    Immovable& operator=(Immovable&&) = delete;

    int a;
};

int acquire(Immovable&& a, int b) {
    return a.a*b;
}

void FunctionTest::rvalueArgumentFunction() {
    Function<int(Immovable&&, int)> a = acquire;

    Immovable immovable{-176};
    CORRADE_COMPARE(a(Utility::move(immovable), 3), -176*3);
}

void FunctionTest::rvalueArgumentMemberFunction() {
    struct Acquirer {
        int acquire(Immovable&& a) {
            return a.a*b;
        }
        int acquireLvalue(Immovable&& a) & {
            return a.a*b;
        }
        int acquireConst(Immovable&& a) const {
            return a.a*b;
        }
        int acquireConstLvalue(Immovable&& a) const & {
            return a.a*b;
        }

        int b = 3;
    } acquirer;

    Function<int(Immovable&&)> a{acquirer, &Acquirer::acquire};
    Function<int(Immovable&&)> b{acquirer, &Acquirer::acquireLvalue};
    Function<int(Immovable&&)> c{acquirer, &Acquirer::acquireConst};
    Function<int(Immovable&&)> d{acquirer, &Acquirer::acquireConstLvalue};

    Immovable immovable{-176};
    CORRADE_COMPARE(a(Utility::move(immovable)), -176*3);
    CORRADE_COMPARE(b(Utility::move(immovable)), -176*3);
    CORRADE_COMPARE(c(Utility::move(immovable)), -176*3);
    CORRADE_COMPARE(d(Utility::move(immovable)), -176*3);
}

void FunctionTest::rvalueArgumentTrivialFunctor() {
    struct Acquirer {
        int operator()(Immovable&& a, int b) {
            return a.a*b;
        }
    } acquirer;

    Function<int(Immovable&&, int)> a = acquirer;
    Function<int(Immovable&&, int)> b{NoAllocateInit, acquirer};
    CORRADE_VERIFY(!a.isAllocated());
    CORRADE_VERIFY(!b.isAllocated());

    Immovable immovable{-176};
    CORRADE_COMPARE(a(Utility::move(immovable), 3), -176*3);
    CORRADE_COMPARE(b(Utility::move(immovable), 3), -176*3);
}

void FunctionTest::rvalueArgumentFunctor() {
    /* Like rvalueArgumentTrivialFunctor(), just with the non-trivial
       destructor added */

    struct Acquirer {
        /* To make it non-trivially-copyable */
        ~Acquirer() {}

        int operator()(Immovable&& a, int b) {
            return a.a*b;
        }
    } acquirer;

    Function<int(Immovable&&, int)> a = acquirer;
    CORRADE_VERIFY(a.isAllocated());

    Immovable immovable{-176};
    CORRADE_COMPARE(a(Utility::move(immovable), 3), -176*3);
}

Immovable&& release(Immovable&& a, int b) {
    a.a /= b;
    return Utility::move(a);
}

void FunctionTest::rvalueResultFunction() {
    Function<Immovable&&(Immovable&&, int)> a = release;

    Immovable immovable{-176*3};
    CORRADE_COMPARE(a(Utility::move(immovable), 3).a, -176);
}

void FunctionTest::rvalueResultMemberFunction() {
    struct Releaser {
        Immovable&& release(Immovable&& a) {
            a.a /= b;
            return Utility::move(a);
        }
        Immovable&& releaseLvalue(Immovable&& a) & {
            a.a /= b;
            return Utility::move(a);
        }
        Immovable&& releaseConst(Immovable&& a) const {
            a.a /= b;
            return Utility::move(a);
        }
        Immovable&& releaseConstLvalue(Immovable&& a) const & {
            a.a /= b;
            return Utility::move(a);
        }

        int b = 3;
    } releaser;

    Function<Immovable&&(Immovable&&)> a{releaser, &Releaser::release};
    Function<Immovable&&(Immovable&&)> b{releaser, &Releaser::releaseLvalue};
    Function<Immovable&&(Immovable&&)> c{releaser, &Releaser::releaseConst};
    Function<Immovable&&(Immovable&&)> d{releaser, &Releaser::releaseConstLvalue};

    Immovable immovable{-176*3*3*3*3};
    CORRADE_COMPARE(a(Utility::move(immovable)).a, -176*3*3*3);
    CORRADE_COMPARE(b(Utility::move(immovable)).a, -176*3*3);
    CORRADE_COMPARE(c(Utility::move(immovable)).a, -176*3);
    CORRADE_COMPARE(d(Utility::move(immovable)).a, -176);
}

void FunctionTest::rvalueResultTrivialFunctor() {
    struct {
        Immovable&& operator()(Immovable&& a, int b) {
            a.a /= b;
            return Utility::move(a);
        }
    } releaser;

    Function<Immovable&&(Immovable&&, int)> a = releaser;
    Function<Immovable&&(Immovable&&, int)> b{NoAllocateInit, releaser};
    CORRADE_VERIFY(!a.isAllocated());
    CORRADE_VERIFY(!b.isAllocated());

    Immovable immovable{-176*3*3};
    CORRADE_COMPARE(a(Utility::move(immovable), 3).a, -176*3);
    CORRADE_COMPARE(b(Utility::move(immovable), 3).a, -176);
}

void FunctionTest::rvalueResultFunctor() {
    /* Like rvalueResultTrivialFunctor(), just with the non-trivial destructor
       added */

    struct Releaser {
        /* To make it non-trivially-copyable */
        ~Releaser() {}

        Immovable&& operator()(Immovable&& a, int b) {
            a.a /= b;
            return Utility::move(a);
        }
    } releaser;

    Function<Immovable&&(Immovable&&, int)> a = releaser;
    CORRADE_VERIFY(a.isAllocated());

    Immovable immovable{-176*3};
    CORRADE_COMPARE(a(Utility::move(immovable), 3).a, -176);
}

int sum(MoveOnly a, int b) {
    return a.a + b;
}

void FunctionTest::moveOnlyArgumentFunction() {
    {
        Function<int(MoveOnly, int)> a = sum;
        CORRADE_COMPARE(a(MoveOnly{2}, 3), 5);

        /* One temporary that got moved to the wrapper lambda, and then to the
           function */
        CORRADE_COMPARE(MoveOnly::constructed, 3);
        CORRADE_COMPARE(MoveOnly::destructed, 3);
        CORRADE_COMPARE(MoveOnly::moved, 2);
    }

    /* No extra instances should get used outside of operator() */
    CORRADE_COMPARE(MoveOnly::constructed, 3);
    CORRADE_COMPARE(MoveOnly::destructed, 3);
    CORRADE_COMPARE(MoveOnly::moved, 2);
}

void FunctionTest::moveOnlyArgumentMemberFunction() {
    {
        struct Sum {
            int sum(MoveOnly a) {
                return a.a + b;
            }
            int sumLvalue(MoveOnly a) & {
                return a.a + b;
            }
            int sumConst(MoveOnly a) const {
                return a.a + b;
            }
            int sumConstLvalue(MoveOnly a) const & {
                return a.a + b;
            }

            int b = 3;
        } sum;

        Function<int(MoveOnly)> a{sum, &Sum::sum};
        Function<int(MoveOnly)> b{sum, &Sum::sumLvalue};
        Function<int(MoveOnly)> c{sum, &Sum::sumConst};
        Function<int(MoveOnly)> d{sum, &Sum::sumConstLvalue};
        CORRADE_COMPARE(a(MoveOnly{2}), 5);
        CORRADE_COMPARE(b(MoveOnly{4}), 7);
        CORRADE_COMPARE(c(MoveOnly{8}), 11);
        CORRADE_COMPARE(d(MoveOnly{36}), 39);

        /* One temporary that got moved to the wrapper lambda, and then to the
           function, four times */
        CORRADE_COMPARE(MoveOnly::constructed, 3*4);
        CORRADE_COMPARE(MoveOnly::destructed, 3*4);
        CORRADE_COMPARE(MoveOnly::moved, 2*4);
    }

    /* No extra instances should get used outside of operator() */
    CORRADE_COMPARE(MoveOnly::constructed, 3*4);
    CORRADE_COMPARE(MoveOnly::destructed, 3*4);
    CORRADE_COMPARE(MoveOnly::moved, 2*4);
}

void FunctionTest::moveOnlyArgumentTrivialFunctor() {
    {
        struct {
            int operator()(MoveOnly a, int b) {
                return a.a + b;
            }
        } sum;

        Function<int(MoveOnly, int)> a = sum;
        Function<int(MoveOnly, int)> b{NoAllocateInit, sum};
        CORRADE_VERIFY(!a.isAllocated());
        CORRADE_VERIFY(!b.isAllocated());
        CORRADE_COMPARE(a(MoveOnly{2}, 3), 5);
        CORRADE_COMPARE(b(MoveOnly{4}, 5), 9);

        /* One temporary that got moved to the wrapper lambda, and then to the
           function, twice */
        CORRADE_COMPARE(MoveOnly::constructed, 3*2);
        CORRADE_COMPARE(MoveOnly::destructed, 3*2);
        CORRADE_COMPARE(MoveOnly::moved, 2*2);
    }

    /* No extra instances should get used outside of operator() */
    CORRADE_COMPARE(MoveOnly::constructed, 3*2);
    CORRADE_COMPARE(MoveOnly::destructed, 3*2);
    CORRADE_COMPARE(MoveOnly::moved, 2*2);
}

void FunctionTest::moveOnlyArgumentFunctor() {
    /* Like moveOnlyArgumentTrivialFunctor(), just with the non-trivial
       destructor added */

    {
        struct Sum {
            /* To make it non-trivially-copyable */
            ~Sum() {}

            int operator()(MoveOnly a, int b) {
                return a.a + b;
            }
        } sum;

        Function<int(MoveOnly, int)> a = sum;
        CORRADE_VERIFY(a.isAllocated());
        CORRADE_COMPARE(a(MoveOnly{2}, 3), 5);

        /* One temporary that got moved to the wrapper lambda, and then to the
           function */
        CORRADE_COMPARE(MoveOnly::constructed, 3);
        CORRADE_COMPARE(MoveOnly::destructed, 3);
        CORRADE_COMPARE(MoveOnly::moved, 2);
    }

    /* No extra instances should get used outside of operator() */
    CORRADE_COMPARE(MoveOnly::constructed, 3);
    CORRADE_COMPARE(MoveOnly::destructed, 3);
    CORRADE_COMPARE(MoveOnly::moved, 2);
}

MoveOnly subtract(int a, int b) {
    return MoveOnly{a - b};
}

void FunctionTest::moveOnlyResultFunction() {
    {
        Function<MoveOnly(int, int)> a = subtract;
        CORRADE_COMPARE(a(2, 3).a, -1);

        /* One temporary that got created and directly returned (move elision),
           or three without elision -- once in the function, once moved to the
           wrapper lambda, and once moved to the caller */
        if(MoveOnly::constructed == 1) {
            CORRADE_COMPARE(MoveOnly::constructed, 1);
            CORRADE_COMPARE(MoveOnly::destructed, 1);
            CORRADE_COMPARE(MoveOnly::moved, 0);
        } else {
            CORRADE_COMPARE(MoveOnly::constructed, 3);
            CORRADE_COMPARE(MoveOnly::destructed, 3);
            CORRADE_COMPARE(MoveOnly::moved, 2);
        }
    }

    /* No extra instances should get used outside of operator() */
    if(MoveOnly::constructed == 1) {
        CORRADE_COMPARE(MoveOnly::constructed, 1);
        CORRADE_COMPARE(MoveOnly::destructed, 1);
        CORRADE_COMPARE(MoveOnly::moved, 0);
    } else {
        CORRADE_COMPARE(MoveOnly::constructed, 3);
        CORRADE_COMPARE(MoveOnly::destructed, 3);
        CORRADE_COMPARE(MoveOnly::moved, 2);
    }
}

void FunctionTest::moveOnlyResultMemberFunction() {
    {
        struct Subtract {
            MoveOnly subtract(int a) {
                return MoveOnly{a - b};
            }
            MoveOnly subtractLvalue(int a) & {
                return MoveOnly{a - b};
            }
            MoveOnly subtractConst(int a) const {
                return MoveOnly{a - b};
            }
            MoveOnly subtractConstLvalue(int a) const & {
                return MoveOnly{a - b};
            }

            int b = 3;
        } subtract;

        Function<MoveOnly(int)> a{subtract, &Subtract::subtract};
        Function<MoveOnly(int)> b{subtract, &Subtract::subtractLvalue};
        Function<MoveOnly(int)> c{subtract, &Subtract::subtractConst};
        Function<MoveOnly(int)> d{subtract, &Subtract::subtractConstLvalue};
        CORRADE_COMPARE(a(2).a, -1);
        CORRADE_COMPARE(b(4).a, 1);
        CORRADE_COMPARE(c(8).a, 5);
        CORRADE_COMPARE(d(36).a, 33);

        /* One temporary that got created and directly returned (move elision),
           four times, or three times four without elision -- once in the
           function, once moved to the wrapper lambda, and once moved to the
           caller */
        if(MoveOnly::constructed == 1*4) {
            CORRADE_COMPARE(MoveOnly::constructed, 1*4);
            CORRADE_COMPARE(MoveOnly::destructed, 1*4);
            CORRADE_COMPARE(MoveOnly::moved, 0*4);
        } else {
            CORRADE_COMPARE(MoveOnly::constructed, 3*4);
            CORRADE_COMPARE(MoveOnly::destructed, 3*4);
            CORRADE_COMPARE(MoveOnly::moved, 2*4);
        }
    }

    /* No extra instances should get used outside of operator() */
    if(MoveOnly::constructed == 1*4) {
        CORRADE_COMPARE(MoveOnly::constructed, 1*4);
        CORRADE_COMPARE(MoveOnly::destructed, 1*4);
        CORRADE_COMPARE(MoveOnly::moved, 0*4);
    } else {
        CORRADE_COMPARE(MoveOnly::constructed, 3*4);
        CORRADE_COMPARE(MoveOnly::destructed, 3*4);
        CORRADE_COMPARE(MoveOnly::moved, 2*4);
    }
}

void FunctionTest::moveOnlyResultTrivialFunctor() {
    {
        struct {
            MoveOnly operator()(int a, int b) {
                return MoveOnly{a - b};
            }
        } subtract;

        Function<MoveOnly(int, int)> a = subtract;
        Function<MoveOnly(int, int)> b{NoAllocateInit, subtract};
        CORRADE_VERIFY(!a.isAllocated());
        CORRADE_VERIFY(!b.isAllocated());
        CORRADE_COMPARE(a(2, 3).a, -1);
        CORRADE_COMPARE(b(4, 2).a, 2);

        /* One temporary that got created and directly returned (move elision),
           twice, or twice times two without elision -- once (inlined) in the
           wrapper lambda, and once moved to the caller */
        if(MoveOnly::constructed == 1*2) {
            CORRADE_COMPARE(MoveOnly::constructed, 1*2);
            CORRADE_COMPARE(MoveOnly::destructed, 1*2);
            CORRADE_COMPARE(MoveOnly::moved, 0*2);
        } else {
            CORRADE_COMPARE(MoveOnly::constructed, 2*2);
            CORRADE_COMPARE(MoveOnly::destructed, 2*2);
            CORRADE_COMPARE(MoveOnly::moved, 1*2);
        }
    }

    /* No extra instances should get used outside of operator() */
    if(MoveOnly::constructed == 1*2) {
        CORRADE_COMPARE(MoveOnly::constructed, 1*2);
        CORRADE_COMPARE(MoveOnly::destructed, 1*2);
        CORRADE_COMPARE(MoveOnly::moved, 0*2);
    } else {
        CORRADE_COMPARE(MoveOnly::constructed, 2*2);
        CORRADE_COMPARE(MoveOnly::destructed, 2*2);
        CORRADE_COMPARE(MoveOnly::moved, 1*2);
    }
}

void FunctionTest::moveOnlyResultFunctor() {
    /* Like moveOnlyResultTrivialFunctor(), just with the non-trivial
       destructor added */

    {
        struct Subtract {
            /* To make it non-trivially-copyable */
            ~Subtract() {}

            MoveOnly operator()(int a, int b) {
                return MoveOnly{a - b};
            }
        } subtract;

        Function<MoveOnly(int, int)> a = subtract;
        CORRADE_VERIFY(a.isAllocated());
        CORRADE_COMPARE(a(2, 3).a, -1);

        /* One temporary that got created and directly returned (move elision),
           or twice times two without elision -- once (inlined) in the wrapper
           lambda, and once moved to the caller */
        if(MoveOnly::constructed == 1) {
            CORRADE_COMPARE(MoveOnly::constructed, 1);
            CORRADE_COMPARE(MoveOnly::destructed, 1);
            CORRADE_COMPARE(MoveOnly::moved, 0);
        } else {
            CORRADE_COMPARE(MoveOnly::constructed, 2);
            CORRADE_COMPARE(MoveOnly::destructed, 2);
            CORRADE_COMPARE(MoveOnly::moved, 1);
        }
    }

    /* No extra instances should get used outside of operator() */
    if(MoveOnly::constructed == 1) {
        CORRADE_COMPARE(MoveOnly::constructed, 1);
        CORRADE_COMPARE(MoveOnly::destructed, 1);
        CORRADE_COMPARE(MoveOnly::moved, 0);
    } else {
        CORRADE_COMPARE(MoveOnly::constructed, 2);
        CORRADE_COMPARE(MoveOnly::destructed, 2);
        CORRADE_COMPARE(MoveOnly::moved, 1);
    }
}

int argumentOverload(Containers::Function<int(int)>&& a) {
    return a(356);
}
int argumentOverload(Containers::Function<int(float)>&& a) {
    return a(35.6f);
}
/* Is here to verify that it doesn't only match a common prefix of the argument
   lists */
int argumentOverload(Containers::Function<int()>&& a) {
    return a();
}

int argumentOverloadA(int a) {
    return a - 3;
}
int argumentOverloadB(float a) {
    return int(a*0.1f);
}
int argumentOverloadC() {
    return 1337;
}

void FunctionTest::functionArgumentOverloadFunction() {
    int(*argumentOverloadAPointer)(int) = argumentOverloadA;
    int(*argumentOverloadBPointer)(float) = argumentOverloadB;
    int(*argumentOverloadCPointer)() = argumentOverloadC;
    #ifndef CORRADE_MSVC2015_COMPATIBILITY
    CORRADE_COMPARE(argumentOverload(argumentOverloadA), 353);
    CORRADE_COMPARE(argumentOverload(argumentOverloadB), 3);
    CORRADE_COMPARE(argumentOverload(argumentOverloadC), 1337);
    #else
    CORRADE_WARN("MSVC 2015 is unable to perform overload resolution for plain function references, only for function pointers.");
    #endif
    CORRADE_COMPARE(argumentOverload(argumentOverloadAPointer), 353);
    CORRADE_COMPARE(argumentOverload(argumentOverloadBPointer), 3);
    CORRADE_COMPARE(argumentOverload(argumentOverloadCPointer), 1337);
}

void FunctionTest::functionArgumentOverloadMemberFunction() {
    struct Overload {
        int a(int a) {
            return a - 3;
        }
        int b(float a) {
            return int(a*0.1f);
        }
        int c() {
            return 1337;
        }
    } overload;

    CORRADE_COMPARE(argumentOverload({overload, &Overload::a}), 353);
    CORRADE_COMPARE(argumentOverload({overload, &Overload::b}), 3);
    CORRADE_COMPARE(argumentOverload({overload, &Overload::c}), 1337);
}

void FunctionTest::functionArgumentOverloadTrivialFunctor() {
    struct {
        int operator()(int a) {
            return a - 3;
        }
    } a;
    struct {
        int operator()(float a) {
            return int(a*0.1f);
        }
    } b;
    struct {
        int operator()() {
            return 1337;
        }
    } c;
    CORRADE_VERIFY(!Function<int(int)>{a}.isAllocated());
    CORRADE_VERIFY(!Function<int(float)>{b}.isAllocated());
    CORRADE_VERIFY(!Function<int()>{c}.isAllocated());

    CORRADE_COMPARE(argumentOverload(a), 353);
    CORRADE_COMPARE(argumentOverload(b), 3);
    CORRADE_COMPARE(argumentOverload(c), 1337);
}

void FunctionTest::functionArgumentOverloadFunctor() {
    /* Like functionArgumentOverloadFunctor(), just with the non-trivial
       destructor added */

    struct A {
        /* To make it non-trivially-copyable */
        ~A() {}

        int operator()(int a) {
            return a - 3;
        }
    } a;
    struct B {
        /* To make it non-trivially-copyable */
        ~B() {}

        int operator()(float a) {
            return int(a*0.1f);
        }
    } b;
    struct C {
        /* To make it non-trivially-copyable */
        ~C() {}

        int operator()() {
            return 1337;
        }
    } c;
    CORRADE_VERIFY(Function<int(int)>{a}.isAllocated());
    CORRADE_VERIFY(Function<int(float)>{b}.isAllocated());
    CORRADE_VERIFY(Function<int()>{c}.isAllocated());

    CORRADE_COMPARE(argumentOverload(a), 353);
    CORRADE_COMPARE(argumentOverload(b), 3);
    CORRADE_COMPARE(argumentOverload(c), 1337);
}

void FunctionTest::functionArgumentOverloadLambda() {
    CORRADE_COMPARE(argumentOverload([](int a) { return a - 3; }), 353);
    CORRADE_COMPARE(argumentOverload([](float a) { return int(a*0.1f); }), 3);
    CORRADE_COMPARE(argumentOverload([]() { return 1337; }), 1337);
}

int resultOverload(Containers::Function<int(int)>&& a) {
    return a(356);
}
float resultOverload(Containers::Function<float(int)>&& a) {
    return a(356);
}

int resultOverloadA(int a) {
    return a - 3;
}
float resultOverloadB(int a) {
    return a*0.1f;
}

void FunctionTest::functionResultOverloadFunction() {
    int(*resultOverloadAPointer)(int) = resultOverloadA;
    float(*resultOverloadBPointer)(int) = resultOverloadB;
    #ifndef CORRADE_MSVC2015_COMPATIBILITY
    CORRADE_COMPARE(resultOverload(resultOverloadA), 353);
    CORRADE_COMPARE(resultOverload(resultOverloadB), 35.6f);
    #else
    CORRADE_WARN("MSVC 2015 is unable to perform overload resolution for plain function references, only for function pointers.");
    #endif
    CORRADE_COMPARE(resultOverload(resultOverloadAPointer), 353);
    CORRADE_COMPARE(resultOverload(resultOverloadBPointer), 35.6f);
}

void FunctionTest::functionResultOverloadMemberFunction() {
    struct Overload {
        int a(int a) {
            return a - 3;
        }
        float b(int a) {
            return a*0.1f;
        }
    } overload;

    CORRADE_COMPARE(resultOverload({overload, &Overload::a}), 353);
    CORRADE_COMPARE(resultOverload({overload, &Overload::b}), 35.6f);
}

void FunctionTest::functionResultOverloadTrivialFunctor() {
    struct {
        int operator()(int a) {
            return a - 3;
        }
    } a;
    struct {
        float operator()(int a) {
            return a*0.1f;
        }
    } b;
    CORRADE_VERIFY(!Function<int(int)>{a}.isAllocated());
    CORRADE_VERIFY(!Function<float(int)>{b}.isAllocated());

    CORRADE_COMPARE(resultOverload(a), 353);
    CORRADE_COMPARE(resultOverload(b), 35.6f);
}

void FunctionTest::functionResultOverloadFunctor() {
    /* Like functionResultOverloadTrivialFunctor(), just with the non-trivial
       destructor added */

    struct A {
        /* To make it non-trivially-copyable */
        ~A() {}

        int operator()(int a) {
            return a - 3;
        }
    } a;

    struct B {
        /* To make it non-trivially-copyable */
        ~B() {}

        float operator()(int a) {
            return a*0.1f;
        }
    } b;

    CORRADE_VERIFY(Function<int(int)>{a}.isAllocated());
    CORRADE_VERIFY(Function<float(int)>{b}.isAllocated());

    CORRADE_COMPARE(resultOverload(a), 353);
    CORRADE_COMPARE(resultOverload(b), 35.6f);
}

void FunctionTest::functionResultOverloadLambda() {
    CORRADE_COMPARE(resultOverload([](int a) { return a - 3; }), 353);
    CORRADE_COMPARE(resultOverload([](int a) { return a*0.1f; }), 35.6f);
}

}}}}

CORRADE_TEST_MAIN(Corrade::Containers::Test::FunctionTest)
