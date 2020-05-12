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
#include <vector>

#include "Corrade/Containers/LinkedList.h"
#include "Corrade/TestSuite/Tester.h"
#include "Corrade/Utility/DebugStl.h" /** @todo remove when <sstream> is gone */

namespace Corrade { namespace Containers { namespace Test { namespace {

struct LinkedListTest: TestSuite::Tester {
    explicit LinkedListTest();

    void listBackReference();
    void insert();
    void insertFromOtherList();
    void insertBeforeFromOtherList();
    void cut();
    void cutFromOtherList();
    void move();
    void moveBeforeItself();
    void clear();
    void moveList();
    void moveItem();

    void rangeBasedFor();
    void overrideErase();
    void overrideEraseVirtual();
};

class Item: public LinkedListItem<Item> {
    public:
        Item(Item&& other): LinkedListItem<Item>(std::forward<LinkedListItem<Item>>(other)) {}

        Item& operator=(Item&& other) {
            LinkedListItem<Item>::operator=(std::forward<LinkedListItem<Item>>(other));
            return *this;
        }

        static int count;
        Item() { ++count; }
        ~Item() { --count; }
};

typedef Containers::LinkedList<Item> LinkedList;

int Item::count = 0;

LinkedListTest::LinkedListTest() {
    addTests({&LinkedListTest::listBackReference,
              &LinkedListTest::insert,
              &LinkedListTest::insertFromOtherList,
              &LinkedListTest::insertBeforeFromOtherList,
              &LinkedListTest::cut,
              &LinkedListTest::cutFromOtherList,
              &LinkedListTest::move,
              &LinkedListTest::moveBeforeItself,
              &LinkedListTest::clear,
              &LinkedListTest::moveList,
              &LinkedListTest::moveItem,

              &LinkedListTest::rangeBasedFor,
              &LinkedListTest::overrideErase,
              &LinkedListTest::overrideEraseVirtual});
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
    #ifdef CORRADE_NO_ASSERT
    CORRADE_SKIP("CORRADE_NO_ASSERT defined, can't test assertions");
    #endif

    std::stringstream out;
    Error redirectError{&out};

    LinkedList list;
    Item item;
    list.insert(&item);

    LinkedList list2;
    list2.insert(&item);
    CORRADE_COMPARE(out.str(), "Containers::LinkedList::insert(): cannot insert an item already connected elsewhere\n");
}

void LinkedListTest::insertBeforeFromOtherList() {
    #ifdef CORRADE_NO_ASSERT
    CORRADE_SKIP("CORRADE_NO_ASSERT defined, can't test assertions");
    #endif

    std::stringstream out;
    Error redirectError{&out};

    LinkedList list;
    Item item;
    list.insert(&item);

    LinkedList list2;
    Item item2;
    list2.insert(&item2, &item);
    CORRADE_COMPARE(out.str(), "Containers::LinkedList::insert(): cannot insert before an item which is not a part of the list\n");
}

void LinkedListTest::cutFromOtherList() {
    #ifdef CORRADE_NO_ASSERT
    CORRADE_SKIP("CORRADE_NO_ASSERT defined, can't test assertions");
    #endif

    std::stringstream out;
    Error redirectError{&out};

    LinkedList list;
    Item item;
    list.insert(&item);

    LinkedList list2;
    list2.cut(&item);
    CORRADE_COMPARE(out.str(), "Containers::LinkedList::cut(): cannot cut out an item which is not a part of the list\n");
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

void LinkedListTest::move() {
    LinkedList list;
    Item item;
    Item item2;
    Item item3;
    Item item4;
    list.insert(&item);
    list.insert(&item2);
    list.insert(&item3);
    list.insert(&item4);

    list.move(&item3, &item2);

    CORRADE_COMPARE(item.next(), &item3);
    CORRADE_COMPARE(item3.next(), &item2);
    CORRADE_COMPARE(item2.next(), &item4);
}

void LinkedListTest::moveBeforeItself() {
    LinkedList list;
    Item item;
    Item item2;
    Item item3;
    Item item4;
    list.insert(&item);
    list.insert(&item2);
    list.insert(&item3);
    list.insert(&item4);

    list.move(&item3, &item3);

    CORRADE_COMPARE(item2.next(), &item3);
    CORRADE_COMPARE(item3.next(), &item4);
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

void LinkedListTest::rangeBasedFor() {
    LinkedList list;
    Item item;
    Item item2;
    Item item3;
    list.insert(&item);
    list.insert(&item2);
    list.insert(&item3);

    {
        std::vector<Item*> items;
        for(auto&& i: list) items.push_back(&i);
        CORRADE_COMPARE(items, (std::vector<Item*>{&item, &item2, &item3}));
    } {
        const LinkedList& clist = list;
        std::vector<const Item*> items;
        for(auto&& i: clist) items.push_back(&i);
        CORRADE_COMPARE(items, (std::vector<const Item*>{&item, &item2, &item3}));
    }
}

void LinkedListTest::overrideErase() {
    struct NonErasingItem: LinkedListItem<NonErasingItem> {
        void erase() {
            list()->cut(this);
            dead = true;
        }
        bool dead = false;
    };

    /* Have the items initialized before the list so we test that the list
       doesn't try to call delete on them first. */
    NonErasingItem item;
    NonErasingItem item2;

    CORRADE_VERIFY(!item.dead);
    CORRADE_VERIFY(!item2.dead);
    CORRADE_COMPARE(item.list(), nullptr);
    CORRADE_COMPARE(item2.list(), nullptr);

    {
        Containers::LinkedList<NonErasingItem> list;
        list.insert(&item);
        list.insert(&item2);
        CORRADE_COMPARE(item.list(), &list);
        CORRADE_COMPARE(item2.list(), &list);
    }

    CORRADE_VERIFY(item.dead);
    CORRADE_VERIFY(item2.dead);
    CORRADE_COMPARE(item.list(), nullptr);
    CORRADE_COMPARE(item2.list(), nullptr);
}

void LinkedListTest::overrideEraseVirtual() {
    struct NonErasingItemBase: LinkedListItem<NonErasingItemBase> {
        void doErase() override {
            list()->cut(this);
            dead = true;
        }
        bool dead = false;
    };
    struct NonErasingItem: NonErasingItemBase {
        /* This shouldn't get called, doErase() should */
        CORRADE_UNUSED void erase() { CORRADE_INTERNAL_ASSERT_UNREACHABLE(); }
    };

    /* Have the items initialized before the list so we test that the list
       doesn't try to call delete on them first. */
    NonErasingItem item;
    NonErasingItem item2;

    CORRADE_VERIFY(!item.dead);
    CORRADE_VERIFY(!item2.dead);
    CORRADE_COMPARE(item.list(), nullptr);
    CORRADE_COMPARE(item2.list(), nullptr);

    {
        /* Have list of the base items so erase() doesn't get called */
        Containers::LinkedList<NonErasingItemBase> list;
        list.insert(&item);
        list.insert(&item2);
        CORRADE_COMPARE(item.list(), &list);
        CORRADE_COMPARE(item2.list(), &list);
    }

    CORRADE_VERIFY(item.dead);
    CORRADE_VERIFY(item2.dead);
    CORRADE_COMPARE(item.list(), nullptr);
    CORRADE_COMPARE(item2.list(), nullptr);
}

}}}}

CORRADE_TEST_MAIN(Corrade::Containers::Test::LinkedListTest)
