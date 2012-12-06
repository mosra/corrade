#ifndef Corrade_Containers_Test_LinkedListTest_h
#define Corrade_Containers_Test_LinkedListTest_h
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

#include "TestSuite/Tester.h"

#include "Containers/LinkedList.h"

namespace Corrade { namespace Containers { namespace Test {

class LinkedListTest: public TestSuite::Tester {
    public:
        LinkedListTest();

        void listBackReference();
        void insert();
        void insertFromOtherList();
        void insertBeforeFromOtherList();
        void cut();
        void cutFromOtherList();
        void clear();
        void moveList();
        void moveItem();

        class Item: public LinkedListItem<Item> {
            public:
                inline Item(Item&& other): LinkedListItem<Item>(std::forward<LinkedListItem<Item>>(other)) {}

                inline Item& operator=(Item&& other) {
                    LinkedListItem<Item>::operator=(std::forward<LinkedListItem<Item>>(other));
                    return *this;
                }

                static int count;
                inline Item() { ++count; }
                inline ~Item() { --count; }
        };
};

}}}

#endif
