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

#include "Corrade/Containers/Array.h"
#include "Corrade/Containers/String.h"
#include "Corrade/TestSuite/Tester.h"
#include "Corrade/TestSuite/Compare/Container.h"

namespace Corrade { namespace TestSuite { namespace Compare { namespace Test { namespace {

struct ContainerTest: Tester {
    explicit ContainerTest();

    void same();
    void outputActualSmaller();
    void outputExpectedSmaller();
    void output();
    void floatingPoint();
};

ContainerTest::ContainerTest() {
    addTests({&ContainerTest::same,
              &ContainerTest::outputActualSmaller,
              &ContainerTest::outputExpectedSmaller,
              &ContainerTest::output,
              &ContainerTest::floatingPoint});
}

void ContainerTest::same() {
    Containers::Array<int> a{InPlaceInit, {1, 2, 3, 4}};
    CORRADE_COMPARE(Comparator<Compare::Container<Containers::Array<int>>>{}(a, a), ComparisonStatusFlags{});

    /* Should not return any flags for a success */
    Comparator<Compare::Container<Containers::Array<int>>> compare;
    CORRADE_COMPARE(compare(a, a), ComparisonStatusFlags{});
}

void ContainerTest::outputActualSmaller() {
    Containers::String out;

    Containers::Array<int> a{InPlaceInit, {1, 2, 3}};
    Containers::Array<int> b{InPlaceInit, {1, 2, 3, 4}};

    {
        Debug redirectOutput{&out};
        Comparator<Compare::Container<Containers::Array<int>>> compare;
        ComparisonStatusFlags flags = compare(a, b);
        CORRADE_COMPARE(flags, ComparisonStatusFlag::Failed);
        compare.printMessage(flags, redirectOutput, "a", "b");
    }

    CORRADE_COMPARE(out,
        "Containers a and b have different size, actual 3 but 4 expected. Actual contents:\n"
        "        {1, 2, 3}\n"
        "        but expected\n"
        "        {1, 2, 3, 4}\n"
        "        Expected has 4 on position 3.\n");
}

void ContainerTest::outputExpectedSmaller() {
    Containers::String out;

    Containers::Array<int> a{InPlaceInit, {1, 2, 3, 4}};
    Containers::Array<int> b{InPlaceInit, {1, 2, 3}};

    {
        Debug redirectOutput{&out};
        Comparator<Compare::Container<Containers::Array<int>>> compare;
        ComparisonStatusFlags flags = compare(a, b);
        CORRADE_COMPARE(flags, ComparisonStatusFlag::Failed);
        compare.printMessage(flags, redirectOutput, "a", "b");
    }

    CORRADE_COMPARE(out, "Containers a and b have different size, actual 4 but 3 expected. Actual contents:\n"
        "        {1, 2, 3, 4}\n"
        "        but expected\n"
        "        {1, 2, 3}\n"
        "        Actual has 4 on position 3.\n");
}

void ContainerTest::output() {
    Containers::String out;

    Containers::Array<int> a{InPlaceInit, {1, 9, 3, 4}};
    Containers::Array<int> b{InPlaceInit, {1, 2, 3, 4}};

    {
        Debug redirectOutput{&out};
        Comparator<Compare::Container<Containers::Array<int>>> compare;
        ComparisonStatusFlags flags = compare(a, b);
        CORRADE_COMPARE(flags, ComparisonStatusFlag::Failed);
        compare.printMessage(flags, redirectOutput, "a", "b");
    }

    CORRADE_COMPARE(out, "Containers a and b have different contents, actual:\n"
        "        {1, 9, 3, 4}\n"
        "        but expected\n"
        "        {1, 2, 3, 4}\n"
        "        Actual 9 but 2 expected on position 1.\n");
}

void ContainerTest::floatingPoint() {
    Containers::String out;

    Containers::Array<float> a{InPlaceInit, {3.20212f, 3.20212f}};
    Containers::Array<float> b{InPlaceInit, {3.20212f, 3.20213f}};
    Containers::Array<float> c{InPlaceInit, {3.20213f, 3.20219f}};

    CORRADE_COMPARE(Comparator<Compare::Container<Containers::Array<float>>>{}(a, b), ComparisonStatusFlags{});

    {
        Debug redirectOutput{&out};
        Comparator<Compare::Container<Containers::Array<float>>> compare;
        ComparisonStatusFlags flags = compare(a, c);
        CORRADE_COMPARE(flags, ComparisonStatusFlag::Failed);
        compare.printMessage(flags, redirectOutput, "a", "c");
    }

    /* It should report the second element, not the first */
    CORRADE_COMPARE(out, "Containers a and c have different contents, actual:\n"
        "        {3.20212, 3.20212}\n"
        "        but expected\n"
        "        {3.20213, 3.20219}\n"
        "        Actual 3.20212 but 3.20219 expected on position 1.\n");
}

}}}}}

CORRADE_TEST_MAIN(Corrade::TestSuite::Compare::Test::ContainerTest)
