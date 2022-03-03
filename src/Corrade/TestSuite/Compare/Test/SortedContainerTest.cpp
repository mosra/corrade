/*
    This file is part of Corrade.

    Copyright © 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016,
                2017, 2018, 2019, 2020, 2021, 2022
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

#include <sstream>
#include <unordered_set>
#include <vector>

#include "Corrade/Containers/Array.h"
#include "Corrade/Containers/Reference.h"
#include "Corrade/Containers/StringStl.h"
#include "Corrade/TestSuite/Tester.h"
#include "Corrade/TestSuite/Compare/SortedContainer.h"
#include "Corrade/TestSuite/Compare/String.h"

namespace Corrade { namespace TestSuite { namespace Compare { namespace Test { namespace {

struct SortedContainerTest: Tester {
    explicit SortedContainerTest();

    void copyableContainer();
    void nonOwningView();
    void nonCopyableContainer();
    void noRandomAccessContainer();
    void noDefaultConstructor();
    void differentSize();
    void copyConstructPlainStruct();
};

SortedContainerTest::SortedContainerTest() {
    addTests({&SortedContainerTest::copyableContainer,
              &SortedContainerTest::nonOwningView,
              &SortedContainerTest::nonCopyableContainer,
              &SortedContainerTest::noRandomAccessContainer,
              &SortedContainerTest::noDefaultConstructor,
              &SortedContainerTest::differentSize,
              &SortedContainerTest::copyConstructPlainStruct});
}

/* Majority is tested in ContainerTest, this tests only specifics to this
   derived class */

void SortedContainerTest::copyableContainer() {
    std::vector<int> a{1, 2, 4, 3};
    std::vector<int> b{1, 4, 3, 2};
    std::vector<int> c{1, 4, 3, 3};

    CORRADE_COMPARE((Comparator<Compare::SortedContainer<std::vector<int>>>{}(a, b)), ComparisonStatusFlags{});
    CORRADE_COMPARE((Comparator<Compare::SortedContainer<std::vector<int>>>{}(b, a)), ComparisonStatusFlags{});
    CORRADE_COMPARE((Comparator<Compare::SortedContainer<std::vector<int>>>{}(a, c)), ComparisonStatusFlag::Failed);
}

void SortedContainerTest::nonOwningView() {
    int a[]{1, 2, 4, 3};
    int b[]{1, 4, 3, 2};
    int c[]{1, 4, 3, 3};

    CORRADE_COMPARE((Comparator<Compare::SortedContainer<Containers::ArrayView<int>>>{}(a, b)), ComparisonStatusFlags{});
    CORRADE_COMPARE((Comparator<Compare::SortedContainer<Containers::ArrayView<int>>>{}(b, a)), ComparisonStatusFlags{});
    CORRADE_COMPARE((Comparator<Compare::SortedContainer<Containers::ArrayView<int>>>{}(a, c)), ComparisonStatusFlag::Failed);

    /* The actual data shouldn't be changed by the comparator */
    CORRADE_COMPARE_AS(Containers::arrayView(a),
        Containers::arrayView({1, 2, 4, 3}),
        TestSuite::Compare::Container);
    CORRADE_COMPARE_AS(Containers::arrayView(b),
        Containers::arrayView({1, 4, 3, 2}),
        TestSuite::Compare::Container);
    CORRADE_COMPARE_AS(Containers::arrayView(c),
        Containers::arrayView({1, 4, 3, 3}),
        TestSuite::Compare::Container);
}

void SortedContainerTest::nonCopyableContainer() {
    Containers::Array<int> a{InPlaceInit, {1, 2, 4, 3}};
    Containers::Array<int> b{InPlaceInit, {1, 4, 3, 2}};
    Containers::Array<int> c{InPlaceInit, {1, 4, 3, 3}};

    CORRADE_COMPARE((Comparator<Compare::SortedContainer<Containers::Array<int>>>{}(a, b)), ComparisonStatusFlags{});
    CORRADE_COMPARE((Comparator<Compare::SortedContainer<Containers::Array<int>>>{}(b, a)), ComparisonStatusFlags{});
    CORRADE_COMPARE((Comparator<Compare::SortedContainer<Containers::Array<int>>>{}(a, c)), ComparisonStatusFlag::Failed);
}

void SortedContainerTest::noRandomAccessContainer() {
    std::unordered_set<int> a{1, 2, 4, 3};
    std::unordered_set<int> b{1, 4, 3, 2};
    std::unordered_set<int> c{1, 4, 3, 3};

    CORRADE_COMPARE((Comparator<Compare::SortedContainer<std::unordered_set<int>>>{}(a, b)), ComparisonStatusFlags{});
    CORRADE_COMPARE((Comparator<Compare::SortedContainer<std::unordered_set<int>>>{}(b, a)), ComparisonStatusFlags{});
    CORRADE_COMPARE((Comparator<Compare::SortedContainer<std::unordered_set<int>>>{}(a, c)), ComparisonStatusFlag::Failed);
}

void SortedContainerTest::noDefaultConstructor() {
    int oneData = 1;
    int twoData = 2;
    int threeData = 3;
    int fourData = 4;
    Containers::Reference<int> one = oneData;
    Containers::Reference<int> two = twoData;
    Containers::Reference<int> three = threeData;
    Containers::Reference<int> four = fourData;
    std::vector<Containers::Reference<int>> a{one, two, four, three};
    std::vector<Containers::Reference<int>> b{one, four, three, two};
    std::vector<Containers::Reference<int>> c{one, four, three, three};

    CORRADE_COMPARE((Comparator<Compare::SortedContainer<std::vector<Containers::Reference<int>>>>{}(a, b)), ComparisonStatusFlags{});
    CORRADE_COMPARE((Comparator<Compare::SortedContainer<std::vector<Containers::Reference<int>>>>{}(b, a)), ComparisonStatusFlags{});
    CORRADE_COMPARE((Comparator<Compare::SortedContainer<std::vector<Containers::Reference<int>>>>{}(a, c)), ComparisonStatusFlag::Failed);
}

void SortedContainerTest::differentSize() {
    /* Mainly to verify we're not accidentally using wrong sizes for copying */

    std::vector<int> a{1, 2, 4, 3};
    std::vector<int> b;

    {
        std::stringstream out;
        Debug redirectOutput{&out};
        Comparator<Compare::SortedContainer<std::vector<int>>> compare;
        ComparisonStatusFlags flags = compare(a, b);
        CORRADE_COMPARE(flags, ComparisonStatusFlag::Failed);
        compare.printMessage(flags, redirectOutput, "a", "b");
        CORRADE_COMPARE_AS(out.str(),
            "Containers a and b have different size, actual 4 but 0 expected.",
            TestSuite::Compare::StringHasPrefix);
    } {
        std::stringstream out;
        Debug redirectOutput{&out};
        Comparator<Compare::SortedContainer<std::vector<int>>> compare;
        ComparisonStatusFlags flags = compare(b, a);
        CORRADE_COMPARE(flags, ComparisonStatusFlag::Failed);
        compare.printMessage(flags, redirectOutput, "b", "a");
        CORRADE_COMPARE_AS(out.str(),
            "Containers b and a have different size, actual 0 but 4 expected.",
            TestSuite::Compare::StringHasPrefix);
    }
}

struct Int {
    int a;

    friend bool operator==(Int a, Int b) { return a.a == b.a; }
    friend bool operator<(Int a, Int b) { return a.a < b.a; }
};

void SortedContainerTest::copyConstructPlainStruct() {
    /* This needs special handling on GCC 4.8, where T{b} (copy-construction)
       attempts to convert Int to int to initialize the first argument and
       fails miserably. */

    Int a[]{{1}, {2}, {4}, {3}};
    Int b[]{{1}, {4}, {3}, {2}};
    Int c[]{{1}, {4}, {3}, {3}};

    CORRADE_COMPARE((Comparator<Compare::SortedContainer<Containers::ArrayView<Int>>>{}(a, b)), ComparisonStatusFlags{});
    CORRADE_COMPARE((Comparator<Compare::SortedContainer<Containers::ArrayView<Int>>>{}(b, a)), ComparisonStatusFlags{});
    CORRADE_COMPARE((Comparator<Compare::SortedContainer<Containers::ArrayView<Int>>>{}(a, c)), ComparisonStatusFlag::Failed);
}

}}}}}

CORRADE_TEST_MAIN(Corrade::TestSuite::Compare::Test::SortedContainerTest)
