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

#include <cstdlib> /* std::div_t */
#include <utility>

#include "Corrade/Containers/Pair.h" /* need a template that isn't in Utility or std */
#include "Corrade/TestSuite/Tester.h"
#include "Corrade/Utility/AbstractHash.h" /* need just *some* type from Utility */
#include "Corrade/Utility/Move.h"

namespace Corrade { namespace Utility { namespace Test { namespace {

struct MoveTest: TestSuite::Tester {
    explicit MoveTest();

    void forward();
    void move();

    void swap();
    void swapArray();
    void swapMoveOnly();
    void swapMoveOnlyArray();
    void swapStlTypesAdlAmbiguity();
    void swapStlTypesAdlAmbiguityArray();
    void swapUtilityTypesAdlAmbiguity();
    void swapUtilityTypesAdlAmbiguityArray();
};

MoveTest::MoveTest() {
    addTests({&MoveTest::forward,
              &MoveTest::move,

              &MoveTest::swap,
              &MoveTest::swapArray,
              &MoveTest::swapMoveOnly,
              &MoveTest::swapMoveOnlyArray,
              &MoveTest::swapStlTypesAdlAmbiguity,
              &MoveTest::swapStlTypesAdlAmbiguityArray,
              &MoveTest::swapUtilityTypesAdlAmbiguity,
              &MoveTest::swapUtilityTypesAdlAmbiguityArray});
}

struct Foo {
    int a;
};

template<class L, class CL, class R, class CR, class V> void check(L&& l, CL&& cl, R&& r, CR&& cr, V&& v) {
    /* It makes sense to take std::forward() as a ground truth */
    CORRADE_VERIFY(std::is_same<decltype(std::forward<L>(l)), Foo&>::value);
    CORRADE_VERIFY(std::is_same<decltype(Utility::forward<L>(l)), Foo&>::value);

    CORRADE_VERIFY(std::is_same<decltype(std::forward<CL>(cl)), const Foo&>::value);
    CORRADE_VERIFY(std::is_same<decltype(Utility::forward<CL>(cl)), const Foo&>::value);

    CORRADE_VERIFY(std::is_same<decltype(std::forward<R>(r)), Foo&&>::value);
    CORRADE_VERIFY(std::is_same<decltype(Utility::forward<R>(r)), Foo&&>::value);

    CORRADE_VERIFY(std::is_same<decltype(std::forward<CR>(cr)), const Foo&&>::value);
    CORRADE_VERIFY(std::is_same<decltype(Utility::forward<CR>(cr)), const Foo&&>::value);

    CORRADE_VERIFY(std::is_same<decltype(std::forward<V>(v)), Foo&&>::value);
    CORRADE_VERIFY(std::is_same<decltype(Utility::forward<V>(v)), Foo&&>::value);
}

constexpr Foo ca{5};

void MoveTest::forward() {
    CORRADE_VERIFY(true); /* to register correct function name */

    Foo a{1};

    /* Using std::move() here to avoid accumulating bugs in case
       Utility::move() would be broken */
    check(a, ca, std::move(a), std::move(ca), Foo{3});

    /* Verify the utility can be used in a constexpr context */
    constexpr Foo cb = Utility::forward<const Foo>(ca);
    constexpr Foo cc = Utility::forward<Foo>(Foo{7});
    CORRADE_COMPARE(cb.a, 5);
    CORRADE_COMPARE(cc.a, 7);
}

void MoveTest::move() {
    Foo a;

    /* It makes sense to take std::move() as a ground truth */
    CORRADE_VERIFY(std::is_same<decltype(std::move(a)), Foo&&>::value);
    CORRADE_VERIFY(std::is_same<decltype(Utility::move(a)), Foo&&>::value);

    CORRADE_VERIFY(std::is_same<decltype(std::move(ca)), const Foo&&>::value);
    CORRADE_VERIFY(std::is_same<decltype(Utility::move(ca)), const Foo&&>::value);

    constexpr Foo cb = Utility::move(ca);
    CORRADE_COMPARE(cb.a, 5);
}

void MoveTest::swap() {
    int a = 3;
    int b = -27;
    Utility::swap(a, b);
    CORRADE_COMPARE(a, -27);
    CORRADE_COMPARE(b, 3);
}

void MoveTest::swapArray() {
    int a[]{3, 16};
    int b[]{-27, 44};
    Utility::swap(a, b);
    CORRADE_COMPARE(a[0], -27);
    CORRADE_COMPARE(a[1], 44);
    CORRADE_COMPARE(b[0], 3);
    CORRADE_COMPARE(b[1], 16);
}

void MoveTest::swapMoveOnly() {
    Containers::Pointer<int> a{InPlaceInit, 3};
    Containers::Pointer<int> b{InPlaceInit, -27};
    Utility::swap(a, b);
    CORRADE_COMPARE(*a, -27);
    CORRADE_COMPARE(*b, 3);
}

void MoveTest::swapMoveOnlyArray() {
    Containers::Pointer<int> a[]{
        Containers::Pointer<int>{InPlaceInit, 3},
        Containers::Pointer<int>{InPlaceInit, 16},
    };
    Containers::Pointer<int> b[]{
        Containers::Pointer<int>{InPlaceInit, -27},
        Containers::Pointer<int>{InPlaceInit, 44}
    };
    Utility::swap(a, b);
    CORRADE_COMPARE(*a[0], -27);
    CORRADE_COMPARE(*a[1], 44);
    CORRADE_COMPARE(*b[0], 3);
    CORRADE_COMPARE(*b[1], 16);
}

void MoveTest::swapStlTypesAdlAmbiguity() {
    /* With the `using` pattern, it should delegate to std::swap() (for which
       std::pair has a specialization) */
    {
        std::pair<int, int> a{3, -27};
        std::pair<int, int> b{-6, 54};
        using Utility::swap;
        swap(a, b);
        CORRADE_COMPARE(a, std::make_pair(-6, 54));
        CORRADE_COMPARE(b, std::make_pair(3, -27));

    /* A pointer to std::pair should also use std::swap(), as that's what ADL
       finds for it. The `using Utility::swap` does not introduce an ambiguity
       as the second argument is a `typename std::common_type<T>&` instead,
       which makes it a "worse" candidate. */
    } {
        std::pair<int, int> aData;
        std::pair<int, int> bData;
        std::pair<int, int>* a = &aData;
        std::pair<int, int>* b = &bData;
        using Utility::swap;
        swap(a, b);
        CORRADE_COMPARE(a, &bData);
        CORRADE_COMPARE(b, &aData);

    /* On the other hand, std::div_t picks Utility::swap, as -- I suspect --
       div_t exists in the global namespace and is brought into the std
       namespace with an `using`, so std::swap isn't found for it by ADL. */
    } {
        std::div_t aData, bData;
        std::div_t* a = &aData;
        std::div_t* b = &bData;
        using Utility::swap;
        swap(a, b);
        CORRADE_COMPARE(a, &bData);
        CORRADE_COMPARE(b, &aData);
    }
}

void MoveTest::swapStlTypesAdlAmbiguityArray() {
    /* Like swapStlTypesAdlAmbiguity(), but single-item arrays. Should be
       enough to trigger the same problems. */

    {
        std::pair<int, int> a[]{{3, -27}};
        std::pair<int, int> b[]{{-6, 54}};
        using Utility::swap;
        swap(a, b);
        CORRADE_COMPARE(*a, std::make_pair(-6, 54));
        CORRADE_COMPARE(*b, std::make_pair(3, -27));
    } {
        std::pair<int, int> aData;
        std::pair<int, int> bData;
        std::pair<int, int>* a[]{&aData};
        std::pair<int, int>* b[]{&bData};
        using Utility::swap;
        swap(a, b);
        CORRADE_COMPARE(*a, &bData);
        CORRADE_COMPARE(*b, &aData);
    } {
        std::div_t aData, bData;
        std::div_t* a[]{&aData};
        std::div_t* b[]{&bData};
        using Utility::swap;
        swap(a, b);
        CORRADE_COMPARE(*a, &bData);
        CORRADE_COMPARE(*b, &aData);
    }
}

void MoveTest::swapUtilityTypesAdlAmbiguity() {
    /* With `using Utility::swap`, it should pick the implementation that it
       would pick with ADL anyway. There's no potential for ambiguity with
       std::swap here. */
    {
        HashDigest<3> a{'a', 'b', 'c'};
        HashDigest<3> b{'C', 'B', 'A'};
        using Utility::swap;
        swap(a, b);
        CORRADE_COMPARE(a, (HashDigest<3>{'C', 'B', 'A'}));
        CORRADE_COMPARE(b, (HashDigest<3>{'a', 'b', 'c'}));

    /* With `using std::swap` (as opposed to `using Utility::swap`), it picks
       std::swap, because the Utility::swap picked by ADL is a worse
       candidate due to `typename std::common_type<T>&` for its second
       argument.

       An alternative solution here would be a so-called "ADL Barrier idiom",
       i.e.

        namespace Corrade { namespace Utility {

            namespace StopAdl {
                template<class T> void swap(T& a, T& b);
            }

            // Note that using StopAdl::swap wouldn't work!
            using namespace StopAdl;
        }

       but that doesn't solve the conflicts with std::swap() in the pair* case
       above, or the "both std and Utility" case below. The common_type does. */
    } {
        HashDigest<3> a{'a', 'b', 'c'};
        HashDigest<3> b{'C', 'B', 'A'};
        using std::swap;
        swap(a, b);
        CORRADE_COMPARE(a, (HashDigest<3>{'C', 'B', 'A'}));
        CORRADE_COMPARE(b, (HashDigest<3>{'a', 'b', 'c'}));

    /* A type that's both in the std and Utility namespace and there's no
       specialization for it. Like with the std::pair* case above,
       `using Utility::swap` would be ambiguous without the common_type, and
       ADL Barrier wouldn't help either. */
    } {
        Containers::Pair<HashDigest<3>, std::pair<int, int>> a{HashDigest<3>{'a', 'b', 'c'}, {-3, 6}};
        Containers::Pair<HashDigest<3>, std::pair<int, int>> b{HashDigest<3>{'C', 'B', 'A'}, {2, -4}};
        using Utility::swap;
        swap(a, b);
        CORRADE_COMPARE(a, Containers::pair(HashDigest<3>{'C', 'B', 'A'}, std::make_pair(2, -4)));
        CORRADE_COMPARE(b, Containers::pair(HashDigest<3>{'a', 'b', 'c'}, std::make_pair(-3, 6)));

    /* Then, `using std::swap` would stop being ambiguous with just the ADL
       Barrier alone, but common_type fixes it too. */
    } {
        Containers::Pair<HashDigest<3>, std::pair<int, int>> a{HashDigest<3>{'a', 'b', 'c'}, {-3, 6}};
        Containers::Pair<HashDigest<3>, std::pair<int, int>> b{HashDigest<3>{'C', 'B', 'A'}, {2, -4}};
        using std::swap;
        swap(a, b);
        CORRADE_COMPARE(a, Containers::pair(HashDigest<3>{'C', 'B', 'A'}, std::make_pair(2, -4)));
        CORRADE_COMPARE(b, Containers::pair(HashDigest<3>{'a', 'b', 'c'}, std::make_pair(-3, 6)));
    }
}

void MoveTest::swapUtilityTypesAdlAmbiguityArray() {
    /* Like swapUtilityTypesAdlAmbiguity(), but single-item arrays. Should be
       enough to trigger the same problems. */

    {
        HashDigest<3> a[]{HashDigest<3>{'a', 'b', 'c'}};
        HashDigest<3> b[]{HashDigest<3>{'C', 'B', 'A'}};
        using Utility::swap;
        swap(a, b);
        CORRADE_COMPARE(*a, (HashDigest<3>{'C', 'B', 'A'}));
        CORRADE_COMPARE(*b, (HashDigest<3>{'a', 'b', 'c'}));
    } {
        HashDigest<3> a[]{HashDigest<3>{'a', 'b', 'c'}};
        HashDigest<3> b[]{HashDigest<3>{'C', 'B', 'A'}};
        using std::swap;
        swap(a, b);
        CORRADE_COMPARE(*a, (HashDigest<3>{'C', 'B', 'A'}));
        CORRADE_COMPARE(*b, (HashDigest<3>{'a', 'b', 'c'}));
    } {
        Containers::Pair<HashDigest<3>, std::pair<int, int>> a[]{{HashDigest<3>{'a', 'b', 'c'}, {-3, 6}}};
        Containers::Pair<HashDigest<3>, std::pair<int, int>> b[]{{HashDigest<3>{'C', 'B', 'A'}, {2, -4}}};
        using Utility::swap;
        swap(a, b);
        CORRADE_COMPARE(*a, Containers::pair(HashDigest<3>{'C', 'B', 'A'}, std::make_pair(2, -4)));
        CORRADE_COMPARE(*b, Containers::pair(HashDigest<3>{'a', 'b', 'c'}, std::make_pair(-3, 6)));
    } {
        Containers::Pair<HashDigest<3>, std::pair<int, int>> a[]{{HashDigest<3>{'a', 'b', 'c'}, {-3, 6}}};
        Containers::Pair<HashDigest<3>, std::pair<int, int>> b[]{{HashDigest<3>{'C', 'B', 'A'}, {2, -4}}};
        using std::swap;
        swap(a, b);
        CORRADE_COMPARE(*a, Containers::pair(HashDigest<3>{'C', 'B', 'A'}, std::make_pair(2, -4)));
        CORRADE_COMPARE(*b, Containers::pair(HashDigest<3>{'a', 'b', 'c'}, std::make_pair(-3, 6)));
    }
}

}}}}

CORRADE_TEST_MAIN(Corrade::Utility::Test::MoveTest)
