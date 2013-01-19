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

#include <sstream>

#include "Containers/LinkedList.h"
#include "TestSuite/Tester.h"

using namespace Corrade::Utility;

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
};

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

typedef Containers::LinkedList<Item> LinkedList;

int Item::count = 0;

LinkedListTest::LinkedListTest() {
    addTests(&LinkedListTest::listBackReference,
             &LinkedListTest::insert,
             &LinkedListTest::insertFromOtherList,
             &LinkedListTest::insertBeforeFromOtherList,
             &LinkedListTest::cut,
             &LinkedListTest::cutFromOtherList,
             &LinkedListTest::clear,
             &LinkedListTest::moveList,
             &LinkedListTest::moveItem);
}

void LinkedListTest::listBackReference() {
    LinkedList list;
    Item* item = new Item;

    /* Insert -> list is backreferenced from the item */
    list.insert(item);
    CORRADE_VERIFY(item->list() == &list);

    /* Cut -> list is not referenced */
    list.cut(item);
    CORRADE_VERIFY(item->list() == nullptr);

    /* Destruct -> item removes itself from the list */
    list.insert(item);
    CORRADE_VERIFY(!list.isEmpty());
    delete item;
    CORRADE_VERIFY(list.isEmpty());
}

void LinkedListTest::insert() {
    LinkedList list;

    CORRADE_VERIFY(list.isEmpty());

    /* Inserting first item */
    Item* item = new Item;
    list.insert(item);
    CORRADE_VERIFY(!list.isEmpty());
    CORRADE_VERIFY(list.first() == item);
    CORRADE_VERIFY(list.last() == item);
    CORRADE_VERIFY(item->previous() == nullptr);
    CORRADE_VERIFY(item->next() == nullptr);

    /* Inserting item at the beginning */
    Item* item2 = new Item;
    list.insert(item2, item);
    CORRADE_VERIFY(list.first() == item2);
    CORRADE_VERIFY(item2->previous() == nullptr);
    CORRADE_VERIFY(item2->next() == item);
    CORRADE_VERIFY(item->previous() == item2);

    /* ...same as previously */
    CORRADE_VERIFY(list.last() == item);
    CORRADE_VERIFY(item->next() == nullptr);

    /* Inserting item at the end */
    Item* item3 = new Item;
    list.insert(item3);
    CORRADE_VERIFY(list.last() == item3);
    CORRADE_VERIFY(item->next() == item3);
    CORRADE_VERIFY(item3->previous() == item);
    CORRADE_VERIFY(item3->next() == nullptr);

    /* ..same as previously */
    CORRADE_VERIFY(list.first() == item2);
    CORRADE_VERIFY(item2->previous() == nullptr);
    CORRADE_VERIFY(item2->next() == item);
    CORRADE_VERIFY(item->previous() == item2);

    /* Inserting item in the middle */
    Item* item4 = new Item;
    list.insert(item4, item);
    CORRADE_VERIFY(item2->next() == item4);
    CORRADE_VERIFY(item4->previous() == item2);
    CORRADE_VERIFY(item4->next() == item);
    CORRADE_VERIFY(item->previous() == item4);

    /* ...same as previously */
    CORRADE_VERIFY(list.first() == item2);
    CORRADE_VERIFY(list.last() == item3);
    CORRADE_VERIFY(item2->previous() == nullptr);
    CORRADE_VERIFY(item->next() == item3);
    CORRADE_VERIFY(item3->previous() == item);
    CORRADE_VERIFY(item3->next() == nullptr);
}

void LinkedListTest::insertFromOtherList() {
    std::stringstream out;
    Error::setOutput(&out);

    LinkedList list;
    Item item;
    list.insert(&item);

    LinkedList list2;
    list2.insert(&item);
    CORRADE_COMPARE(out.str(), "Containers::LinkedList: Cannot insert item already connected elsewhere.\n");
}

void LinkedListTest::insertBeforeFromOtherList() {
    std::stringstream out;
    Error::setOutput(&out);

    LinkedList list;
    Item item;
    list.insert(&item);

    LinkedList list2;
    Item item2;
    list2.insert(&item2, &item);
    CORRADE_COMPARE(out.str(), "Containers::LinkedList: Cannot insert before item which is not part of the list.\n");
}

void LinkedListTest::cutFromOtherList() {
    std::stringstream out;
    Error::setOutput(&out);

    LinkedList list;
    Item item;
    list.insert(&item);

    LinkedList list2;
    list2.cut(&item);
    CORRADE_COMPARE(out.str(), "Containers::LinkedList: Cannot cut out item which is not part of the list.\n");
}

void LinkedListTest::cut() {
    LinkedList list;
    Item item;
    Item item2;
    Item item3;
    Item item4;
    list.insert(&item2);
    list.insert(&item4);
    list.insert(&item);
    list.insert(&item3);

    /* Cut from the middle */
    list.cut(&item);
    CORRADE_VERIFY(item4.next() == &item3);
    CORRADE_VERIFY(item3.previous() == &item4);
    CORRADE_VERIFY(item.previous() == nullptr);
    CORRADE_VERIFY(item.next() == nullptr);

    /* ...same as previously */
    CORRADE_VERIFY(list.first() == &item2);
    CORRADE_VERIFY(list.last() == &item3);
    CORRADE_VERIFY(item2.previous() == nullptr);
    CORRADE_VERIFY(item2.next() == &item4);
    CORRADE_VERIFY(item4.previous() == &item2);
    CORRADE_VERIFY(item3.next() == nullptr);

    /* Cut from beginning */
    list.cut(&item2);
    CORRADE_VERIFY(list.first() == &item4);
    CORRADE_VERIFY(item4.previous() == nullptr);
    CORRADE_VERIFY(item2.previous() == nullptr);
    CORRADE_VERIFY(item2.next() == nullptr);

    /* ...same as previously */
    CORRADE_VERIFY(list.last() == &item3);
    CORRADE_VERIFY(item4.next() == &item3);
    CORRADE_VERIFY(item3.previous() == &item4);
    CORRADE_VERIFY(item3.next() == nullptr);

    /* Cut from the end */
    list.cut(&item3);
    CORRADE_VERIFY(list.last() == &item4);
    CORRADE_VERIFY(item4.next() == nullptr);
    CORRADE_VERIFY(item3.previous() == nullptr);
    CORRADE_VERIFY(item3.next() == nullptr);

    /* ...same as previously */
    CORRADE_VERIFY(list.first() == &item4);
    CORRADE_VERIFY(item4.previous() == nullptr);

    /* Cut last item */
    list.cut(&item4);
    CORRADE_VERIFY(list.first() == nullptr);
    CORRADE_VERIFY(list.last() == nullptr);
    CORRADE_VERIFY(item4.previous() == nullptr);
    CORRADE_VERIFY(item4.next() == nullptr);

    CORRADE_VERIFY(list.isEmpty());
}

void LinkedListTest::clear() {
    CORRADE_COMPARE(Item::count, 0);

    /* Explicit clear */
    {
        LinkedList list;

        list.insert(new Item);
        list.insert(new Item);
        list.insert(new Item);
        list.insert(new Item);
        CORRADE_COMPARE(Item::count, 4);

        list.clear();
        CORRADE_COMPARE(Item::count, 0);
    }

    /* Destructor */
    {
        LinkedList list;
        list.insert(new Item);
        list.insert(new Item);
        list.insert(new Item);
        list.insert(new Item);
    }
    CORRADE_COMPARE(Item::count, 0);
}

void LinkedListTest::moveList() {
    Item* item1 = new Item;
    Item* item2 = new Item;
    LinkedList list;
    list.insert(item1);
    list.insert(item2);

    /* Move constructor */
    LinkedList list2(std::move(list));
    CORRADE_VERIFY(list.first() == nullptr);
    CORRADE_VERIFY(list.last() == nullptr);
    CORRADE_VERIFY(list2.first() == item1);
    CORRADE_VERIFY(list2.last() == item2);
    CORRADE_VERIFY(item1->list() == &list2);
    CORRADE_VERIFY(item2->list() == &list2);

    CORRADE_COMPARE(Item::count, 2);

    LinkedList list3;
    list3.insert(new Item);

    /* Move assignment */
    list3 = std::move(list2);
    CORRADE_VERIFY(list2.first() == nullptr);
    CORRADE_VERIFY(list2.last() == nullptr);
    CORRADE_VERIFY(list3.first() == item1);
    CORRADE_VERIFY(list3.last() == item2);
    CORRADE_VERIFY(item1->list() == &list3);
    CORRADE_VERIFY(item2->list() == &list3);

    list3.clear();
    CORRADE_COMPARE(Item::count, 0);
}

void LinkedListTest::moveItem() {
    LinkedList list;
    Item item;
    Item item2;
    Item item3;
    list.insert(&item);
    list.insert(&item2);
    list.insert(&item3);

    /* Move item in the midde */
    Item item2Moved(std::move(item2));
    CORRADE_VERIFY(item2.list() == nullptr);
    CORRADE_VERIFY(item2.previous() == nullptr);
    CORRADE_VERIFY(item2.next() == nullptr);
    CORRADE_VERIFY(item2Moved.list() == &list);
    CORRADE_VERIFY(item2Moved.previous() == &item);
    CORRADE_VERIFY(item2Moved.next() == &item3);
    CORRADE_VERIFY(item.next() == &item2Moved);
    CORRADE_VERIFY(item3.previous() == &item2Moved);

    /* Move assignment */
    LinkedList list2;
    Item item4;

    list2.insert(&item4);
    CORRADE_VERIFY(!list2.isEmpty());

    item4 = std::move(item2Moved);
    CORRADE_VERIFY(list2.isEmpty());

    CORRADE_VERIFY(item2Moved.list() == nullptr);
    CORRADE_VERIFY(item2Moved.previous() == nullptr);
    CORRADE_VERIFY(item2Moved.next() == nullptr);
    CORRADE_VERIFY(item4.list() == &list);
    CORRADE_VERIFY(item4.previous() == &item);
    CORRADE_VERIFY(item4.next() == &item3);
    CORRADE_VERIFY(item.next() == &item4);
    CORRADE_VERIFY(item3.previous() == &item4);

    /* Remove other items to have only one remaining */
    list.cut(&item4);
    list.cut(&item3);

    /* Move item at the beginning/end */
    Item itemMoved(std::move(item));
    CORRADE_VERIFY(itemMoved.list() == &list);
    CORRADE_VERIFY(list.first() == &itemMoved);
    CORRADE_VERIFY(list.last() == &itemMoved);

    /* Move assignment */
    Item item5;

    list2.insert(&item5);
    CORRADE_VERIFY(!list2.isEmpty());

    item5 = std::move(itemMoved);
    CORRADE_VERIFY(item5.list() == &list);
    CORRADE_VERIFY(list2.isEmpty());

    CORRADE_VERIFY(list.first() == &item5);
    CORRADE_VERIFY(list.last() == &item5);

    list.cut(&item5);
}

}}}

CORRADE_TEST_MAIN(Corrade::Containers::Test::LinkedListTest)
